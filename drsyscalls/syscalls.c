/* 
 * Based on Code Manipulation API Sample (syscall.c) from DynamoRIO
 */

#include <string.h> /* memset */
#include "dr_api.h"
#include "drmgr.h"

#include "fastbuf/shm_monitored.h"

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

typedef struct {
	int fd;
} per_thread_t;

/* Thread-context-local storage index from drmgr */
static int tcls_idx;

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
static void
event_thread_context_init(void *drcontext, bool new_depth);
static void
event_thread_context_exit(void *drcontext, bool process_exit);

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
    tcls_idx =
        drmgr_register_cls_field(event_thread_context_init, event_thread_context_exit);
    DR_ASSERT(tcls_idx != -1);
    if (dr_is_notify_on()) {
#    ifdef WINDOWS
        /* ask for best-effort printing to cmd window.  must be called at init. */
        dr_enable_console_printing();
#    endif
        dr_fprintf(STDERR, "Client syscall is running\n");
    }
    initialize_application_buffer();
}


static void
event_exit(void)
{
    close_app_buffer();

    if (!drmgr_unregister_cls_field(event_thread_context_init,
			    	    event_thread_context_exit,
                                    tcls_idx) ||
        !drmgr_unregister_pre_syscall_event(event_pre_syscall) ||
        !drmgr_unregister_post_syscall_event(event_post_syscall))
        DR_ASSERT(false && "failed to unregister");
    drmgr_exit();
}

static void
event_thread_context_init(void *drcontext, bool new_depth)
{
    /* create an instance of our data structure for this thread context */
    per_thread_t *data;
    if (new_depth) {
        data = (per_thread_t *)dr_thread_alloc(drcontext, sizeof(per_thread_t));
        drmgr_set_cls_field(drcontext, tcls_idx, data);
	data->fd = -1;
        // FIXME: typo in the name
        intialize_thread_buffer(1, 2);
    } else {
        data = (per_thread_t *)drmgr_get_cls_field(drcontext, tcls_idx);
    }
}

static void
event_thread_context_exit(void *drcontext, bool thread_exit)
{
    if (!thread_exit)
	    return;
    per_thread_t *data = (per_thread_t *)drmgr_get_cls_field(drcontext, tcls_idx);
    dr_thread_free(drcontext, data, sizeof(per_thread_t));
    close_thread_buffer();
}

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
    per_thread_t *data = (per_thread_t *)drmgr_get_cls_field(drcontext, tcls_idx);
    data->fd = fd; /* store the fd for post-event */

    dr_printf("\033[0;36m[write@%d (%u)]: %.*s\033[0m", fd, len, len, output);
    return true; /* execute normally */
}

static void
event_post_syscall(void *drcontext, int sysnum)
{
    if (sysnum != read_sysnum)
	return;

    per_thread_t *data = (per_thread_t *)drmgr_get_cls_field(drcontext, tcls_idx);
    int fd = data->fd;
    /* dr_syscall_get_param can be called only from pre-event
    int fd = dr_syscall_get_param(drcontext, 0);
    */
    if (fd != STDIN || fd == -1)
	return;
    
    byte *output = (byte *)dr_syscall_get_param(drcontext, 1);
    byte first;
    if (!dr_safe_read(output, 1, &first, NULL)) {
        dr_fprintf(STDERR, "err: cannot read syscall data (read)");
	return;
    }

    int len = dr_syscall_get_result(drcontext);
    if (len > 0) {
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
