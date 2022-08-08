# use debug info in tests
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(ringbufs-bench ringbufs.c)
target_link_libraries(ringbufs-bench shamon-shmbuf shamon-parallel-queue)
target_include_directories(ringbufs-bench PRIVATE ${CMAKE_SOURCE_DIR}
					  PRIVATE ${CMAKE_SOURCE_DIR}/core)


target_include_directories(ringbufs-bench PRIVATE ${CMAKE_SOURCE_DIR}/ringbuf/src)
target_link_libraries(ringbufs-bench ${CMAKE_SOURCE_DIR}/ringbuf/src/ringbuf.o)
