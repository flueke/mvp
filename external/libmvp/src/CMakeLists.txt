cmake_policy(SET CMP0020 NEW) # Automatically link Qt executables to qtmain target on Windows.
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(QuaZip REQUIRED)
find_package(Boost 1.56 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${QUAZIP_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

set(LIBMVP_SOURCES
    file_dialog.cc
    firmware.cc
    firmware_ops.cc
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

target_link_libraries(${LIBMVP_TARGET} ${QUAZIP_LIBRARIES})

include(GenerateExportHeader)
generate_export_header(${LIBMVP_TARGET})

install(TARGETS ${LIBMVP_TARGET} DESTINATION lib COMPONENT libraries)

# vim:tw=0
