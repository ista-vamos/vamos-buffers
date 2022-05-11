add_library(shamon-streams STATIC
            streams.c
            stream-regex.c
            #stream-fds.c
            stream-funs.c)
target_link_libraries(shamon-streams INTERFACE shamon-signature)
target_include_directories(shamon-streams PRIVATE
                           ${CMAKE_SOURCE_DIR}
                           ${CMAKE_SOURCE_DIR}/shmbuf)
