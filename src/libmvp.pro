TEMPLATE = lib
TARGET = mvp
win32:LIBS += -lquazip
unix:LIBS += -lquazip-qt5
include($$PWD/libmvp.pri)
