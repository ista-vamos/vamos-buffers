project(DrShamonSyscalls)
cmake_minimum_required(VERSION 3.0)

add_library(syscalls SHARED syscalls.c ../fastbuf/shm_monitored.c  ../fastbuf/shm_common.c)
find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
	  message(FATAL_ERROR "DynamoRIO package required to build")
endif(NOT DynamoRIO_FOUND)
use_DynamoRIO_extension(syscalls drmgr)
configure_DynamoRIO_client(syscalls)

SET (CMAKE_VERBOSE_MAKEFILE 1)
include_directories("${CMAKE_SOURCE_DIR}/..")

