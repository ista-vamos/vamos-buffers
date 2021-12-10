project(DrShamon)
cmake_minimum_required(VERSION 3.0)

add_library(funs SHARED funs.c ../shm.c)
find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
	  message(FATAL_ERROR "DynamoRIO package required to build")
endif(NOT DynamoRIO_FOUND)
use_DynamoRIO_extension(funs drmgr)
use_DynamoRIO_extension(funs drsyms)
configure_DynamoRIO_client(funs)

SET (CMAKE_VERBOSE_MAKEFILE 1)
include_directories("${CMAKE_SOURCE_DIR}/..")

