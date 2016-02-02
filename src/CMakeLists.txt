set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5Widgets REQUIRED)

find_package(Boost 1.56 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

set(LIBMVP_SOURCES
    file_dialog.cc
    flash.cc
    flash_widget.cc
    firmware.cc
    firmware_ops.cc
    instruction_file.cc
    instruction_interpreter.cc
    port_helper.cc
    util.cc
    )

qt5_wrap_ui(UIC_OUT flash_widget.ui)
qt5_add_resources(RC_OUT resources.qrc)

set(LIBMVP_SHARED_TARGET "libmvp_shared")
set(LIBMVP_STATIC_TARGET "libmvp_static")

add_library(${LIBMVP_SHARED_TARGET} SHARED ${LIBMVP_SOURCES} ${UIC_OUT} ${RC_OUT})
add_library(${LIBMVP_STATIC_TARGET} STATIC ${LIBMVP_SOURCES} ${UIC_OUT} ${RC_OUT})

set_target_properties(${LIBMVP_SHARED_TARGET} PROPERTIES OUTPUT_NAME "mvp")
set_target_properties(${LIBMVP_STATIC_TARGET} PROPERTIES OUTPUT_NAME "mvp")

qt5_use_modules(${LIBMVP_SHARED_TARGET}
  Concurrent
  Gui
  SerialPort
  Widgets
  )

qt5_use_modules(${LIBMVP_STATIC_TARGET}
  Concurrent
  Gui
  SerialPort
  Widgets
  )

# vim:tw=0
