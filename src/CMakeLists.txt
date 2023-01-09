set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Boost 1.56 REQUIRED)

# quazip
find_path(QUAZIP_INCLUDE_DIR
    NAMES quazip.h
    PATH_SUFFIXES quazip quazip-qt5 quazip5
    QuaZip-Qt5-1.1/quazip
    QuaZip-Qt5-1.3/quazip
    )

#include_directories(${QUAZIP_INCLUDE_DIR})

find_library(QUAZIP_QT5_RELEASE NAMES quazip quazip-qt5 quazip5 quazip1-qt5)
find_library(QUAZIP_QT5_DEBUG NAMES quazipd quazip-qt5d quazip5d)

if (WIN32 OR QUAZIP_QT5_DEBUG)
    if (QUAZIP_QT5_DEBUG)
        set(QUAZIP_QT5 optimized ${QUAZIP_QT5_RELEASE} debug ${QUAZIP_QT5_DEBUG})
    else()
        set(QUAZIP_QT5 optimized ${QUAZIP_QT5_RELEASE} debug ${QUAZIP_QT5_RELEASE})
    endif()
else()
    set(QUAZIP_QT5 ${QUAZIP_QT5_RELEASE})
endif()

message("-- Using QUAZIP_INCLUDE_DIR=${QUAZIP_INCLUDE_DIR}")
message("-- Using QUAZIP_QT5=${QUAZIP_QT5}")

include_directories(${Boost_INCLUDE_DIRS} ${QUAZIP_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

set(LIBMVP_SOURCES
    file_dialog.cc
    firmware.cc
    firmware_ops.cc
    flash_address.cc
    flash.cc
    flash_widget.cc
    instruction_file.cc
    instruction_interpreter.cc
    mdpp16.cc
    port_helper.cc
    util.cc
    )

qt5_wrap_ui(UIC_OUT flash_widget.ui)
qt5_add_resources(RC_OUT resources.qrc)

set(LIBMVP_TARGET "libmvp")

add_library(${LIBMVP_TARGET} ${LIBMVP_SOURCES} ${UIC_OUT} ${RC_OUT})

set_target_properties(${LIBMVP_TARGET} PROPERTIES
  OUTPUT_NAME "mvp"
#  INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
#  LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}
  )

qt5_use_modules(${LIBMVP_TARGET}
  Concurrent
  Gui
  SerialPort
  Widgets
  )

target_link_libraries(${LIBMVP_TARGET} ${QUAZIP_QT5})

include(GenerateExportHeader)
generate_export_header(${LIBMVP_TARGET})

install(TARGETS ${LIBMVP_TARGET} DESTINATION lib)

# vim:tw=0
