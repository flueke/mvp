set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(QuaZip REQUIRED)
find_package(Boost 1.56 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

include_directories(${Boost_INCLUDE_DIRS} ${libmvp_SOURCE_DIR}/src ${libmvp_BINARY_DIR}/src)

#message("libmvp_SOURCE_DIR: ${libmvp_SOURCE_DIR}")
#message("libmvp_BINARY_DIR: ${libmvp_BINARY_DIR}")

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

add_executable(${LIBMVP_TEST_TARGET} EXCLUDE_FROM_ALL ${LIBMVP_TEST_SOURCES} ${MOC_OUT})

qt5_use_modules(${LIBMVP_TEST_TARGET}
  Concurrent
  Gui
  SerialPort
  Test
  Widgets
  )

# Use find_library() here to get the full path to libmvp. This way CMake will
# relink the binary if the library has changed.
link_directories(${libmvp_BINARY_DIR}/src)

#get_property(LIBMVP_LIBRARY_PATH TARGET "libmvp" PROPERTY LIBRARY_PATH)
#get_property(LIBMVP_LIBRARY_NAME TARGET "libmvp" PROPERTY LIBRARY_OUTPUT_NAME)

target_link_libraries(${LIBMVP_TEST_TARGET}
  libmvp
  ${QUAZIP_LIBRARIES}
  )

# vim:tw=0
