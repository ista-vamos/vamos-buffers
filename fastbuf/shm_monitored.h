#pragma once
#include "shm_common.h"

void intialize_thread_buffer(size_t size_in_pages, buffer_kind kind);
void close_thread_buffer();

void push_event_wait_64(buffer_entry_kind kind, int64_t payload);
void push_event_nowait_64(buffer_entry_kind kind, int64_t payload);