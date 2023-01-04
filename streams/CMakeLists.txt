add_library(shamon-streams STATIC
            streams.c
            stream-regex.c
            stream-regexrw.c
            stream-drregex.c
            stream-generic.c
            stream-funs.c)
target_link_libraries(shamon-streams PUBLIC shamon-shmbuf shamon-stream)
set_property(TARGET shamon-streams PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(shamon-streams PRIVATE
                           ${CMAKE_SOURCE_DIR}
                           ${CMAKE_SOURCE_DIR}/shmbuf)
