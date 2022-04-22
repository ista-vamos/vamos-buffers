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

#include "shm.h"
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

struct call_event_spec events[] = {
	{
		.name = "main",
		.signature = ""
	},
};

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[])
{
    if (argc >= 2) {
            events[0].name = argv[1];
            if (argc == 3) {
                DR_ASSERT(strlen(argv[2]) <=  sizeof(events[0].signature));
                strncpy(events[0].signature, argv[2],
                        sizeof(events[0].signature));
            }
    } else {
            dr_fprintf(STDERR, "Need arguments 'fun' [signature]\n");
            DR_ASSERT(0);
            return;
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
    drsym_error_t ok = drsym_lookup_symbol(main_module->full_path,
					   events[0].name,
					   &events[0].addr,
					   /* flags = */ 0);
    if (ok == DRSYM_ERROR_LINE_NOT_AVAILABLE || ok == DRSYM_SUCCESS) {
	    dr_printf("Found %s:%s at %lu\n",
		      main_module->full_path,
		      events[0].name,
		      events[0].addr);
    } else {
	    dr_fprintf(STDERR, "Cannot find %s:%s\n",
		       main_module->full_path,
		       events[0].name);
	    DR_ASSERT(0);
    }
    DR_ASSERT(main_module);
    shm = initialize_shared_buffer();
    DR_ASSERT(shm);
    drmgr_register_bb_instrumentation_event(NULL, event_app_instruction, 0);

    //buffer_wait_for_monitor(shm);
}

static void
event_exit(void)
{
    dr_free_module_data(main_module);
    destroy_shared_buffer(shm);
#ifdef SHOW_SYMBOLS
    if (drsym_exit() != DRSYM_SUCCESS) {
        dr_log(NULL, DR_LOG_ALL, 1, "WARNING: error cleaning up symbol library\n");
    }
#endif
    drmgr_exit();
}

#define INSERT instrlist_meta_preinsert

/* load the offset in buffer and store it into 'reg_tmp' */
static void
insert_load_buf_off(void *drcontext, instrlist_t *ilist,
		    instr_t *where, reg_id_t reg_ptr, reg_id_t reg_tmp)
{
	if (!drutil_insert_get_mem_addr(drcontext, ilist, where,
				   opnd_create_rel_addr(shm, sizeof(shm)),
				   reg_ptr, reg_tmp)) {
		DR_ASSERT(0);
	}
	INSERT(ilist, where,
	       XINST_CREATE_load(drcontext,
				 opnd_create_reg(reg_tmp),
	                         OPND_CREATE_MEMPTR(reg_ptr,
						    buffer_get_pos_offset())));
}

static void
compare_buf_off(void *drcontext, instrlist_t *ilist,
		instr_t *where, reg_id_t scratch,
		reg_id_t reg_tmp, size_t max_val, size_t len)
{
	/* scratch has been used to store a pointer, so it has size
	 * to fit size_t. Store the maximal offset into it */
	INSERT(ilist, where,
		XINST_CREATE_load_int(drcontext, opnd_create_reg(scratch),
                                  OPND_CREATE_INT64(max_val - len)));
	/* compare the buffer offset with the maximal offset that
	 * we can have */
	INSERT(ilist, where,
	       XINST_CREATE_cmp(drcontext,
				opnd_create_reg(reg_tmp),
	                        opnd_create_reg(scratch)));
}


static void rotate_buffer(void) {
	dr_fprintf(STDOUT, "Rotating buffer\n");
	buffer_rotate(shm);
}

static void
rotate_if_needed(void *drcontext, instrlist_t *ilist,
		 instr_t *where, reg_id_t scratch,
		 reg_id_t reg_tmp)
{

	/* reg_tmp now contains the offset into the buffer */
	compare_buf_off(drcontext, ilist, where, scratch, reg_tmp,
			buffer_allocation_size(), /* len = */ 12);

	/* this is the last inserted instruction */
	instr_t *cmp = instr_get_prev(where);
	DR_ASSERT(cmp); /* we've already inserted instruction before */
	instr_t *call_entry = INSTR_CREATE_label(drcontext);

	dr_insert_clean_call(drcontext, ilist, where,
			     (void *) rotate_buffer,
			     /* safe_fp_state = */ false,
			     /* num_args = */ 0);

	/* set the offset to 0 after rotating*/
	INSERT(ilist, where,
		XINST_CREATE_load_int(drcontext, opnd_create_reg(reg_tmp),
                                  OPND_CREATE_INT64(0)));

	INSERT(ilist, where, call_entry);
	/* Note that this instruction is inserted 'before'
	 * all the code that has been inserted by dr_insert_clean_call() */
	INSERT(ilist, instr_get_next(cmp),
	       XINST_CREATE_jump_cond(drcontext,
				      DR_PRED_L,
				      opnd_create_instr(call_entry)));
}

static size_t
write_call(void *drcontext, instrlist_t *ilist,
	   instr_t *where, reg_id_t reg_ptr, reg_id_t reg_tmp,
	   size_t addr)
{
	/* get pointer to the data */
	if (!drutil_insert_get_mem_addr(drcontext, ilist, where,
				   opnd_create_rel_addr(buffer_get_beginning(shm),
					                sizeof(void *)),
				   reg_ptr, reg_ptr)) {
		// TODO: cannot reg_ptr, reg_ptr make trouble?
		DR_ASSERT(0);
	}
	INSERT(ilist, where,
	       XINST_CREATE_add(drcontext,
				opnd_create_reg(reg_ptr),
				opnd_create_reg(reg_tmp)));

	INSERT(ilist, where,
		XINST_CREATE_load_int(drcontext, opnd_create_reg(reg_tmp),
	                          OPND_CREATE_INT64(addr)));
	INSERT(ilist, where,
	       XINST_CREATE_store(drcontext,
				  OPND_CREATE_MEM64(reg_ptr, 0),
				  opnd_create_reg(reg_tmp)));

	/* store the first argument */
	INSERT(ilist, where,
	       XINST_CREATE_store(drcontext,
				  OPND_CREATE_MEM32(reg_ptr, sizeof(size_t)),
				  opnd_create_reg(DR_REG_EDI)));

	return sizeof(size_t) + 4;
#if 0
	/* shift pointer by 4 bytes */
	INSERT(ilist, where,
	       XINST_CREATE_add(drcontext,
				opnd_create_reg(reg_ptr),
				OPND_CREATE_INT64(4)));

	/* store the second argument */
	INSERT(ilist, where,
	       XINST_CREATE_store(drcontext,
				  OPND_CREATE_MEM32(reg_ptr, 0),
				  opnd_create_reg(DR_REG_ESI)));

#endif
}

static void
update_buf_off(void *drcontext, instrlist_t *ilist,
	       instr_t *where, reg_id_t reg_ptr, reg_id_t reg_tmp,
	       size_t add_len)
{
	if (!drutil_insert_get_mem_addr(drcontext, ilist, where,
				   opnd_create_rel_addr(shm, sizeof(shm)),
				   reg_ptr, reg_tmp)) {
		DR_ASSERT(0);
	}
	INSERT(ilist, where,
	       XINST_CREATE_load(drcontext,
				 opnd_create_reg(reg_tmp),
	                         OPND_CREATE_MEMPTR(reg_ptr,
						    buffer_get_pos_offset())));
	INSERT(ilist, where,
	       XINST_CREATE_add(drcontext,
				opnd_create_reg(reg_tmp),
				OPND_CREATE_INT64(add_len)));
	INSERT(ilist, where,
	       XINST_CREATE_store(drcontext,
	                          OPND_CREATE_MEMPTR(reg_ptr,
						    buffer_get_pos_offset()),
				  opnd_create_reg(reg_tmp)));
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
       //instr_t *tgt = opnd_get_instr(targetop);
       //target = (app_pc)instr_get_translation(tgt);
        DR_ASSERT(target != 0 &&
                  "call_get_target: unknown target");
       //if (opnd_is_far_instr(targetop)) {
       //    /* FIXME: handle far instr */
       //    DR_ASSERT(false &&
       //              "call_get_target: far instr "
       //              "not supported");
       //}
    } else {
        DR_ASSERT(false && "call_get_target: unknown target");
        target = 0;
    }
    return target;
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
#ifdef VERBOSE
    if (drmgr_is_first_instr(drcontext, instr)) {
        instrlist_disassemble(drcontext, tag, bb, STDOUT);
    }
#endif
    if (instr_is_call_direct(instr)) {
	app_pc target = call_get_target(instr);
	DR_ASSERT(target && "Do not have call target");
	size_t off = dr_module_addr_offset(main_module, target);
	//dr_printf("off: %lu == %lu events[0].addr\n", off, events[0].addr);
	if (off == (~(size_t)0))
		return DR_EMIT_DEFAULT;
	if (off != events[0].addr) {
		return DR_EMIT_DEFAULT;
	}

	/* We need two registers */
	reg_id_t reg_ptr, reg_tmp;
	if (drreg_reserve_register(drcontext, bb, instr, NULL, &reg_ptr)
		!= DRREG_SUCCESS ||
	    drreg_reserve_register(drcontext, bb, instr, NULL, &reg_tmp)
		!= DRREG_SUCCESS) {
		DR_ASSERT(false); /* just fail for now */
		abort();
	}
	//dr_printf("Found a call of %s\n", events[0].name);

	insert_load_buf_off(drcontext, bb, instr, reg_ptr, reg_tmp);
	rotate_if_needed(drcontext, bb, instr, reg_ptr, reg_tmp);
	size_t written = write_call(drcontext, bb, instr, reg_ptr, reg_tmp, off);
	update_buf_off(drcontext, bb, instr, reg_ptr, reg_tmp, written);

	if (drreg_unreserve_register(drcontext, bb, instr, reg_ptr) != DRREG_SUCCESS ||
	    drreg_unreserve_register(drcontext, bb, instr, reg_tmp) != DRREG_SUCCESS) {
		DR_ASSERT(false);
		abort();
	}

	instrlist_disassemble(drcontext, tag, bb, STDOUT);
        //dr_insert_call_instrumentation(drcontext, bb, instr, (app_pc)at_call);
    }/* else if (instr_is_call_indirect(instr)) {
        dr_insert_mbr_instrumentation(drcontext, bb, instr, (app_pc)at_call_ind,
                                      SPILL_SLOT_1);

    } else if (instr_is_return(instr)) {
        dr_insert_mbr_instrumentation(drcontext, bb, instr, (app_pc)at_return,
                                      SPILL_SLOT_1);
    }
    */
    return DR_EMIT_DEFAULT;
}
