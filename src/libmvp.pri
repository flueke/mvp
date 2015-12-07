QT += serialport concurrent
CONFIG += c++11

HEADERS  += \
  $$PWD/flash.h \
  $$PWD/util.h \
  $$PWD/port_helper.h \
  $$PWD/file_dialog.h \

SOURCES += \
  $$PWD/flash.cc \
  $$PWD/util.cc \
  $$PWD/port_helper.cc \
  $$PWD/file_dialog.cc \

INCLUDEPATH += $$PWD

include($$PWD/../external/external.pri)
