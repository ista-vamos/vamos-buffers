#pragma once
#include "shm_common.h"

void initialize_application_buffer();
void app_buffer_wait_for_client();
void intialize_thread_buffer(size_t size_in_pages, buffer_kind kind);
void close_thread_buffer();
void close_app_buffer();

void push_event_wait_64(buffer_entry_kind kind, int64_t payload);
void push_event_nowait_64(buffer_entry_kind kind, int64_t payload);
void push_event_wait_32(buffer_entry_kind kind, int32_t payload);
void push_event_nowait_32(buffer_entry_kind kind, int32_t payload);