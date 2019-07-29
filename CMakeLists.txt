cmake_minimum_required(VERSION 2.8.9)
project(libmvp)

## Create binaries in the root of the build directory
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add the local cmake directory module path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/gsl-lite/include")

# quazip
find_path(QUAZIP_INCLUDE_DIRS
    NAMES quazip.h
    PATH_SUFFIXES quazip quazip-qt5 quazip5)

include_directories(${QUAZIP_INCLUDE_DIRS})

find_library(QUAZIP_QT5_RELEASE NAMES quazip quazip-qt5 quazip5)
find_library(QUAZIP_QT5_DEBUG NAMES quazipd quazip-qt5d quazip5d)

if (WIN32 OR QUAZIP_QT5_DEBUG)
    set(QUAZIP_LIBRARIES
        optimized ${QUAZIP_QT5_RELEASE}
        debug ${QUAZIP_QT5_DEBUG}
        )
else()
    set(QUAZIP_LIBRARIES ${QUAZIP_QT5_RELEASE})
endif()

message("-- Using QUAZIP_INCLUDE_DIRS=${QUAZIP_INCLUDE_DIRS}")
message("-- Using QUAZIP_LIBRARIES=${QUAZIP_LIBRARIES}")

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(test)
enable_testing()
add_test(NAME libmvp-test COMMAND testrunner)

add_dependencies(testrunner libmvp)

# vim:tw=0
