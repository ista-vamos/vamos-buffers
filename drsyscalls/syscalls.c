/* **********************************************************
 * Copyright (c) 2011-2018 Google, Inc.  All rights reserved.
 * Copyright (c) 2009-2010 VMware, Inc.  All rights reserved.
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

/* Based on Code Manipulation API Sample (syscall.c) from DynamoRIO
 */

#include "dr_api.h"
#include "drmgr.h"
#include <string.h> /* memset */

#ifdef UNIX
#    if defined(MACOS) || defined(ANDROID)
#        include <sys/syscall.h>
#    else
#        include <syscall.h>
#    endif
#endif

/* Some syscalls have more args, but this is the max we need for SYS_write/NtWriteFile */
#ifdef WINDOWS
#    define SYS_MAX_ARGS 9
#else
#    define SYS_MAX_ARGS 3
#endif

#if 0
/* Thread-context-local data structure for storing system call
 * parameters.  Since this state spans application system call
 * execution, thread-local data is not sufficient on Windows: we need
 * thread-context-local, or "callback-local", provided by the drmgr
 * extension.
 */
typedef struct {
    reg_t param[SYS_MAX_ARGS];
#ifdef WINDOWS
    reg_t xcx; /* emulation parameter for WOW64 */
#endif
    bool repeat;
} per_thread_t;

/* Thread-context-local storage index from drmgr */
static int tcls_idx;
#endif

/* The system call number of SYS_write/NtWriteFile */
static int write_sysnum, read_sysnum;

static int
get_write_sysnum(void);
static int
get_read_sysnum(void);
static void
event_exit(void);
static bool
event_filter_syscall(void *drcontext, int sysnum);
static bool
event_pre_syscall(void *drcontext, int sysnum);
static void
event_post_syscall(void *drcontext, int sysnum);
/*
static void
event_thread_context_init(void *drcontext, bool new_depth);
static void
event_thread_context_exit(void *drcontext, bool process_exit);
*/

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[])
{
    dr_set_client_name("Shamon intercept write and read syscalls", "http://...");
    drmgr_init();
    write_sysnum = get_write_sysnum();
    read_sysnum = get_read_sysnum();
    dr_register_filter_syscall_event(event_filter_syscall);
    drmgr_register_pre_syscall_event(event_pre_syscall);
    drmgr_register_post_syscall_event(event_post_syscall);
    dr_register_exit_event(event_exit);
    /*
    tcls_idx =
        drmgr_register_cls_field(event_thread_context_init, event_thread_context_exit);
    DR_ASSERT(tcls_idx != -1);
    */
    if (dr_is_notify_on()) {
#    ifdef WINDOWS
        /* ask for best-effort printing to cmd window.  must be called at init. */
        dr_enable_console_printing();
#    endif
        dr_fprintf(STDERR, "Client syscall is running\n");
    }
}


static void
event_exit(void)
{
    if (/*!drmgr_unregister_cls_field(event_thread_context_init, event_thread_context_exit,
                                    tcls_idx) ||*/
        !drmgr_unregister_pre_syscall_event(event_pre_syscall) ||
        !drmgr_unregister_post_syscall_event(event_post_syscall))
        DR_ASSERT(false && "failed to unregister");
    drmgr_exit();
}

#if 0
static void
event_thread_context_init(void *drcontext, bool new_depth)
{
    /* create an instance of our data structure for this thread context */
    per_thread_t *data;
#ifdef SHOW_RESULTS
    dr_fprintf(STDERR, "new thread context id=" TIDFMT "%s\n",
               dr_get_thread_id(drcontext), new_depth ? " new depth" : "");
#endif
    if (new_depth) {
        data = (per_thread_t *)dr_thread_alloc(drcontext, sizeof(per_thread_t));
        drmgr_set_cls_field(drcontext, tcls_idx, data);
    } else
        data = (per_thread_t *)drmgr_get_cls_field(drcontext, tcls_idx);
    memset(data, 0, sizeof(*data));
}

static void
event_thread_context_exit(void *drcontext, bool thread_exit)
{
#ifdef SHOW_RESULTS
    dr_fprintf(STDERR, "resuming prior thread context id=" TIDFMT "\n",
               dr_get_thread_id(drcontext));
#endif
    if (thread_exit) {
        per_thread_t *data = (per_thread_t *)drmgr_get_cls_field(drcontext, tcls_idx);
        dr_thread_free(drcontext, data, sizeof(per_thread_t));
    }
    /* else, nothing to do: we leave the struct for re-use on next context */
}
#endif

static bool
event_filter_syscall(void *drcontext, int sysnum)
{
    return sysnum == write_sysnum || sysnum == read_sysnum ;
}

static bool
event_pre_syscall(void *drcontext, int sysnum)
{
    if (sysnum != write_sysnum)
	return true;

    int fd = dr_syscall_get_param(drcontext, 0);
    if (fd != STDERR && fd != STDOUT)
	return true;
    
    byte *output = (byte *)dr_syscall_get_param(drcontext, 1);
    byte first;
    if (!dr_safe_read(output, 1, &first, NULL)) {
        dr_fprintf(STDERR, "err: cannot read syscall data (write)");
        return true; /* data unreadable: execute normally */
    }

    unsigned int len = dr_syscall_get_param(drcontext, 2);
    // per_thread_t *data = (per_thread_t *)drmgr_get_cls_field(drcontext, tcls_idx);
    dr_printf("\033[0;36m[write@%d (%u)]: %.*s\033[0m", fd, len, len, output);
    return true; /* execute normally */
}

static void
event_post_syscall(void *drcontext, int sysnum)
{
    if (sysnum != read_sysnum)
	return;

    int fd = dr_syscall_get_param(drcontext, 0);
    if (fd != STDIN)
	return;
    
    byte *output = (byte *)dr_syscall_get_param(drcontext, 1);
    byte first;
    if (!dr_safe_read(output, 1, &first, NULL)) {
        dr_fprintf(STDERR, "err: cannot read syscall data (read)");
	return;
    }

    int len = dr_syscall_get_result(drcontext);
    if (len > 0) {
    	//per_thread_t *data = (per_thread_t *)drmgr_get_cls_field(drcontext, tcls_idx);
    	dr_printf("\033[0;34m[read@%d (%u)]: %.*s\033[0m", fd, len, len, output);
    }
}

static int
get_write_sysnum(void)
{
    /* XXX: we could use the "drsyscall" Extension from the Dr. Memory Framework
     * (DRMF) to obtain the number of any system call from the name.
     */
#ifdef UNIX
    return SYS_write;
#else
    byte *entry;
    module_data_t *data = dr_lookup_module_by_name("ntdll.dll");
    DR_ASSERT(data != NULL);
    entry = (byte *)dr_get_proc_address(data->handle, "NtWriteFile");
    DR_ASSERT(entry != NULL);
    dr_free_module_data(data);
    return drmgr_decode_sysnum_from_wrapper(entry);
#endif
}

static int
get_read_sysnum(void)
{
#ifdef UNIX
    return SYS_read;
#else
    byte *entry;
    module_data_t *data = dr_lookup_module_by_name("ntdll.dll");
    DR_ASSERT(data != NULL);
    entry = (byte *)dr_get_proc_address(data->handle, "NtReadFile");
    DR_ASSERT(entry != NULL);
    dr_free_module_data(data);
    return drmgr_decode_sysnum_from_wrapper(entry);
#endif
}
