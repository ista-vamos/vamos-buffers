cmake_minimum_required(VERSION 3.1)
project(Shamon C)

include(CTest)

SET (CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
# we need POSIX

cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)
check_ipo_supported()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

#if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
#    check_ipo_supported(RESULT IPO)
#endif()

OPTION(DYNAMORIO_SOURCES "Build sources based on DynamoRIO" ON)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wpedantic -Werror")
add_compile_options(-Wall -Wextra -pedantic -Werror)

if (DYNAMORIO_SOURCES)
        find_package(DynamoRIO)
        if (NOT DynamoRIO_FOUND)
	        message(FATAL_ERROR
                        "DynamoRIO package required to build DynamoRIO sources")
        endif(NOT DynamoRIO_FOUND)
endif()

add_library(shamon-list OBJECT list.c)
add_library(shamon-vector OBJECT vector.c)
add_library(shamon-queue OBJECT queue.c)
add_library(shamon-source OBJECT source.c)

add_library(shamon-utils STATIC utils.c)
target_compile_definitions(shamon-utils PUBLIC -D_POSIX_C_SOURCE=200809L)
add_library(shamon-parallel-queue STATIC parallel_queue.c)
add_library(shamon-arbiter STATIC shamon.c arbiter.c event.c stream.c)
target_compile_options(shamon-arbiter PRIVATE -fPIC)
target_compile_definitions(shamon-arbiter PUBLIC -D_POSIX_C_SOURCE=200809L)
add_library(shamon-signature STATIC signatures.c)
target_compile_options(shamon-signature PRIVATE -fPIC)

add_library(shamon STATIC shamon.c)

add_subdirectory(streams)
add_subdirectory(sources)
add_subdirectory(shmbuf)
add_subdirectory(fastbuf)

add_library(shamon-lib SHARED arbiter.c event.c stream.c)
target_compile_definitions(shamon-lib PUBLIC -D_POSIX_C_SOURCE=200809L)
target_link_libraries(shamon-lib PUBLIC shamon-parallel-queue
		                        shamon-signature
		                        shamon-shmbuf)

add_subdirectory(experiments)
add_subdirectory(tests)

