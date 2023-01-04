include_directories(${CMAKE_SOURCE_DIR})

add_library(shamon-utils          STATIC utils.c)
add_library(shamon-list           STATIC list.c list-embedded.c)
add_library(shamon-event          STATIC event.c)
add_library(shamon-queue-spsc     STATIC queue_spsc.c)
add_library(shamon-vector         STATIC vector.c vector-aligned.c)
add_library(shamon-string         STATIC shm_string.c)
add_library(shamon-source         STATIC source.c)
add_library(shamon-stream         STATIC stream.c)
add_library(shamon-arbiter        STATIC arbiter.c)
add_library(shamon-signature      STATIC signatures.c)
add_library(shamon-ringbuf        STATIC spsc_ringbuf.c)
add_library(shamon-parallel-queue STATIC par_queue.c)
add_library(shamon-shamon         STATIC shamon.c)

set_property(TARGET shamon-utils     PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET shamon-source    PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET shamon-arbiter   PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET shamon-signature PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET shamon-stream    PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET shamon-shamon    PROPERTY POSITION_INDEPENDENT_CODE 1)
target_compile_definitions(shamon-utils   PRIVATE -D_POSIX_C_SOURCE=200809L)
target_compile_definitions(shamon-stream  PRIVATE -D_POSIX_C_SOURCE=200809L)
target_compile_definitions(shamon-arbiter PRIVATE -D_POSIX_C_SOURCE=200809L)
target_compile_definitions(shamon-shamon  PRIVATE -D_POSIX_C_SOURCE=200809L)

add_library(shamon-lib SHARED shamon.c)
target_compile_definitions(shamon-lib PUBLIC -D_POSIX_C_SOURCE=200809L)
target_link_libraries(shamon-lib PUBLIC shamon-utils shamon-list shamon-event
                                        shamon-queue-spsc shamon-vector shamon-string
                                        shamon-ringbuf shamon-source shamon-signature)

add_library(shamon-static STATIC shamon.c)
target_link_libraries(shamon-static PUBLIC shamon-utils shamon-list shamon-event shamon-queue-spsc
                                           shamon-vector shamon-string shamon-ringbuf shamon-source
                                           shamon-signature)

install(TARGETS shamon-lib shamon-static
                shamon-utils shamon-list shamon-event shamon-queue-spsc
                shamon-vector shamon-string shamon-ringbuf shamon-source shamon-signature
    EXPORT shamonCore
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

install(FILES shamon.h arbiter.h stream.h event.h spsc_ringbuf.h par_queue.h signatures.h
	DESTINATION include/shamon/core)
