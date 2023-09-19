set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Concurrent Gui Network SerialPort Widgets)

# Windows resource file
if (WIN32)
    set(RES_FILES "windows_metafile.rc")
    set(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif()

# Git Version file generation
configure_file("git_version.cc.in" "git_version.cc" @ONLY)

add_executable(mvp WIN32
  "${CMAKE_CURRENT_BINARY_DIR}/git_version.cc"
  ${RES_FILES}
  main.cc
)

target_link_libraries(mvp PRIVATE libmvp)
target_compile_options(mvp
    PRIVATE $<${not-msvc}:-Wall -Wextra>
    PRIVATE $<${not-msvc}:-Wno-deprecated-declarations> # disable Qt deprecation warnings
)

install(TARGETS mvp RUNTIME DESTINATION bin)

# vim: tw=0
