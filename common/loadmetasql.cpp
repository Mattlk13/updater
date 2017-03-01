/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "loadmetasql.h"

#include <QDomDocument>
#include <QSqlError>
#include <QVariant>     // used by XSqlQuery::bindValue()

#include "metasql.h"
#include "xsqlquery.h"

#define DEBUG false

LoadMetasql::LoadMetasql(const QString &name, const QString &group,
                         const bool system,
                         const QString &comment, const QString &filename)
  : Loadable("loadmetasql", name, 0, system, comment, filename)
{
  _pkgitemtype = "M";
  _group = group;
}

LoadMetasql::LoadMetasql(const QDomElement &elem, const bool system,
                         QStringList &msg, QList<bool> &fatal)
  : Loadable(elem, system, msg, fatal)
{
  if (DEBUG)
    qDebug("LoadMetasql::LoadMetasql(QDomElement) entered");

  _pkgitemtype = "M";

  if (elem.nodeName() != "loadmetasql")
  {
    msg.append(TR("Creating a LoadMetasql element from a %1 node.")
              .arg(elem.nodeName()));
    fatal.append(false);
  }

  if (elem.hasAttribute("group"))
    _group = elem.attribute("group");

  if (elem.hasAttribute("enabled"))
  {
    msg.append(TR("Node %1 '%2' has an 'enabled' "
                           "attribute which is ignored for MetaSQL statements.")
                       .arg(elem.nodeName()).arg(elem.attribute("name")));
    fatal.append(false);
  }

}

int LoadMetasql::writeToDB(const QByteArray &pdata, const QString pkgname, QString &errMsg)
{
  if (pdata.isEmpty())
  {
    errMsg = TR("<font color=orange>The MetaSQL statement %1 is empty.</font>")
                         .arg(_name);
    return -2;
  }

  const char *fileContent = pdata.data();
  QString metasqlStr = QString::fromLocal8Bit(fileContent);
  QStringList lines  = metasqlStr.split("\n");
  QRegExp groupRE    = QRegExp("(^\\s*--\\s*GROUP:\\s*)(.*)",Qt::CaseInsensitive);
  QRegExp nameRE     = QRegExp("(^\\s*--\\s*NAME:\\s*)(.*)", Qt::CaseInsensitive);
  QRegExp notesRE    = QRegExp("(^\\s*--\\s*NOTES:\\s*)(.*)",Qt::CaseInsensitive);
  QRegExp dashdashRE = QRegExp("(^\\s*--\\s*)(.*)");

  for (int i = 0; i < lines.size(); i++)
  {
    if (DEBUG)
      qDebug("LoadMetasql::writeToDB looking at %s", qPrintable(lines.at(i)));

    if (groupRE.indexIn(lines.at(i)) >= 0)
    {
      _group = groupRE.cap(2).trimmed();
      if (DEBUG)
        qDebug("LoadMetasql::writeToDB() found group %s", qPrintable(_group));
    }
    else if (nameRE.indexIn(lines.at(i)) >= 0)
    {
      _name = nameRE.cap(2).trimmed();
      if (DEBUG)
        qDebug("LoadMetasql::writeToDB() found name %s", qPrintable(_name));
    }
    else if (notesRE.indexIn(lines.at(i)) >= 0)
    {
      _comment = notesRE.cap(2).trimmed();
      while (dashdashRE.indexIn(lines.at(++i)) >= 0)
        _comment += " " + dashdashRE.cap(2).trimmed();
      if (DEBUG)
        qDebug("LoadMetasql::writeToDB() found notes %s", qPrintable(_comment));
    }
  }

  if (DEBUG)
    qDebug("LoadMetasql::writeToDB(): name %s group %s notes %s\n%s",
           qPrintable(_name), qPrintable(_group), qPrintable(_comment),
           qPrintable(metasqlStr));

  _minMql = new MetaSQLQuery("SELECT MIN(metasql_grade) AS min "
                   "FROM metasql "
                   "WHERE (metasql_group=<? value('group') ?>) "
                   "AND (metasql_name=<? value('name') ?>);");

  _maxMql = new MetaSQLQuery("SELECT MAX(metasql_grade) AS max "
                   "FROM metasql "
                   "WHERE (metasql_group=<? value('group') ?>) "
                   "AND (metasql_name=<? value('name') ?>);");

  _gradeMql = new MetaSQLQuery("SELECT MIN(sequence_value-1) "
                     "            FROM sequence "
                     "           WHERE sequence_value-1>=<? value('grade') ?> "
                     "             AND sequence_value-1 NOT IN (SELECT metasql_grade "
                     "                                            FROM metasql "
                     "                                            JOIN pg_class c ON metasql.tableoid=c.oid "
                     "                                            JOIN pg_namespace n ON c.relnamespace=n.oid "
                     "                                           WHERE metasql_group=<? value('group') ?> "
                     "                                             AND metasql_name=<? value('name') ?> "
                     "                                             AND n.nspname!=<? value('pkgname') ?>);");

  _selectMql = new MetaSQLQuery("SELECT metasql_id, -1, -1"
                      "  FROM ONLY <? literal('tablename') ?> "
                      " WHERE ((metasql_group=<? value('group') ?>) "
                      "    AND (metasql_name=<? value('name') ?>) "
                      "    AND (metasql_grade=<? value('grade') ?>) );");

  _updateMql = new MetaSQLQuery("UPDATE <? literal('tablename') ?> "
                      "   SET metasql_notes=<? value('notes') ?>, "
                      "       metasql_query=<? value('source') ?> "
                      " WHERE (metasql_id=<? value('id') ?>) "
                      "RETURNING metasql_id AS id;");

  _insertMql = new MetaSQLQuery("INSERT INTO <? literal('tablename') ?> ("
                      "    metasql_group, metasql_name,"
                      "    metasql_grade, metasql_query, metasql_notes"
                      ") VALUES ("
                      "    <? value('group') ?>, <? value('name') ?>,"
                      "    <? value('grade') ?>, <? value('source') ?>,"
                      "    <? value('notes') ?>) "
                      "RETURNING metasql_id AS id;");

  ParameterList params;
  params.append("tablename", "metasql");
  params.append("group", _group);

  return Loadable::writeToDB(pdata, pkgname, errMsg, params);
}
