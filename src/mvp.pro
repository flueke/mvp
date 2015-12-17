TEMPLATE = app
TARGET = ../mvp
win32:LIBS += -lquazip
unix:LIBS += -lquazip-qt5

include($$PWD/../external/external.pri)

QT += core gui widgets serialport concurrent
CONFIG += debug_and_release c++11

HEADERS += \
    gui.h

SOURCES += \
  main.cc \
    gui.cc

FORMS += \
    gui.ui

RESOURCES += $$PWD/resources.qrc
