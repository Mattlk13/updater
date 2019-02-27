/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2019 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __LOADQM_H__
#define __LOADQM_H__

#include "loadable.h"

class LoadQm : public Loadable
{
  public:
    LoadQm(const QString &name, const int grade = 0,
           const bool system = false, const QString &comment = QString::null,
           const QString &filename = QString::null);
    LoadQm(const QDomElement &pElem, const bool system,
           QStringList &, QList<bool> &);

    virtual int writeToDB(QByteArray &pData, const QString pPkgname, QString &errMsg);
};

#endif
