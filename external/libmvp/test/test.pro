include(../src/libmvp.pri)

QT += testlib

CONFIG += testcase

TARGET = libmvptest
TEMPLATE = app

HEADERS  += \
  $$PWD/tests.h \

SOURCES += \
  $$PWD/test_util.cc \
  $$PWD/test_flash.cc \
  $$PWD/testmain.cc \
