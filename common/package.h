/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2019 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <QString>
#include <QList>

#include "xversion.h"

class QDomDocument;
class QDomElement;

class Loadable;
class Prerequisite;
class Script;
class XAbstractMessageHandler;

class Package
{
  public:
    Package(const QString & id = QString::null);
    Package(const QDomElement &, QStringList &, QList<bool> &, XAbstractMessageHandler *);

    virtual ~Package();

    QDomElement createElement(QDomDocument &); 
    int writeToDB(QString &errMsg);

    QString id() const { return _id; }
    void setId(const QString & id) { _id = id; }

    QString developer() const { return _developer; }
    QString name()      const { return _name; }
    bool     system()   const;
    XVersion version()  const { return _pkgversion; }

    QList<Script*>       _functions;
    QList<Script*>       _tables;
    QList<Script*>       _triggers;
    QList<Script*>       _views;
    QList<Loadable*>     _appscripts;
    QList<Loadable*>     _appuis;
    QList<Loadable*>     _cmds;
    QList<Loadable*>     _images;
    QList<Loadable*>     _metasqls;
    QList<Loadable*>     _privs;
    QList<Prerequisite*> _prerequisites;
    QList<Loadable*>     _qms;
    QList<Script*>       _scripts;
    QList<Script*>       _finalscripts;
    QList<Script*>       _initscripts;
    QList<Loadable*>     _reports;

    bool containsAppScript(const QString &name)    const;
    bool containsAppUI(const QString &name)        const;
    bool containsCmd(const QString &name)          const;
    bool containsFunction(const QString &name)     const;
    bool containsImage(const QString &name)        const;
    bool containsPrerequisite(const QString &name) const;
    bool containsMetasql(const QString &name)      const;
    bool containsPriv(const QString &name)         const;
    bool containsQm(const QString &name)           const;
    bool containsReport(const QString &name)       const;
    bool containsScript(const QString &name)       const;
    bool containsFinalScript(const QString &name)  const;
    bool containsInitScript(const QString &name)  const;
    bool containsTable(const QString &name)        const;
    bool containsTrigger(const QString &name)      const;
    bool containsView(const QString &name)         const;

  protected:
    QString     _developer;
    QString     _descrip;
    QString     _id;
    XVersion    _pkgversion;
    QString     _name;
    QString     _notes;
};

#endif
