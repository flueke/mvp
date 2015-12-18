include(../src/libmvp.pri)

QT += testlib

CONFIG += testcase

TARGET = libmvptest
TEMPLATE = app
win32:LIBS += -lquazip
unix:LIBS += -lquazip-qt5

HEADERS  += \
  $$PWD/tests.h \

SOURCES += \
  $$PWD/test_util.cc \
  $$PWD/test_flash.cc \
  $$PWD/test_mdpp16_firmware.cc \
  $$PWD/testmain.cc \

# GUI
SOURCES += \
  $$PWD/test_file_dialog.cc \
