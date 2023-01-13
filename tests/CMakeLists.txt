# use debug info in tests
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_subdirectory(bench)

enable_testing()

include(ProcessorCount)
ProcessorCount(N)
if(N EQUAL 0)
    set(N 1)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

set(CTEST_OPTS -j${N} --output-on-failure --progress ${CTEST_OPTS})

find_program(CBMC_EXE "cbmc")

add_executable(list-test list-test.c)
target_link_libraries(list-test vamos-buffers-list)
target_include_directories(list-test PRIVATE ${CMAKE_SOURCE_DIR})
add_test(list-test list-test)

add_executable(shmbuffer-test buffer-test.c)
target_link_libraries(shmbuffer-test vamos-buffers-shmbuf vamos-buffers-source vamos-buffers-ringbuf vamos-buffers-utils vamos-buffers-signature vamos-buffers-event vamos-buffers-list)
target_include_directories(shmbuffer-test PRIVATE ${CMAKE_SOURCE_DIR})
add_test(shmbuffer-test shmbuffer-test)

add_executable(spsc-ringbuf-1 spsc-ringbuf-1.c)
target_link_libraries(spsc-ringbuf-1 vamos-buffers-ringbuf)
target_include_directories(spsc-ringbuf-1 PRIVATE ${CMAKE_SOURCE_DIR})

add_executable(queue-par-test queue-par-test.c)
target_link_libraries(queue-par-test vamos-buffers-parallel-queue vamos-buffers-ringbuf pthread)
target_include_directories(queue-par-test PRIVATE ${CMAKE_SOURCE_DIR})
add_test(queue-par-test queue-par-test REPEAT 1000)

add_executable(queue-par-test-2 queue-par-test-2.c)
target_link_libraries(queue-par-test-2 vamos-buffers-parallel-queue vamos-buffers-ringbuf pthread)
target_include_directories(queue-par-test-2 PRIVATE ${CMAKE_SOURCE_DIR})
add_test(queue-par-test-2 queue-par-test-2 REPEAT 1000)

add_executable(queue-par-test-3 queue-par-cbmc_2.c)
target_link_libraries(queue-par-test-3 vamos-buffers-parallel-queue vamos-buffers-ringbuf pthread)
target_include_directories(queue-par-test-3 PRIVATE ${CMAKE_SOURCE_DIR})
add_test(queue-par-test-3 queue-par-test-3 REPEAT 1000)

if (CBMC_EXE)
add_test(NAME queue-par-cbmc
         COMMAND cbmc -I${CMAKE_SOURCE_DIR}/include --c11 --unwind 2
	         ${CMAKE_SOURCE_DIR}/tests/queue-par-cbmc.c
                 ${CMAKE_SOURCE_DIR}/src/core/par_queue.c
                 ${CMAKE_SOURCE_DIR}/src/core/spsc_ringbuf.c)


add_test(NAME queue-par-cbmc-2
         COMMAND cbmc -I${CMAKE_SOURCE_DIR}/include --c11 --unwind 2
	         ${CMAKE_SOURCE_DIR}/tests/queue-par-cbmc_2.c
                 ${CMAKE_SOURCE_DIR}/src/core/par_queue.c
                 ${CMAKE_SOURCE_DIR}/src/core/spsc_ringbuf.c)

add_test(NAME queue-par-cbmc-3
         COMMAND cbmc -I${CMAKE_SOURCE_DIR}/include --c11 --unwind 10
	         ${CMAKE_SOURCE_DIR}/tests/queue-par-cbmc_3.c
                 ${CMAKE_SOURCE_DIR}/src/core/par_queue.c
                 ${CMAKE_SOURCE_DIR}/src/core/spsc_ringbuf.c)

add_test(NAME spsc-ringbuf-cbmc-1
         COMMAND cbmc -I${CMAKE_SOURCE_DIR}/include --c11 --unwind 4
	         ${CMAKE_SOURCE_DIR}/tests/spsc-ringbuf-cbmc-1.c
                 ${CMAKE_SOURCE_DIR}/src/core/spsc_ringbuf.c)
 endif(CBMC_EXE)

add_executable(arbiter-buffer-test arbiter-buffer.c)
target_link_libraries(arbiter-buffer-test vamos-buffers-arbiter vamos-buffers-stream vamos-buffers-shmbuf vamos-buffers-parallel-queue vamos-buffers-ringbuf vamos-buffers-list vamos-buffers-source vamos-buffers-signature vamos-buffers-event vamos-buffers-utils)
target_include_directories(arbiter-buffer-test PRIVATE ${CMAKE_SOURCE_DIR})
add_test(arbiter-buffer-test arbiter-buffer-test)


add_executable(fetch-test fetch-test.c)
target_link_libraries(fetch-test vamos-buffers-arbiter vamos-buffers-parallel-queue vamos-buffers-ringbuf vamos-buffers-stream vamos-buffers-shmbuf vamos-buffers-source vamos-buffers-list vamos-buffers-signature vamos-buffers-event vamos-buffers-utils pthread)
target_include_directories(fetch-test PRIVATE ${CMAKE_SOURCE_DIR})
add_test(fetch-test fetch-test REPEAT 20)

add_executable(fetch-test-2 fetch-test-2.c)
target_link_libraries(fetch-test-2 vamos-buffers-arbiter vamos-buffers-parallel-queue vamos-buffers-ringbuf vamos-buffers-stream vamos-buffers-shmbuf vamos-buffers-source vamos-buffers-list vamos-buffers-signature vamos-buffers-event vamos-buffers-utils pthread)
target_include_directories(fetch-test-2 PRIVATE ${CMAKE_SOURCE_DIR})
add_test(fetch-test-2 fetch-test-2 REPEAT 20)

add_executable(fetch-test-3 fetch-test-3.c)
target_link_libraries(fetch-test-3 vamos-buffers-arbiter vamos-buffers-parallel-queue vamos-buffers-ringbuf vamos-buffers-stream vamos-buffers-shmbuf vamos-buffers-source vamos-buffers-list vamos-buffers-signature vamos-buffers-event vamos-buffers-utils pthread)
target_include_directories(fetch-test-3 PRIVATE ${CMAKE_SOURCE_DIR})
add_test(fetch-test-3 fetch-test-3 REPEAT 20)
