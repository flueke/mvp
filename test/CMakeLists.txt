set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Test REQUIRED)

# Ugly and should be fixed.
include_directories(${Boost_INCLUDE_DIRS} ${libmvp_SOURCE_DIR}/src ${libmvp_BINARY_DIR}/src)

#message("libmvp_SOURCE_DIR: ${libmvp_SOURCE_DIR}")
#message("libmvp_BINARY_DIR: ${libmvp_BINARY_DIR}")

add_executable(libmvp_testrunner
  test_file_dialog.cc
  test_firmware.cc
  test_firmware_ops.cc
  test_flash.cc
  test_instruction_file.cc
  test_instruction_interpreter.cc
  testmain.cc
  test_util.cc
  tests.h # so MOC parses it, otherwise undefined references appear
)

target_compile_features(libmvp_testrunner PRIVATE cxx_std_17)
target_link_libraries(libmvp_testrunner PUBLIC libmvp Qt5::Test)

add_test(NAME libmvp-test COMMAND libmvp_testrunner)

# vim:tw=0
