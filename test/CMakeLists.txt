set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5Widgets REQUIRED)

find_library(QUAZIP_QT5 quazip-qt5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

include_directories(
  ${libmvp_SOURCE_DIR}/src
  ${Qt5Concurrent_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5SerialPort_INCLUDE_DIRS}
  ${Qt5Test_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  )

set(LIBMVP_TEST_SOURCES
  test_file_dialog.cc
  test_flash.cc
  testmain.cc
  test_mdpp16_firmware.cc
  test_util.cc
  )

qt5_wrap_cpp(MOC_OUT tests.h)

set(LIBMVP_TEST_TARGET "testrunner")

link_directories(${libmvp_BINARY_DIR}/src)

add_executable(${LIBMVP_TEST_TARGET} EXCLUDE_FROM_ALL ${LIBMVP_TEST_SOURCES} ${MOC_OUT})

target_link_libraries(
  ${LIBMVP_TEST_TARGET}
  mvp
  Qt5::Concurrent
  Qt5::Gui
  Qt5::SerialPort
  Qt5::Widgets
  Qt5::Test
  ${QUAZIP_QT5}
  )

# vim:tw=0
