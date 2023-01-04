add_library(shamon-shmbuf STATIC buffer.c buffer-local.c buffer-aux.c
                                 buffer-sub.c buffer-control.c
	                         shm.c client.c utils.c)
target_include_directories(shamon-shmbuf PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_definitions(shamon-shmbuf PUBLIC -D_POSIX_C_SOURCE=200809L)
set_property(TARGET shamon-shmbuf PROPERTY POSITION_INDEPENDENT_CODE 1)

install(TARGETS shamon-shmbuf
    EXPORT shamonCore
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

install(FILES client.h buffer.h
	DESTINATION include/shamon/shmbuf)
