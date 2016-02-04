set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5Widgets REQUIRED)

find_package(QuaZip REQUIRED)
#if (WIN32)
#find_library(QUAZIP_QT5 quazip)
#else()
#find_library(QUAZIP_QT5 quazip-qt5)
#endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

include_directories(
  ${libmvp_SOURCE_DIR}/src
  )

set(LIBMVP_TEST_SOURCES
  test_file_dialog.cc
  test_firmware.cc
  test_flash.cc
  test_instruction_file.cc
  test_instruction_interpreter.cc
  testmain.cc
  test_util.cc
  )

qt5_wrap_cpp(MOC_OUT tests.h)

set(LIBMVP_TEST_TARGET "testrunner")

link_directories(${libmvp_BINARY_DIR}/src)

add_executable(${LIBMVP_TEST_TARGET} EXCLUDE_FROM_ALL ${LIBMVP_TEST_SOURCES} ${MOC_OUT})

qt5_use_modules(${LIBMVP_TEST_TARGET}
  Concurrent
  Gui
  SerialPort
  Test
  Widgets
  )

target_link_libraries(${LIBMVP_TEST_TARGET}
  mvp
  ${QUAZIP_QT5}
  )

# vim:tw=0
