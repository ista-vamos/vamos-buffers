/* 
 * Based on Code Manipulation API Sample (syscall.c) from DynamoRIO
 */

#include <string.h> /* memset */
#include "dr_api.h"
#include "drmgr.h"

#include "../fastbuf/shm_monitored.h"

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
    void * buf;
    size_t size;
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

// dr_emit_flags_t process_basic_block(void *drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating)
// {
//   instr_t *instr, *next;
//   for (instr = instrlist_first(bb);
//        instr != NULL;
//        instr = next) {
//     next = instr_get_next(instr);
//         #ifdef WINDOWS
//         //ugh...
//         #else
//         if(instr_is_syscall(instr)&&next!=NULL)
//         {
//             instr_t *jump_end  = XINST_CREATE_jump(drcontext,opnd_create_instr(next));
//             instrlist_preinsert(bb, instr, jump_end);

//             instr_t *comp0 = XINST_CREATE_cmp(drcontext,opnd_create_reg(DR_REG_RAX), opnd_create_immed_int64(0));
//             instr_t *comp1 = XINST_CREATE_cmp(drcontext,opnd_create_reg(DR_REG_RAX), opnd_create_immed_int64(1));
//             instr_t *jump_not1 = XINST_CREATE_jump_cond(drcontext, DR_PRED_NE, opnd_create_instr(comp0));
//             instr_t *jump_not0 = XINST_CREATE_jump_cond(drcontext, DR_PRED_NE, opnd_create_instr(instr));
//             instr_t *jump_end1 = XINST_CREATE_jump(drcontext,opnd_create_instr(next));
//             instrlist_preinsert(bb, jump_end, jump_not1);
//         }
//         #endif
//   }
//   return DR_EMIT_DEFAULT;
// }

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
	app_buffer_wait_for_client();
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
    reg_t fd = dr_syscall_get_param(drcontext, 0);
    reg_t buf = dr_syscall_get_param(drcontext, 1);
    reg_t size = dr_syscall_get_param(drcontext, 2);
    per_thread_t *data = (per_thread_t *)drmgr_get_cls_field(drcontext, tcls_idx);
    data->fd = fd; /* store the fd for post-event */
    data->buf=(void*)buf;
    data->size=size;
    // dr_insert_clean_call(drcontext, ilist, nxt, (void *) at_mbr,
    //                     false/*don't need to save fp state*/,
    //                     2 /* 2 parameters */,
    //                     /* opcode is 1st parameter */
    //                     OPND_CREATE_REG(fdreg),
    //                     /* address is 2nd parameter */
    //                     OPND_CREATE_REG(bufreg),
    //                     OPND_CREATE_REG(sizereg)
    //                     );

                        

    // if (sysnum != write_sysnum)
	// return true;

    // int fd = dr_syscall_get_param(drcontext, 0);
    // if (fd != STDERR && fd != STDOUT)
	// return true;
    
    // byte *output = (byte *)dr_syscall_get_param(drcontext, 1);
    // byte first;
    // if (!dr_safe_read(output, 1, &first, NULL)) {
    //     dr_fprintf(STDERR, "err: cannot read syscall data (write)");
    //     return true; /* data unreadable: execute normally */
    // }

    // unsigned int len = dr_syscall_get_param(drcontext, 2);
    // per_thread_t *data = (per_thread_t *)drmgr_get_cls_field(drcontext, tcls_idx);
    // data->fd = fd; /* store the fd for post-event */

    // dr_printf("\033[0;36m[write@%d (%u)]: %.*s\033[0m", fd, len, len, output);
    return true; /* execute normally */
}

static void
event_post_syscall(void *drcontext, int sysnum)
{
    dr_syscall_result_info_t scri;
    scri.size=sizeof(ssize_t);
    scri.use_high=0;
    scri.use_errno=0;
    int success = dr_syscall_get_result_ex(drcontext, &scri);
    // if (sysnum != read_sysnum)
	// return;
    if(sysnum!=read_sysnum&&sysnum!=write_sysnum)
    {
        return;
    }
    per_thread_t *data = (per_thread_t *)drmgr_get_cls_field(drcontext, tcls_idx);
    if(data->fd>2)
    {
        return;
    }
    ssize_t len;
    if(success==0)
    {
        len=data->size;
    }
    else
    {
        len=*((ssize_t*)&scri.value);
    }
    // /* dr_syscall_get_param can be called only from pre-event
    // int fd = dr_syscall_get_param(drcontext, 0);
    // */
    // if (fd != STDIN || fd == -1)
	// return;
    
    // byte *output = (byte *)dr_syscall_get_param(drcontext, 1);
    // byte first;
    // if (!dr_safe_read(output, 1, &first, NULL)) {
    //     dr_fprintf(STDERR, "err: cannot read syscall data (read)");
	// return;
    // }
    // if (len > 0) {
    // 	dr_printf("\033[0;34m[read@%d (%u)]: %.*s\033[0m", fd, len, len, output);
    // }
    printf("Syscall: %i; len: %li; result: %lu\n",sysnum, len, scri.value);
    if(sysnum==read_sysnum)
    {
        push_read(data->fd, data->buf, data->size, len);
    }
    else if(sysnum==write_sysnum)
    {
        push_write(data->fd, data->buf, data->size, len);
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
