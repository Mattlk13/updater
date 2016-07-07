#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#

include( ../global.pri )

TEMPLATE = lib
CONFIG += qt warn_on thread staticlib
QT += xml sql xmlpatterns
isEqual(QT_MAJOR_VERSION, 5) {
  QT += widgets
}


TARGET = updatercommon
DESTDIR = ../lib
OBJECTS_DIR = tmp
MOC_DIR = tmp
UI_SOURCES_DIR = tmp

HEADERS = updaterdata.h                 \
          package.h \
          createdbobj.h \
          createfunction.h \
          createtable.h \
          createtrigger.h \
          createview.h \
          finalscript.h \
          initscript.h \
          script.h \
          loadable.h \
          loadappscript.h \
          loadappui.h \
          loadcmd.h \
          loadimage.h \
          loadmetasql.h \
          loadpriv.h \
          loadreport.h \
          pkgschema.h \
          prerequisite.h \
          xabstractmessagehandler.h    \
          cmdlinemessagehandler.h      \
          guimessagehandler.h          \
          xversion.h

SOURCES = updaterdata.cpp              \
          package.cpp \
          createdbobj.cpp \
          createfunction.cpp \
          createtable.cpp \
          createtrigger.cpp \
          createview.cpp \
          finalscript.cpp \
          initscript.cpp \
          script.cpp \
          loadable.cpp \
          loadappscript.cpp \
          loadappui.cpp \
          loadcmd.cpp \
          loadimage.cpp \
          loadmetasql.cpp \
          loadpriv.cpp \
          loadreport.cpp \
          pkgschema.cpp \
          prerequisite.cpp \
          xabstractmessagehandler.cpp  \
          cmdlinemessagehandler.cpp    \
          guimessagehandler.cpp        \
          xversion.cpp
