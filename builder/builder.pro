#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2019 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#

include( ../global.pri )

TEMPLATE = app
CONFIG += qt warn_on thread
QT     += xml sql xmlpatterns
isEqual(QT_MAJOR_VERSION, 5) {
  QT += widgets
}

DESTDIR = ../bin

QMAKE_LIBDIR += $${UPDATER_LIBDIR} $${OPENRPT_LIBDIR} $${XTUPLE_LIBDIR}
LIBS += -lxtuplecommon -lupdatercommon -lopenrptcommon -lrenderer -lMetaSQL
win32-msvc* {
  PRE_TARGETDEPS += $${UPDATER_LIBDIR}/updatercommon.lib          \
                    $${OPENRPT_LIBDIR}/MetaSQL.$${OPENRPTLIBEXT}       \
                    $${OPENRPT_LIBDIR}/openrptcommon.$${OPENRPTLIBEXT} \
                    $${OPENRPT_LIBDIR}/renderer.$${OPENRPTLIBEXT}
} else {
  PRE_TARGETDEPS += $${UPDATER_LIBDIR}/libupdatercommon.a \
                    $${OPENRPT_LIBDIR}/libMetaSQL.$${OPENRPTLIBEXT}       \
                    $${OPENRPT_LIBDIR}/libopenrptcommon.$${OPENRPTLIBEXT} \
                    $${OPENRPT_LIBDIR}/librenderer.$${OPENRPTLIBEXT}
}

MOC_DIR = tmp
UI_DIR = tmp
OBJECTS_DIR = tmp

FORMS   += packagewindow.ui \
           texteditdialog.ui \
           providerdialog.ui \
           newprereqdialog.ui \
           queryeditor.ui
HEADERS += packagewindow.h \
           providerdialog.h \
           texteditdialog.h \
           queryeditor.h \
           newprereqdialog.h
SOURCES += main.cpp \
           packagewindow.cpp \
           providerdialog.cpp \
           texteditdialog.cpp \
           queryeditor.cpp \
           newprereqdialog.cpp

