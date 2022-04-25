/* **********************************************************
 * Copyright (c) 2021-2022 IST Austria.  All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of VMware, Inc. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL VMWARE, INC. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
 * We took inspiration in instrace_simple.c and instrcalls.c sample
 * tools from DynamoRIO.
 */

#define SHOW_SYMBOLS 1
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/file.h>

#include "dr_api.h"
#include "dr_defines.h"
#include "drmgr.h"
#include "drreg.h"
#include "drutil.h"
#ifdef SHOW_SYMBOLS
#    include "drsyms.h"
#endif

#include "buffer.h"
#include "client.h"

#ifdef WINDOWS
#    define IF_WINDOWS(x) x
#else
#    define IF_WINDOWS(x) /* nothing */
#endif

#include "events.h"

static void
event_exit(void);

static dr_emit_flags_t
event_app_instruction(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr,
                      bool for_trace, bool translating, void *user_data);

static struct buffer *shm;
static module_data_t *main_module;

static struct call_event_spec *events;
static size_t events_num;
size_t max_elem_size;

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[])
{
    if (argc < 2) {
        dr_fprintf(STDERR, "Need arguments 'fun1:[sig]' 'fun2:[sig]' ...\n");
        DR_ASSERT(0);
    }

    events_num = argc - 1;
    events = initialize_shared_control_buffer(sizeof(struct call_event_spec)*events_num);
    for (int i = 1; i < argc; ++i) {
        const char *sig = strrchr(argv[i], ':');
        if (sig) {
           ++sig;
           DR_ASSERT(strlen(sig) <=  sizeof(events[0].signature));
           strncpy(events[i-1].signature, sig,
                   sizeof(events[0].signature));
           strncpy(events[i-1].name, argv[i], sig - argv[i] - 1);
        } else {
            DR_ASSERT(strlen(argv[i]) < 256 && "Too big function name");
            strncpy(events[i-1].name, argv[i], 255);
            events[i-1].signature[0] = '\0';
        }
    }

    dr_set_client_name("Track function calls", "");
    drmgr_init();
    /* make it easy to tell, by looking at log file, which client executed */
    dr_log(NULL, DR_LOG_ALL, 1, "Client 'drfun' initializing\n");
    /* also give notification to stderr */
    if (dr_is_notify_on()) {
#    ifdef WINDOWS
        /* ask for best-effort printing to cmd window.  must be called at init. */
        dr_enable_console_printing();
#    endif
        dr_fprintf(STDERR, "Client instrcalls is running\n");
    }

    if (drsym_init(0) != DRSYM_SUCCESS) {
        dr_log(NULL, DR_LOG_ALL, 1,
               "WARNING: unable to initialize symbol translation\n");
    }
    dr_register_exit_event(event_exit);

    // TODO: use to instrument only main module: dr_module_set_should_instrument()
    main_module = dr_get_main_module();
    DR_ASSERT(main_module);
    size_t elem_size;
    for (int i = 0; i < events_num; ++i) {
        drsym_error_t ok = drsym_lookup_symbol(main_module->full_path,
                           events[i].name,
                           &events[i].addr,
                           /* flags = */ 0);
        if (ok == DRSYM_ERROR_LINE_NOT_AVAILABLE || ok == DRSYM_SUCCESS) {
            dr_printf("Found %s:%s in %s at %lu\n",
                      events[i].name,
                      events[i].signature,
                      main_module->full_path,
                      events[i].addr);
            elem_size = call_event_spec_get_size(&events[i]);
            if (elem_size > max_elem_size)
                max_elem_size = elem_size;
        } else {
            dr_fprintf(STDERR,
                       "Cannot find %s:%s in %s\n",
                       events[i].name,
                       events[i].signature,
                       main_module->full_path);
            DR_ASSERT(0);
        }
    }
    DR_ASSERT(max_elem_size > 0);
    max_elem_size += sizeof(void *); /* space for fun address */
    shm = initialize_shared_buffer(max_elem_size);
    DR_ASSERT(shm);
    drmgr_register_bb_instrumentation_event(NULL, event_app_instruction, 0);

    dr_printf("Waiting for the monitor to attach\n");
    buffer_wait_for_monitor(shm);
}

