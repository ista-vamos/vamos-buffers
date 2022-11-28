/* **********************************************************
 * Copyright (c) 2021 IST Austria.  All rights reserved.
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
#define _GNU_SOURCE
#define LINUX
#define X86_64
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <unistd.h>

#include "dr_api.h"
#include "dr_defines.h"
#include "drmgr.h"
#include "drreg.h"
#include "drutil.h"
#ifdef SHOW_SYMBOLS
#include "drsyms.h"
#endif

#define BUFFER_SIZE_IN_PAGES 10
#include "shm_monitored.h"

#ifdef WINDOWS
#define IF_WINDOWS(x) x
#else
#define IF_WINDOWS(x) /* nothing */
#endif

#include "events.h"

static void event_exit(void);

static dr_emit_flags_t event_app_instruction(void *drcontext, void *tag,
                                             instrlist_t *bb, instr_t *instr,
                                             bool for_trace, bool translating,
                                             void *user_data);

static module_data_t *main_module;

struct call_event_spec events[] = {
    {.name = "isprime", .signature = "i"},
};

DR_EXPORT void dr_client_main(client_id_t id, int argc, const char *argv[]) {
    dr_set_client_name("Track calls and returns",
                       "http://dynamorio.org/issues");
    drmgr_init();
    /* make it easy to tell, by looking at log file, which client executed */
    dr_log(NULL, DR_LOG_ALL, 1, "Client 'drfun' initializing\n");
    /* also give notification to stderr */
    if (dr_is_notify_on()) {
#ifdef WINDOWS
        /* ask for best-effort printing to cmd window.  must be called at init.
         */
        dr_enable_console_printing();
#endif
        dr_fprintf(STDERR, "Client instrcalls is running\n");
    }

    if (drsym_init(0) != DRSYM_SUCCESS) {
        dr_log(NULL, DR_LOG_ALL, 1,
               "WARNING: unable to initialize symbol translation\n");
    }
    dr_register_exit_event(event_exit);

    // TODO: use to instrument only main module:
    // dr_module_set_should_instrument()
    main_module      = dr_get_main_module();
    drsym_error_t ok = drsym_lookup_symbol(main_module->full_path,
                                           events[0].name, &events[0].addr,
                                           /* flags = */ 0);
    if (ok == DRSYM_ERROR_LINE_NOT_AVAILABLE || ok == DRSYM_SUCCESS) {
        dr_printf("Found %s:%s at %lu\n", main_module->full_path,
                  events[0].name, events[0].addr);
    } else {
        dr_fprintf(STDERR, "Cannot find %s:%s\n", main_module->full_path,
                   events[0].name);
        DR_ASSERT(0);
    }
    DR_ASSERT(main_module);
    initialize_application_buffer();
    initialize_thread_buffer(BUFFER_SIZE_IN_PAGES, 1);

    drmgr_register_bb_instrumentation_event(NULL, event_app_instruction, 0);

    app_buffer_wait_for_client();
}

static void event_exit(void) {
    dr_free_module_data(main_module);
    close_app_buffer();
#ifdef SHOW_SYMBOLS
    if (drsym_exit() != DRSYM_SUCCESS) {
        dr_log(NULL, DR_LOG_ALL, 1,
               "WARNING: error cleaning up symbol library\n");
    }
#endif
    drmgr_exit();
}

#define INSERT instrlist_meta_preinsert

/* adapted from instrcalls.c */
static app_pc call_get_target(instr_t *instr) {
    app_pc target   = 0;
    opnd_t targetop = instr_get_target(instr);
    if (opnd_is_pc(targetop)) {
        if (opnd_is_far_pc(targetop)) {
            DR_ASSERT(false && "call_get_target: far pc not supported");
        }
        target = (app_pc)opnd_get_pc(targetop);
    } else if (opnd_is_instr(targetop)) {
        // instr_t *tgt = opnd_get_instr(targetop);
        // target = (app_pc)instr_get_translation(tgt);
        DR_ASSERT(target != 0 && "call_get_target: unknown target");
        // if (opnd_is_far_instr(targetop)) {
        //     /* FIXME: handle far instr */
        //     DR_ASSERT(false &&
        //               "call_get_target: far instr "
        //               "not supported");
        // }
    } else {
        DR_ASSERT(false && "call_get_target: unknown target");
        target = 0;
    }
    return target;
}

static dr_emit_flags_t event_app_instruction(void *drcontext, void *tag,
                                             instrlist_t *bb, instr_t *instr,
                                             bool for_trace, bool translating,
                                             void *user_data) {
    if (instr_is_meta(instr) || translating)
        return DR_EMIT_DEFAULT;
    /* instrument only calls/returns from the main binary */
    if (!dr_module_contains_addr(main_module, instr_get_app_pc(instr)))
        return DR_EMIT_DEFAULT;
#ifdef VERBOSE
    if (drmgr_is_first_instr(drcontext, instr)) {
        instrlist_disassemble(drcontext, tag, bb, STDOUT);
    }
#endif
    if (instr_is_call_direct(instr)) {
        app_pc target = call_get_target(instr);
        DR_ASSERT(target && "Do not have call target");
        size_t off = dr_module_addr_offset(main_module, target);
        // dr_printf("off: %lu == %lu events[0].addr\n", off, events[0].addr);
        if (off == (~(size_t)0))
            return DR_EMIT_DEFAULT;
        if (off != events[0].addr) {
            return DR_EMIT_DEFAULT;
        }

        dr_insert_clean_call(drcontext, bb, instr, (void *)push_event_wait_64,
                             /* safe_fp_state = */ false,
                             /* num_args = */ 2, OPND_CREATE_INT16(0),
                             opnd_create_reg(DR_REG_EDI));

        instrlist_disassemble(drcontext, tag, bb, STDOUT);
    }
    return DR_EMIT_DEFAULT;
}
