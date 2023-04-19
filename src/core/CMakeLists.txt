add_library(vamos-buffers-utils          STATIC utils.c)
add_library(vamos-buffers-list           STATIC list.c list-embedded.c)
add_library(vamos-buffers-event          STATIC event.c)
add_library(vamos-buffers-queue-spsc     STATIC queue_spsc.c)
add_library(vamos-buffers-vector         STATIC vector.c vector-aligned.c)
add_library(vamos-buffers-string         STATIC vms_string.c)
add_library(vamos-buffers-source         STATIC source.c)
add_library(vamos-buffers-stream         STATIC stream.c)
add_library(vamos-buffers-arbiter        STATIC arbiter.c)
add_library(vamos-buffers-signature      STATIC signatures.c)
add_library(vamos-buffers-ringbuf        STATIC spsc_ringbuf.c)
add_library(vamos-buffers-parallel-queue STATIC par_queue.c)
add_library(vamos-buffers-shamon  STATIC shamon.c)
add_library(vamos-buffers-monitor-buffer STATIC monitor.c)

set_property(TARGET vamos-buffers-utils     PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET vamos-buffers-source    PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET vamos-buffers-arbiter   PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET vamos-buffers-signature PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET vamos-buffers-stream    PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET vamos-buffers-shamon    PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET vamos-buffers-monitor-buffer
	                             PROPERTY POSITION_INDEPENDENT_CODE 1)
target_compile_definitions(vamos-buffers-utils   PRIVATE -D_POSIX_C_SOURCE=200809L)
target_compile_definitions(vamos-buffers-stream  PRIVATE -D_POSIX_C_SOURCE=200809L)
target_compile_definitions(vamos-buffers-arbiter PRIVATE -D_POSIX_C_SOURCE=200809L)
target_compile_definitions(vamos-buffers-shamon  PRIVATE -D_POSIX_C_SOURCE=200809L)

install(TARGETS vamos-buffers-utils vamos-buffers-list vamos-buffers-event vamos-buffers-queue-spsc
                vamos-buffers-vector vamos-buffers-string vamos-buffers-ringbuf vamos-buffers-source
		vamos-buffers-signature vamos-buffers-stream vamos-buffers-arbiter vamos-buffers-parallel-queue
		vamos-buffers-shamon
    EXPORT vamos-buffers_Export
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

