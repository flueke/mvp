cmake_minimum_required(VERSION 3.0.0)
project(libmvp)

## Create binaries in the root of the build directory
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add the local cmake directory module path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/gsl-lite/include")

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(test)
enable_testing()
add_test(NAME libmvp-test COMMAND testrunner)

add_dependencies(testrunner libmvp_shared)

# vim:tw=0
