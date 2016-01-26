cmake_minimum_required(VERSION 3.0.0)
project(mvp)

## Create binaries in the root of the build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add the local cmake directory module path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(external)

get_property(LIBMVP_INCLUDES TARGET libmvp_shared PROPERTY INCLUDE_DIRECTORIES)
#FIXME: this needs a newer cmake version than 3.0.2
# get_property(LIBMVP_SOURCE_DIR TARGET libmvp_shared PROPERTY SOURCE_DIR)
include_directories(${LIBMVP_INCLUDES} "${CMAKE_CURRENT_SOURCE_DIR}/external/libmvp/src")

add_subdirectory(src)

# vim:tw=0

