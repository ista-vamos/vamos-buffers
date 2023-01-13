cmake_minimum_required(VERSION 3.9)
project(vamos-buffers LANGUAGES C VERSION 0.1)

enable_testing()

set(CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(GNUInstallDirs)

if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "Build type not set. Setting default RelWithDebInfo.")
	set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
endif()

OPTION(ENABLE_IPO "Enable interprocedural optimizations" ON)

if (ENABLE_IPO)
if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	cmake_policy(SET CMP0069 NEW)
	include(CheckIPOSupported)
	check_ipo_supported()
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

	if (NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
		message(WARNING "IPO are used, but the compiler is not clang (compiler = ${CMAKE_C_COMPILER_ID}). "
			        "This may cause troubles when using compilation scripts "
				"that are not based on cmake (e.g., gen/compile.sh).")
	endif()
endif()
endif(ENABLE_IPO)

OPTION(DUMP_STATS "Collect and allow dumping statistics" OFF)
OPTION(RMIND_RINGBUF "Build and use githbu:rmind/ringbuf ring buffer" OFF)

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

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wpedantic -Werror")
add_compile_options(-Wall -Wextra -pedantic)

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

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/shamon)

add_subdirectory(src)
include_directories(${CMAKE_SOURCE_DIR}/src)
link_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(tests)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT shamonCore
    FILE vamos-buffers.cmake
    #NAMESPACE shmn::
    DESTINATION lib/cmake/shamon)
# also export the .cmake file into current build directory
# so that we can use also non-installed builds
export(EXPORT shamonCore FILE ${CMAKE_CURRENT_BINARY_DIR}/cmake/vamos-buffers/vamos-buffers.cmake)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vamos-buffers/vamos-buffers-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/vamos-buffers/vamos-buffers-config.cmake"
  INSTALL_DESTINATION "lib/cmake/vamos-buffers"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )

write_basic_package_version_file(
    "vamos-buffers-version.cmake"
    VERSION ${vamos-buffers_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/vamos-buffers/vamos-buffers-config.cmake"
  "vamos-buffers-version.cmake"
  DESTINATION lib/cmake/vamos-buffers)
