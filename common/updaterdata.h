/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __LOADER_DATA_H__
#define __LOADER_DATA_H__

#include <QString>

class Updater {
  public:
    static QString  build;
    static QString  copyright;
    static bool     loggedIn;
    static QString  name;
    static QString  version;
};

#endif

