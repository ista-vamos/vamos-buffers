add_library(funs-lib STATIC events.c)
target_compile_options(funs-lib PRIVATE -fPIC)

add_library(funs SHARED funs.c)
target_link_libraries(funs funs-lib shamon-shmbuf shamon-list)
target_compile_options(funs PUBLIC -Wno-pedantic -Wno-missing-field-initializers)

use_DynamoRIO_extension(funs drmgr)
use_DynamoRIO_extension(funs drsyms)
use_DynamoRIO_extension(funs drreg)
use_DynamoRIO_extension(funs drutil)
configure_DynamoRIO_client(funs)

include_directories("${CMAKE_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/shmbuf")
include_directories("${CMAKE_SOURCE_DIR}/streams")

