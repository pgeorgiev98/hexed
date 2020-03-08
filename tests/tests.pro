QT = core testlib

TARGET = tests
TEMPLATE = app
CONFIG = c++17 qt warn_on depend_includepath testcase

SRCDIR = ../app
INCLUDEPATH += $$SRCDIR

HEADERS += $$SRCDIR/bufferededitor.h

SOURCES += $$SRCDIR/bufferededitor.cpp

SOURCES += tests.cpp
