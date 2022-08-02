# use debug info in tests
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

enable_testing()

include(ProcessorCount)
ProcessorCount(N)
if(N EQUAL 0)
    set(N 1)
endif()

set(CTEST_OPTS -j${N} --output-on-failure --progress ${CTEST_OPTS})
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} ${CTEST_OPTS}
                        USES_TERMINAL)

add_executable(queue-test queue-test.c)
target_link_libraries(queue-test shamon-queue)
target_include_directories(queue-test PRIVATE ${CMAKE_SOURCE_DIR})
add_test(queue-test queue-test)

add_executable(list-test list-test.c)
target_link_libraries(list-test shamon-list)
target_include_directories(list-test PRIVATE ${CMAKE_SOURCE_DIR})
add_test(list-test list-test)

add_executable(shmbuffer-test buffer-test.c)
target_link_libraries(shmbuffer-test shamon-shmbuf shamon-list)
target_include_directories(shmbuffer-test PRIVATE ${CMAKE_SOURCE_DIR})
add_test(shmbuffer-test shmbuffer-test)

add_executable(queue-par-test queue-par-test.c)
target_link_libraries(queue-par-test shamon-parallel-queue pthread)
target_include_directories(queue-par-test PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME queue-par-test
         COMMAND sh -c -e "for i in `seq 1 1000`;do ./queue-par-test; done"
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)

add_executable(queue-par-test-2 queue-par-test-2.c)
target_link_libraries(queue-par-test-2 shamon-parallel-queue pthread)
target_include_directories(queue-par-test-2 PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME queue-par-test-2
         COMMAND sh -c -e "for i in `seq 1 1000`;do ./queue-par-test-2; done"
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)

add_executable(queue-par-test-3 queue-par-cbmc_2.c)
target_link_libraries(queue-par-test-3 shamon-parallel-queue pthread)
target_include_directories(queue-par-test-3 PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME queue-par-test-3
         COMMAND sh -c -e "for i in `seq 1 1000`;do ./queue-par-test-3; done"
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)

add_test(NAME queue-par-cbmc
         COMMAND cbmc -I${CMAKE_SOURCE_DIR} --c11 --unwind 2
	         queue-par-cbmc.c ${CMAKE_SOURCE_DIR}/core/parallel_queue.c)

add_test(NAME queue-par-cbmc-2
         COMMAND cbmc -I${CMAKE_SOURCE_DIR} --c11 --unwind 2
	         queue-par-cbmc_2.c ${CMAKE_SOURCE_DIR}/core/parallel_queue.c)

add_executable(fetch-test fetch-test.c)
target_link_libraries(fetch-test shamon-shmbuf shamon-arbiter shamon-parallel-queue pthread)
target_include_directories(fetch-test PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME fetch-test
         COMMAND sh -c -e "for i in `seq 1 20`;do ./fetch-test; done"
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)

add_executable(fetch-test-2 fetch-test-2.c)
target_link_libraries(fetch-test-2 shamon-shmbuf shamon-arbiter shamon-parallel-queue pthread)
target_include_directories(fetch-test-2 PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME fetch-test-2
         COMMAND sh -c -e "for i in `seq 1 20`;do ./fetch-test-2; done"
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)

add_executable(fetch-test-3 fetch-test-3.c)
target_link_libraries(fetch-test-3 shamon-shmbuf shamon-arbiter shamon-parallel-queue pthread)
target_include_directories(fetch-test-3 PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME fetch-test-3
         COMMAND sh -c -e "for i in `seq 1 20`;do ./fetch-test-3; done"
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)

