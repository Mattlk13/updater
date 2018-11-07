#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#

OPENRPT_HEADERS = $$(OPENRPT_HEADERS)
isEmpty( OPENRPT_HEADERS ) {
  exists(../qt-client/openrpt) { OPENRPT_HEADERS = ../qt-client/openrpt }
  exists(../openrpt)           { OPENRPT_HEADERS = ../openrpt }
  OPENRPT_HEADERS_REL = true
}

! exists($${OPENRPT_HEADERS}) {
  error("Could not set the OPENRPT_HEADERS qmake variable.")
}

OPENRPT_LIBDIR = $$(OPENRPT_LIBDIR)
isEmpty( OPENRPT_LIBDIR ) {
  exists(../qt-client/openrpt)     { OPENRPT_LIBDIR = ../qt-client/openrpt/lib }
  exists(../openrpt)               { OPENRPT_LIBDIR = ../openrpt/lib }
  exists(../openrpt-build-desktop) { OPENRPT_LIBDIR = ../openrpt-build-desktop/lib }
  OPENRPT_LIBDIR_REL = true
}

! exists($${OPENRPT_LIBDIR}) {
  error("Could not set the OPENRPT_LIBDIR qmake variable.")
}

! isEmpty( OPENRPT_HEADERS_REL ) { OPENRPT_HEADERS = ../$${OPENRPT_HEADERS} }
! isEmpty( OPENRPT_LIBDIR_REL  ) { OPENRPT_LIBDIR  = ../$${OPENRPT_LIBDIR}  }
message("Looking for OpenRPT in $${OPENRPT_HEADERS} and $${OPENRPT_LIBDIR}.")

XTUPLE_HEADERS = $$(XTUPLE_HEADERS)
isEmpty( XTUPLE_HEADERS ) {
  exists(../qt-client) { XTUPLE_HEADERS = ../qt-client }
  XTUPLE_HEADERS_REL = true
}

XTUPLE_LIBDIR = $$(XTUPLE_LIBDIR)
isEmpty( XTUPLE_LIBDIR ) {
  exists(../qt-client)               { XTUPLE_LIBDIR = ../qt-client/lib }
  exists(../qt-client-build-desktop) { XTUPLE_LIBDIR = $${XTUPLE_DIR}-build-desktop/lib }
  XTUPLE_LIBDIR_REL = true
}

! isEmpty( XTUPLE_HEADERS_REL ) { XTUPLE_HEADERS = ../$${XTUPLE_HEADERS} }
! isEmpty( XTUPLE_LIBDIR_REL  ) { XTUPLE_LIBDIR  = ../$${XTUPLE_LIBDIR}  }
message("Looking for xTuple in $${XTUPLE_HEADERS} and $${XTUPLE_LIBDIR}.")

UPDATER_LIBDIR=../lib
exists(../updater-desktop-build) {
  UPDATER_LIBDIR=../updater-desktop-build/lib
}

INCLUDEPATH += ../common \
               $${OPENRPT_HEADERS}/common \
               $${OPENRPT_HEADERS}/MetaSQL \
               $${XTUPLE_HEADERS}/common
INCLUDEPATH = $$unique(INCLUDEPATH)
DEPENDPATH  += $${INCLUDEPATH}

CONFIG += release

win32*:OPENRPTLIBEXT       = a
win32*:XTLIBEXT            = a
win32-msvc*:OPENRPTLIBEXT  = lib
win32-msvc*:XTLIBEXT       = lib
unix:OPENRPTLIBEXT         = so
unix:XTLIBEXT              = so
macx:OPENRPTLIBEXT         = a
macx:XTLIBEXT              = a
macx:CONFIG               += c++11

macx:exists(macx.pri) {
  include(macx.pri)
}

win32:exists(win32.pri) {
  include(win32.pri)
}

unix:exists(unix.pri) {
  include(unix.pri)
}
