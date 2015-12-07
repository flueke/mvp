TEMPLATE = subdirs
SUBDIRS  = src test external
include(external/external.pri)
src.file = $$PWD/src/libmvp.pro
src.depends = external
test.depends = src
