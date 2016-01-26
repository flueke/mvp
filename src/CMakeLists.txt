set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

#message("Qt stuff: ${Qt5Widgets_VERSION}")
#message("Qt stuff: ${Qt5Widgets_LIBRARIES}")
#message("Qt stuff: ${Qt5Widgets_INCLUDE_DIRS}")
#message("Qt stuff: ${Qt5Widgets_DEFINITIONS}")
#message("Qt stuff: ${Qt5Widgets_COMPILE_DEFINITIONS}")
#message("Qt stuff: ${Qt5Widgets_FOUND}")
#message("Qt stuff: ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

include_directories(
    ${Qt5Concurrent_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5SerialPort_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    )

set(LIBMVP_SOURCES
    file_dialog.cc
    flash.cc
    mdpp16_firmware.cc
    port_helper.cc
    util.cc
    )

set(LIBMVP_SHARED_TARGET "mvp_shared")
set(LIBMVP_STATIC_TARGET "mvp_static")

add_library(${LIBMVP_SHARED_TARGET} SHARED ${LIBMVP_SOURCES})
add_library(${LIBMVP_STATIC_TARGET} STATIC ${LIBMVP_SOURCES})

set_target_properties(${LIBMVP_SHARED_TARGET} PROPERTIES OUTPUT_NAME "mvp")
set_target_properties(${LIBMVP_STATIC_TARGET} PROPERTIES OUTPUT_NAME "mvp")

target_link_libraries(
    ${LIBMVP_SHARED_TARGET}
    Qt5::Concurrent
    Qt5::Gui
    Qt5::SerialPort
    Qt5::Widgets
    )

target_link_libraries(
    ${LIBMVP_STATIC_TARGET}
    Qt5::Concurrent
    Qt5::Gui
    Qt5::SerialPort
    Qt5::Widgets
    )

# vim:tw=0
