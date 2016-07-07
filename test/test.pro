TEMPLATE = app
TARGET = testxversion

INCLUDEPATH += ../common
DEPENDPATH  += $${INCLUDEPATH}

QMAKE_LIBDIR = ../lib
LIBS        += -lupdatercommon

# Input
SOURCES += testxversion.cpp
