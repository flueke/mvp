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
  resources.qrc
  )

qt5_wrap_ui(UIC_OUT gui.ui)

set(MVP_TARGET "mvp")

add_executable(${MVP_TARGET} ${MVP_SOURCES} ${UIC_OUT})
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
