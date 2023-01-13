add_library(vamos-buffers-streams STATIC
            streams.c
            stream-regex.c
            stream-regexrw.c
            stream-drregex.c
            stream-generic.c
            stream-funs.c)
target_link_libraries(vamos-buffers-streams PUBLIC vamos-buffers-shmbuf vamos-buffers-stream)
set_property(TARGET vamos-buffers-streams PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(vamos-buffers-streams PRIVATE
                           ${CMAKE_SOURCE_DIR}
                           ${CMAKE_SOURCE_DIR}/shmbuf)
install(TARGETS vamos-buffers-streams
    EXPORT vamos-buffers_Export
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

