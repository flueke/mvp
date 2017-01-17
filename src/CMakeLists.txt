set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

set(MVP_SOURCES
  gui.cc
  gui.ui
  main.cc
  )

qt5_wrap_ui(UIC_OUT gui.ui)

set(MVP_TARGET "mvp")

# Git Version file generation
configure_file("git_sha1.cc.in" "git_sha1.cc" @ONLY)
set(GIT_VERSION_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/git_sha1.cc")

# Windows resource file
if (WIN32)
    set(RES_FILES "windows_metafile.rc")
    set(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif()

add_executable(${MVP_TARGET} WIN32 ${MVP_SOURCES} ${UIC_OUT} ${GIT_VERSION_SOURCE} ${RES_FILES})
add_dependencies(mvp libmvp)

qt5_use_modules(${MVP_TARGET}
  Concurrent
  Gui
  SerialPort
  Widgets
  )

target_link_libraries(${MVP_TARGET} libmvp)

install(TARGETS ${MVP_TARGET} DESTINATION bin COMPONENT applications)

# vim: tw=0
