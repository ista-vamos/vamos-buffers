add_library(shamon-shmbuf STATIC buffer.c shm.c client.c)
target_include_directories(shamon-shmbuf PRIVATE
                           ${CMAKE_SOURCE_DIR})
target_compile_definitions(shamon-shmbuf PUBLIC -D_POSIX_C_SOURCE=200809L)
target_compile_options(shamon-shmbuf PUBLIC -fPIC)
target_link_libraries(shamon-shmbuf PRIVATE shamon-utils shamon-list shamon-ringbuf
					    shamon-source shamon-signature shamon-event)

install(TARGETS shamon-shmbuf
    EXPORT shamonCore
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

install(FILES client.h buffer.h
	DESTINATION include/shamon/shmbuf)