static void
event_exit(void)
{
    dr_free_module_data(main_module);
#ifdef SHOW_SYMBOLS
    if (drsym_exit() != DRSYM_SUCCESS) {
        dr_log(NULL, DR_LOG_ALL, 1, "WARNING: error cleaning up symbol library\n");
    }
#endif
    drmgr_exit();
    destroy_shared_buffer(shm);
    release_shared_control_buffer(events);
}

/* adapted from instrcalls.c */
static app_pc
call_get_target(instr_t *instr) {
    app_pc target = 0;
    opnd_t targetop = instr_get_target(instr);
    if (opnd_is_pc(targetop)) {
        if (opnd_is_far_pc(targetop)) {
            DR_ASSERT(false &&
                          "call_get_target: far pc not supported");
        }
        target = (app_pc)opnd_get_pc(targetop);
    } else if (opnd_is_instr(targetop)) {
        DR_ASSERT(target != 0 &&
                  "call_get_target: unknown target");
    } else {
        DR_ASSERT(false && "call_get_target: unknown target");
        target = 0;
    }
    return target;
}

static inline void *
call_get_arg_ptr(dr_mcontext_t *mc, int i, char o) {
    if (o == 'f') {
        DR_ASSERT(i < 7);
        return &mc->simd[i].u64;
    }
    DR_ASSERT(i < 6);
    switch(i) {
        case 0: return &mc->xdi;
        case 1: return &mc->xsi;
        case 2: return &mc->xdx;
        case 3: return &mc->xcx;
        case 4: return &mc->r8;
        case 5: return &mc->r9;
    }
}

static void
at_call_generic(size_t fun_idx, const char *sig)
{
    dr_mcontext_t mc = { sizeof(mc), DR_MC_INTEGER };
    dr_get_mcontext(dr_get_current_drcontext(), &mc);
    void *shmaddr = buffer_start_push(shm);
    DR_ASSERT(shmaddr);
    shmaddr = buffer_partial_push(shm, shmaddr,
                                  &fun_idx, sizeof(fun_idx));
    DR_ASSERT(shmaddr);
    printf("Fun %lu\n", fun_idx);
    int i = 0;
    for (const char *o = sig; *o; ++o) {
        if (*o != '_') {
            shmaddr = buffer_partial_push(shm, shmaddr,
                                          call_get_arg_ptr(&mc, i, *o),
                                          call_event_op_get_size(*o));
            printf(" arg %d=%ld", i, *(size_t*)call_get_arg_ptr(&mc, i, *o));
        }
        ++i;
    }
    if (!buffer_finish_push(shm)) {
        DR_ASSERT(0 && "Buffer push failed");
    }
    putchar('\n');
}

static dr_emit_flags_t
event_app_instruction(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr,
                      bool for_trace, bool translating, void *user_data)
{
    if (instr_is_meta(instr) || translating)
        return DR_EMIT_DEFAULT;

    /* instrument only calls/returns from the main binary */
    if (!dr_module_contains_addr(main_module, instr_get_app_pc(instr)))
        return DR_EMIT_DEFAULT;

    if (instr_is_call_direct(instr)) {
        app_pc target = call_get_target(instr);
        DR_ASSERT(target && "Do not have call target");
        size_t off = dr_module_addr_offset(main_module, target);
        //dr_printf("off: %lu == %lu events[0].addr\n", off, events[0].addr);
        if (off == (~(size_t)0))
            return DR_EMIT_DEFAULT;
        for (size_t i = 0; i < events_num; ++i) {
            if (off == events[i].addr) {
                dr_printf("Found a call of %s\n", events[i].name);
                dr_insert_clean_call_ex(
                    drcontext, bb, instr, (app_pc)at_call_generic,
                    DR_CLEANCALL_READS_APP_CONTEXT, 2,
                    /* call target is 1st parameter */
                    OPND_CREATE_INT64(i),
                    /* signature is 2nd parameter */
                    OPND_CREATE_INTPTR(events[i].signature));
            }
        }
    }

    /* else if (instr_is_call_indirect(instr)) {
        dr_insert_mbr_instrumentation(drcontext, bb, instr, (app_pc)at_call_ind,
                                      SPILL_SLOT_1);

    } else if (instr_is_return(instr)) {
        dr_insert_mbr_instrumentation(drcontext, bb, instr, (app_pc)at_return,
                                      SPILL_SLOT_1);
    }
    */
    return DR_EMIT_DEFAULT;
}
