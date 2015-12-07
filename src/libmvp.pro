TEMPLATE = lib

QT += core gui widgets serialport concurrent

CONFIG += debug_and_release c++11

HEADERS  += \
  $$PWD/flash.h \
  $$PWD/util.h \
  $$PWD/port_helper.h \

SOURCES += \
  $$PWD/flash.cc \
  $$PWD/util.cc \
  $$PWD/port_helper.cc \

INCLUDEPATH += $$PWD

