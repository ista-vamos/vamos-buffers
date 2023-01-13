add_library(vamos-buffers-shmbuf STATIC buffer.c buffer-local.c buffer-aux.c
                                 buffer-sub.c buffer-control.c
	                         shm.c client.c utils.c)
target_include_directories(vamos-buffers-shmbuf PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_definitions(vamos-buffers-shmbuf PUBLIC -D_POSIX_C_SOURCE=200809L)
set_property(TARGET vamos-buffers-shmbuf PROPERTY POSITION_INDEPENDENT_CODE 1)

install(TARGETS vamos-buffers-shmbuf
    EXPORT vamos-buffers_Export
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

