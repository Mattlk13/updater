#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#

include( ../global.pri )

TEMPLATE = app
CONFIG += qt warn_on
DEPENDPATH  += ../$${XTUPLE_BLD}/common

TARGET = updater
OBJECTS_DIR = tmp
MOC_DIR     = tmp
UI_DIR      = tmp

QMAKE_LIBDIR += $${UPDATER_LIBDIR} $${OPENRPT_LIBDIR} $${XTUPLE_LIBDIR}
LIBS += -lxtuplecommon -lupdatercommon -lopenrptcommon -lrenderer -lMetaSQL
macx: LIBS += -lz

win32-msvc* {
  PRE_TARGETDEPS += $${UPDATER_LIBDIR}/updatercommon.lib          \
                    $${XTUPLE_LIBDIR}/xtuplecommon.$${XTLIBEXT} \
                    $${OPENRPT_LIBDIR}/MetaSQL.$${OPENRPTLIBEXT} \
                    $${OPENRPT_LIBDIR}/openrptcommon.$${OPENRPTLIBEXT} \
                    $${OPENRPT_LIBDIR}/renderer.$${OPENRPTLIBEXT}
} else {
  PRE_TARGETDEPS += $${UPDATER_LIBDIR}/libupdatercommon.a          \
                    $${XTUPLE_LIBDIR}/libxtuplecommon.$${XTLIBEXT} \
                    $${OPENRPT_LIBDIR}/libMetaSQL.$${OPENRPTLIBEXT} \
                    $${OPENRPT_LIBDIR}/libopenrptcommon.$${OPENRPTLIBEXT} \
                    $${OPENRPT_LIBDIR}/librenderer.$${OPENRPTLIBEXT}
}

DESTDIR = ../bin

RC_FILE = rcloader.rc
macx {
  RC_FILE = ../images/updater.icns
  QMAKE_INFO_PLIST = Info.plist
}

FORMS   += loaderwindow.ui

HEADERS += loaderwindow.h               \
           cmdlinemessagehandler.h      \
           guimessagehandler.h

SOURCES += loaderwindow.cpp             \
           main.cpp                     \
           cmdlinemessagehandler.cpp    \
           guimessagehandler.cpp

QT += xml sql xmlpatterns

RESOURCES += loader.qrc
