project(DrShamon)
cmake_minimum_required(VERSION 3.0)

add_library(funs SHARED funs.c events.c
                        ../shmbuf/buffer.c ../shmbuf/shm.c ../shmbuf/client.c)
add_library(calls SHARED calls.c events.c
                        ../shmbuf/buffer.c ../shmbuf/shm.c ../shmbuf/client.c)
find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
	  message(FATAL_ERROR "DynamoRIO package required to build")
endif(NOT DynamoRIO_FOUND)
use_DynamoRIO_extension(funs drmgr)
use_DynamoRIO_extension(funs drsyms)
use_DynamoRIO_extension(funs drreg)
use_DynamoRIO_extension(funs drutil)
configure_DynamoRIO_client(funs)

use_DynamoRIO_extension(calls drmgr)
use_DynamoRIO_extension(calls drsyms)
use_DynamoRIO_extension(calls drreg)
use_DynamoRIO_extension(calls drutil)
use_DynamoRIO_extension(calls  drcontainers)
configure_DynamoRIO_client(calls)

SET (CMAKE_VERBOSE_MAKEFILE 1)
include_directories("${CMAKE_SOURCE_DIR}/../shmbuf")
include_directories("${CMAKE_SOURCE_DIR}/..")

