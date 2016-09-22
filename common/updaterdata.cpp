/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updaterdata.h"

#include <QObject>

QString Updater::build     = QObject::tr("%1 %2").arg(__DATE__, __TIME__);
QString Updater::copyright = QObject::tr("Copyright (c) 2004-2016 OpenMFG, LLC., d/b/a xTuple.");
bool    Updater::loggedIn  = false;
QString Updater::name      = QObject::tr("Update Manager");
QString Updater::version   = QObject::tr("2.5.0Beta");

