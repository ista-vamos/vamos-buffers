add_subdirectory(streams)
add_subdirectory(shmbuf)
add_subdirectory(core)

add_library(vamos-buffers-client STATIC core/source.c)
target_link_libraries(vamos-buffers-client PUBLIC
	              vamos-buffers-shmbuf vamos-buffers-utils vamos-buffers-event
		      vamos-buffers-queue-spsc vamos-buffers-vector
                      vamos-buffers-list vamos-buffers-string vamos-buffers-ringbuf
		      vamos-buffers-signature)
set_property(TARGET vamos-buffers-client PROPERTY POSITION_INDEPENDENT_CODE 1)


add_library(vamos-buffers-monitor STATIC core/monitor.c)
target_link_libraries(vamos-buffers-monitor PUBLIC
	              vamos-buffers-streams vamos-buffers-stream vamos-buffers-shmbuf
		      vamos-buffers-source vamos-buffers-arbiter vamos-buffers-parallel-queue
                      vamos-buffers-utils vamos-buffers-event vamos-buffers-queue-spsc vamos-buffers-vector
                      vamos-buffers-list vamos-buffers-string vamos-buffers-ringbuf vamos-buffers-signature)
set_property(TARGET vamos-buffers-monitor PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(vamos-buffers-lib SHARED core/shamon.c)
target_compile_definitions(vamos-buffers-lib PUBLIC -D_POSIX_C_SOURCE=200809L)
target_link_libraries(vamos-buffers-lib PUBLIC vamos-buffers-utils vamos-buffers-list vamos-buffers-event
                                        vamos-buffers-queue-spsc vamos-buffers-vector vamos-buffers-string
                                        vamos-buffers-ringbuf vamos-buffers-source vamos-buffers-signature)

add_library(vamos-buffers-static STATIC core/shamon.c)
target_link_libraries(vamos-buffers-static PUBLIC
	              vamos-buffers-utils vamos-buffers-list vamos-buffers-event vamos-buffers-queue-spsc
                      vamos-buffers-vector vamos-buffers-string vamos-buffers-ringbuf vamos-buffers-source
                      vamos-buffers-signature)
install(TARGETS vamos-buffers-lib vamos-buffers-static vamos-buffers-monitor vamos-buffers-client
    EXPORT vamos-buffers_Export
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

