project(DrShamon)
cmake_minimum_required(VERSION 3.0)

SET (CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -flto")

add_library(funs SHARED funs.c events.c
                        ../list.c)
target_link_libraries(funs "${CMAKE_SOURCE_DIR}/../shmbuf/libshmbuf.a")

find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
	  message(FATAL_ERROR "DynamoRIO package required to build")
endif(NOT DynamoRIO_FOUND)

use_DynamoRIO_extension(funs drmgr)
use_DynamoRIO_extension(funs drsyms)
use_DynamoRIO_extension(funs drreg)
use_DynamoRIO_extension(funs drutil)
configure_DynamoRIO_client(funs)


# add_library(calls SHARED calls.c events.c
#                         ../vector.c)
# target_link_libraries(calls PRIVATE ../shmbuf/libshmbuf.a)
#
# use_DynamoRIO_extension(calls drmgr)
# use_DynamoRIO_extension(calls drsyms)
# use_DynamoRIO_extension(calls drreg)
# use_DynamoRIO_extension(calls drutil)
# use_DynamoRIO_extension(calls  drcontainers)
# configure_DynamoRIO_client(calls)

include_directories("${CMAKE_SOURCE_DIR}/../shmbuf")
include_directories("${CMAKE_SOURCE_DIR}/../streams")
include_directories("${CMAKE_SOURCE_DIR}/..")

