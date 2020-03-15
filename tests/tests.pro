QT = core testlib

TARGET = tests
TEMPLATE = app
CONFIG = c++17 qt warn_on depend_includepath testcase no_testcase_installs

SRCDIR = ../app
INCLUDEPATH += $$SRCDIR

HEADERS += $$SRCDIR/bufferededitor.h \
           $$SRCDIR/finder.h

SOURCES += $$SRCDIR/bufferededitor.cpp \
           $$SRCDIR/finder.cpp

SOURCES += tests.cpp
