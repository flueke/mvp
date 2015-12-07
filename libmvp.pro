QT += core gui widgets
CONFIG += debug_and_release c++11
TEMPLATE = subdirs
SUBDIRS  = src test external
include(external/external.pri)
src.file      = $$PWD/src/libmvp.pro
src.depends   = external
test.depends  = src
