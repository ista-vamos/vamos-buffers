cmake_minimum_required(VERSION 3.1)
project(shamon LANGUAGES C VERSION 0.1)

enable_testing()

include(ExternalProject)

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
OPTION(BPF_SOURCES "Build and use eBPF sources" OFF)
OPTION(LLVM_SOURCES "Build and use LLVM sources" ON)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Here we can setup the responsivity of buffers:
# If there is no event on a buffer, we first busy wait
# BUSY_WAIT_FOR_EVENTS times. Then we suspend the thread
# for SLEEP_TIME_INIT_NS seconds, then two times longer,
# etc. up the the SLEEP_TIME_THRES_NS. If no event
# still came, we keep suspending the thread using
# the SLEEP_TIME_THRES_NS duration.
add_compile_definitions(BUSY_WAIT_FOR_EVENTS=1000)
add_compile_definitions(SLEEP_TIME_INIT_NS=10)
add_compile_definitions(SLEEP_TIME_THRES_NS=30000)

if (DUMP_STATS)
	add_compile_definitions(DUMP_STATS)
endif()

if (RMIND_RINGBUF)
	ExternalProject_Add(rmind-ringbuf
			    GIT_REPOSITORY https://github.com/rmind/ringbuf
			    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rmind-ringbuf
			    CONFIGURE_COMMAND ""
			    BUILD_COMMAND make -C src
			    INSTALL_COMMAND ""
			    BUILD_IN_SOURCE 1)

	add_compile_definitions(RMIND_RINGBUF)
endif()

if (BPF_SOURCES)
	ExternalProject_Add(bcc
			    GIT_REPOSITORY https://github.com/mchalupa/bcc
			    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bcc
                            GIT_TAG readwrite
			    INSTALL_COMMAND ""
			    BUILD_IN_SOURCE 1)

	add_compile_definitions(BPF_SOURCES)
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
add_subdirectory(monitors)
add_subdirectory(experiments)
add_subdirectory(tests)

install(EXPORT shamonCore
    FILE shamon.cmake
    #NAMESPACE shmn::
    DESTINATION lib/cmake/shamon)

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/shamonConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/shamonConfig.cmake"
  INSTALL_DESTINATION "lib/cmake/shamon"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )

write_basic_package_version_file(
    "shamonVersion.cmake"
    VERSION ${shamon_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/shamonConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/shamonVersion.cmake"
  DESTINATION lib/cmake/shamon)
