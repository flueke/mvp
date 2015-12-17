QT += core gui widgets
CONFIG += debug_and_release c++11
TEMPLATE = subdirs
SUBDIRS  = src test
include(external/external.pri)
src.file      = $$PWD/src/libmvp.pro
test.depends  = src
