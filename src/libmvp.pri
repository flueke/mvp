QT += serialport concurrent gui widgets
CONFIG += c++11

HEADERS  += \
  $$PWD/flash.h \
  $$PWD/util.h \
  $$PWD/port_helper.h \
  $$PWD/file_dialog.h \
  $$PWD/mdpp16_firmware.h \

SOURCES += \
  $$PWD/flash.cc \
  $$PWD/util.cc \
  $$PWD/port_helper.cc \
  $$PWD/file_dialog.cc \
  $$PWD/mdpp16_firmware.cc \

INCLUDEPATH += $$PWD

include($$PWD/../external/external.pri)
