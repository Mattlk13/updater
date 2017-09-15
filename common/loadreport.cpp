/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "loadreport.h"

#include <QDomDocument>

#include "metasql.h"
#include "xsqlquery.h"

LoadReport::LoadReport(const QString &name, const int grade, const bool system,
                       const QString &comment, const QString &filename)
  : Loadable("loadreport", name, grade, system, comment, filename)
{
  _pkgitemtype = "R";
}

LoadReport::LoadReport(const QDomElement & elem, const bool system,
                       QStringList &msg, QList<bool> &fatal)
  : Loadable(elem, system, msg, fatal)
{
  _pkgitemtype = "R";

  if (elem.nodeName() != "loadreport")
  {
    msg.append(TR("Creating a LoadAppReport element from a %1 node.")
                       .arg(elem.nodeName()));
    fatal.append(false);
  }
}

int LoadReport::writeToDB(QByteArray &pData, const QString pPkgname, QString &errMsg)
{
  int errLine = 0;
  int errCol  = 0;
  QDomDocument doc;
  cleanData(pData);
  if (! doc.setContent(pData, &errMsg, &errLine, &errCol))
  {
    errMsg = (TR("<font color=red>Error parsing file %1: %2 on "
                          "line %3 column %4</font>")
                          .arg(_filename).arg(errMsg).arg(errLine).arg(errCol));
    return -1;
  }

  QDomElement root = doc.documentElement();
  if(root.tagName() != "report")
  {
    errMsg = TR("<font color=red>XML Document %1 does not have root"
                         " node of report</font>")
                         .arg(_filename);
    return -2;
  }

  for(QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling())
  {
    if(n.nodeName() == "name")
      _name = n.firstChild().nodeValue();
    else if(n.nodeName() == "description")
      _comment = n.firstChild().nodeValue();
  }
  QString report_src = doc.toString();

  if(_filename.isEmpty())
  {
    errMsg = TR("<font color=orange>The document %1 does not have"
                         " a report name defined</font>")
                         .arg(_filename);
    return -3;
  }

  _minMql = new MetaSQLQuery("SELECT MIN(report_grade) AS min "
                   "FROM report "
                   "WHERE (report_name=<? value('name') ?>);");

  _maxMql = new MetaSQLQuery("SELECT MAX(report_grade) AS max "
                   "FROM report "
                   "WHERE (report_name=<? value('name') ?>);");

  _gradeMql = new MetaSQLQuery("SELECT MIN(sequence_value-1) "
                     "            FROM sequence "
                     "           WHERE sequence_value-1>=<? value('grade') ?> "
                     "             AND sequence_value-1 NOT IN (SELECT report_grade "
                     "                                            FROM report "
                     "                                            JOIN pg_class c ON report.tableoid=c.oid "
                     "                                            JOIN pg_namespace n ON c.relnamespace=n.oid "
                     "                                           WHERE report_name=<? value('name') ?> "
                     "                                             AND n.nspname!=<? value('pkgname') ?>);");

  _selectMql = new MetaSQLQuery("SELECT report_id, -1, -1"
                      "  FROM ONLY <? literal('tablename') ?> "
                      " WHERE ((report_name=<? value('name') ?>) "
                      "    AND (report_grade=<? value('grade') ?>) );");

  _updateMql = new MetaSQLQuery("UPDATE <? literal('tablename') ?> "
                      "   SET report_descrip=<? value('notes') ?>, "
                      "       report_source=<? value('source') ?> "
                      " WHERE (report_id=<? value('id') ?>) "
                      "RETURNING report_id AS id;");

  _insertMql = new MetaSQLQuery("INSERT INTO <? literal('tablename') ?> ("
                      "    report_id, report_name,"
                      "    report_grade, report_source, report_descrip"
                      ") VALUES ("
                      "    DEFAULT, <? value('name') ?>,"
                      "    <? value('grade') ?>, <? value('source') ?>,"
                      "    <? value('notes') ?>) "
                      "RETURNING report_id AS id;");

  ParameterList params;
  params.append("tablename", "report");

  return Loadable::writeToDB(pData, pPkgname, errMsg, params);
}
