# use debug info in tests
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(ringbufs-bench ringbufs.c)
target_link_libraries(ringbufs-bench vamos-buffers-shmbuf vamos-buffers-parallel-queue vamos-buffers-queue-spsc vamos-buffers-ringbuf pthread)
target_include_directories(ringbufs-bench PRIVATE ${CMAKE_SOURCE_DIR}
					  PRIVATE ${CMAKE_SOURCE_DIR}/core)
target_compile_definitions(ringbufs-bench PUBLIC -D_POSIX_C_SOURCE=200809L)

if (RMIND_RINGBUF)
	target_include_directories(ringbufs-bench PRIVATE ${CMAKE_SOURCE_DIR}/rmind-ringbuf/src)
	target_link_libraries(ringbufs-bench ${CMAKE_SOURCE_DIR}/rmind-ringbuf/src/ringbuf.o)
	add_dependencies(ringbufs-bench rmind-ringbuf)
endif()
