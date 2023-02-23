set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

find_package(Qt5 COMPONENTS Concurrent Gui Network SerialPort Widgets)
find_package(Boost 1.56 REQUIRED)

# quazip
find_path(QUAZIP_INCLUDE_DIR
    NAMES quazip.h
    PATH_SUFFIXES quazip quazip-qt5 quazip5
    QuaZip-Qt5-1.1/quazip
    QuaZip-Qt5-1.3/quazip
    QuaZip-Qt5-1.4/quazip
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

add_library(libmvp
    flash_widget.ui
    resources.qrc
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

target_compile_features(libmvp PRIVATE cxx_std_17)
set_target_properties(libmvp PROPERTIES OUTPUT_NAME "mvp")
target_link_libraries(libmvp
    PUBLIC ${QUAZIP_QT5}
    PUBLIC Qt5::Concurrent
    PUBLIC Qt5::Gui
    PUBLIC Qt5::Network
    PUBLIC Qt5::SerialPort
    PUBLIC Qt5::Widgets
)

include(GenerateExportHeader)
generate_export_header(libmvp)

install(TARGETS libmvp DESTINATION lib)

# vim:tw=0
