add_library(shamon-fastbuf STATIC shm_common.c shm_monitor.c)
target_include_directories(shamon-fastbuf PRIVATE
                           ${CMAKE_SOURCE_DIR})
target_compile_definitions(shamon-fastbuf PUBLIC -D_POSIX_C_SOURCE=200809L)
target_compile_options(shamon-fastbuf PUBLIC -fPIC)

