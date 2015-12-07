TEMPLATE = subdirs
SUBDIRS  = src test external
src.file = $$PWD/src/libmvp.pro
src.depends = external
test.depends = src
