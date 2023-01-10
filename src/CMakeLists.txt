add_subdirectory(streams)
add_subdirectory(shmbuf)
add_subdirectory(core)

add_library(shamon-client STATIC core/source.c)
target_link_libraries(shamon-client PUBLIC
	              shamon-shmbuf shamon-utils shamon-event
		      shamon-queue-spsc shamon-vector
                      shamon-list shamon-string shamon-ringbuf
		      shamon-signature)
set_property(TARGET shamon-client PROPERTY POSITION_INDEPENDENT_CODE 1)


add_library(shamon-monitor STATIC core/monitor.c)
target_link_libraries(shamon-monitor PUBLIC
	              shamon-streams shamon-stream shamon-shmbuf
		      shamon-source shamon-arbiter shamon-parallel-queue
                      shamon-utils shamon-event shamon-queue-spsc shamon-vector
                      shamon-list shamon-string shamon-ringbuf shamon-signature)
set_property(TARGET shamon-monitor PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(shamon-lib SHARED core/shamon.c)
target_compile_definitions(shamon-lib PUBLIC -D_POSIX_C_SOURCE=200809L)
target_link_libraries(shamon-lib PUBLIC shamon-utils shamon-list shamon-event
                                        shamon-queue-spsc shamon-vector shamon-string
                                        shamon-ringbuf shamon-source shamon-signature)

add_library(shamon-static STATIC core/shamon.c)
target_link_libraries(shamon-static PUBLIC
	              shamon-utils shamon-list shamon-event shamon-queue-spsc
                      shamon-vector shamon-string shamon-ringbuf shamon-source
                      shamon-signature)
install(TARGETS shamon-lib shamon-static shamon-monitor shamon-client
    EXPORT shamonCore
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

