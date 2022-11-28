#pragma once
#include "shm_common.h"

struct monitor_buffer;
typedef struct monitor_buffer *monitor_buffer;
struct monitored_process;
typedef struct monitored_process *monitored_process;

monitored_process
attach_to_process(pid_t pid,
                  int (*register_thread_buffer)(monitor_buffer buffer));
// the same as attach_to_process but does not create the event loop that handles
// the events
monitored_process
       only_attach_to_process(pid_t pid,
                              int (*register_thread_buffer)(monitor_buffer buffer));
void   wait_for_process(monitored_process proc);
size_t copy_events_wait(monitor_buffer buffer, buffer_entry *buffer_buffer,
                        size_t count);
size_t copy_events_nowait(monitor_buffer buffer, buffer_entry *buffer_buffer,
                          size_t count);
