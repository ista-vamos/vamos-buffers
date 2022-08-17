cmake_minimum_required(VERSION 3.1)
project(Shamon C)

include(CTest)

SET (CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	cmake_policy(SET CMP0069 NEW)
	include(CheckIPOSupported)
	check_ipo_supported()
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

OPTION(DUMP_STATS "Collect and allow dumping statistics" OFF)
OPTION(DYNAMORIO_SOURCES "Build sources based on DynamoRIO" ON)
OPTION(RMIND_RINGBUF "Build and use githbu:rmind/ringbuf ring buffer" OFF)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if (DUMP_STATS)
	add_compile_definitions(DUMP_STATS)
endif()

if (RMIND_RINGBUF)
	include(ExternalProject)
	ExternalProject_Add(rmind-ringbuf
			    GIT_REPOSITORY https://github.com/rmind/ringbuf
			    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rmind-ringbuf
			    CONFIGURE_COMMAND ""
			    BUILD_COMMAND make -C src
			    INSTALL_COMMAND ""
			    BUILD_IN_SOURCE 1)

	add_compile_definitions(RMIND_RINGBUF)
endif()

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wpedantic -Werror")
add_compile_options(-Wall -Wextra -pedantic -Werror)

if (DYNAMORIO_SOURCES)
        find_package(DynamoRIO)
        if (NOT DynamoRIO_FOUND)
	        message(FATAL_ERROR
                        "DynamoRIO package required to build DynamoRIO sources")
        endif(NOT DynamoRIO_FOUND)
endif()

add_subdirectory(core)
include_directories(${CMAKE_SOURCE_DIR}/core)
link_directories(${CMAKE_SOURCE_DIR}/core)

add_subdirectory(streams)
add_subdirectory(sources)
add_subdirectory(shmbuf)
add_subdirectory(fastbuf)
add_subdirectory(monitors)
add_subdirectory(experiments)
add_subdirectory(tests)

