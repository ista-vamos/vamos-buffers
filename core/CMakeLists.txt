include_directories(${CMAKE_SOURCE_DIR})

add_library(shamon-utils STATIC utils.c)
target_compile_definitions(shamon-utils PUBLIC -D_POSIX_C_SOURCE=200809L)
target_compile_options(shamon-utils PRIVATE -fPIC)

add_library(shamon-list OBJECT list.c)
add_library(shamon-event OBJECT event.c)
add_library(shamon-queue-spsc OBJECT queue_spsc.c)

add_library(shamon-vector STATIC vector.c vector-aligned.c)
target_link_libraries(shamon-vector PRIVATE shamon-utils)

add_library(shamon-string STATIC shm_string.c)

add_library(shamon-source OBJECT source.c)
add_library(shamon-monitor STATIC monitor.c)

target_compile_options(shamon-source PRIVATE -fPIC)
target_link_libraries(shamon-source PRIVATE shamon-signature)

add_library(shamon-stream STATIC stream.c)
target_link_libraries(shamon-stream PRIVATE shamon-shmbuf)
target_compile_definitions(shamon-stream PUBLIC -D_POSIX_C_SOURCE=200809L)

add_library(shamon-arbiter STATIC shamon.c arbiter.c)
target_link_libraries(shamon-arbiter PRIVATE shamon-event shamon-stream PUBLIC shamon-parallel-queue)
target_compile_options(shamon-arbiter PRIVATE -fPIC)
target_compile_definitions(shamon-arbiter PUBLIC -D_POSIX_C_SOURCE=200809L)

add_library(shamon-signature STATIC signatures.c)
target_compile_options(shamon-signature PRIVATE -fPIC)

add_library(shamon-ringbuf OBJECT spsc_ringbuf.c)
add_library(shamon-parallel-queue STATIC par_queue.c)
target_link_libraries(shamon-parallel-queue PUBLIC shamon-ringbuf)

add_library(shamon STATIC shamon.c)
target_link_libraries(shamon PRIVATE shamon-vector)

add_library(shamon-lib SHARED arbiter.c)
target_compile_definitions(shamon-lib PUBLIC -D_POSIX_C_SOURCE=200809L)
target_link_libraries(shamon-lib PUBLIC shamon-parallel-queue
		                 PUBLIC shamon-stream
		                 PUBLIC shamon-event
		                 PUBLIC shamon-shmbuf)

