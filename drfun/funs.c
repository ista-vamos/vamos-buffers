#include <string.h>
#include "dr_api.h"
#include "drmgr.h"
#include "drsyms.h"

#ifdef WINDOWS
#    define IF_WINDOWS_ELSE(x, y) x
#else
#    define IF_WINDOWS_ELSE(x, y) y
#endif

#ifdef WINDOWS
#    define DISPLAY_STRING(msg) dr_messagebox(msg)
#else
#    define DISPLAY_STRING(msg) dr_printf("%s\n", msg);
#endif


static void
event_exit(void);
static void
wrap_pre(void *wrapcxt, OUT void **user_data);
static void
wrap_post(void *wrapcxt, void *user_data);


#if 0
static bool register_symbol(const char *name, size_t modoffs, void *data)
{
	dr_fprintf(STDOUT, "%s: %lu\n", name, modoffs);
	return true; /* continue enumeration */
}

static void
module_load_event(void *drcontext, const module_data_t *mod, bool loaded)
{
	drsym_error_t status = drsym_enumerate_symbols(mod->full_path,
						       register_symbol,
						       /*data = */ NULL,
						       /* flags = */ 0U);
	if (status != DRSYM_SUCCESS) {
		dr_fprintf(STDERR, "Failed enumerating symbols\n");
		return;
	}
	drsym_free_resources(mod->full_path);
}
#endif

static bool register_symbol(drsym_info_t *info, drsym_error_t status, void *data)
{
	if (strncmp(info->name, "foo", 3) != 0)
		return true;
	dr_fprintf(STDOUT, "%s: %lu\n", info->name, info->start_offs);
	return true; /* continue enumeration */
}

static void
module_load_event(void *drcontext, const module_data_t *mod, bool loaded)
{
	drsym_error_t status = drsym_enumerate_symbols_ex(mod->full_path,
							  register_symbol,
							  sizeof(drsym_info_t),
							  /*data = */ NULL,
							  /* flags = */ 0U);
#if 0
	/* full = false means search only functions */
	drsym_error_t status  = drsym_search_symbols(mod->full_path,
						     NULL,
						     /* full = */ false,
						     register_symbol,
						     /*data = */ NULL);
#endif
	if (status != DRSYM_SUCCESS) {
		dr_fprintf(STDERR, "Failed enumerating symbols\n");
		return;
	}
	drsym_free_resources(mod->full_path);
}



DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[])
{
    dr_set_client_name("DynamoRIO 'funs wo'", "---");
    /* make it easy to tell, by looking at log file, which client executed */
    dr_log(NULL, DR_LOG_ALL, 1, "Client 'funs' initializing\n");
    /* also give notification to stderr */
    if (dr_is_notify_on()) {
        dr_fprintf(STDERR, "Client FUNS is running\n");
    }

    drmgr_init();
    drsym_init(IF_WINDOWS_ELSE(NULL, 0));

    dr_register_exit_event(event_exit);
    drmgr_register_module_load_event(module_load_event);
}

static void
event_exit(void)
{
	drsym_exit();
	drmgr_exit();
}

