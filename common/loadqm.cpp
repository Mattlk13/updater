/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "loadqm.h"

#include <QDomElement>
#include <QSqlError>
#include <QString>
#include <QVariant>

#include "metasql.h"
#include "xsqlquery.h"

LoadQm::LoadQm(const QString &name, const int grade, const bool system, const QString &comment, const QString &filename)
       : Loadable("loadqm", name, grade, system, comment, filename)
{
  _pkgitemtype = "Q";
}

LoadQm::LoadQm(const QDomElement & elem, const bool system, QStringList &msg, QList<bool> &fatal)
       : Loadable(elem, system, msg, fatal)
{
  _pkgitemtype = "Q";

  if (elem.nodeName() != "loadqm")
  {
    msg.append(TR("Creating a LoadQm element from a %1 node.")
                 .arg(elem.nodeName()));
    fatal.append(false);
  }
}

int LoadQm::writeToDB(QByteArray &pData, const QString pPkgname, QString &errMsg)
{
  QString country = "";
  QString lang = "";
  int langid = -1;
  int countryid = -1;

  QStringList directory_chop = _filename.split("/");
  QString formatted_filename = directory_chop[directory_chop.size() - 1];

  QStringList file_parts = formatted_filename.split(".");
  QStringList locale_parts = file_parts[1].split("_");

  lang = locale_parts[0];
  if (locale_parts.size() > 1)
  {
    country = locale_parts[1];
  }

  XSqlQuery getids;
  getids.prepare("SELECT lang_id, country_id "
                 "  FROM lang, country "
                 " WHERE lang_abbr2=:lang "
                 "   AND (country_abbr=:country OR :country IS NULL);");
  getids.bindValue(":lang", lang);
  if (!country.isEmpty())
    getids.bindValue(":country", country.toUpper());
  getids.exec();
  if (getids.first())
  {
    langid = getids.value("lang_id").toInt();
    if (!country.isEmpty())
      countryid = getids.value("country_id").toInt();
  }
  else if (getids.lastError().type() != QSqlError::NoError)
  {
    QSqlError err = getids.lastError();
    errMsg = err.databaseText();
    return -1;
  }

  QString version;
  XSqlQuery getver;
  if (pPkgname.isEmpty())
    getver.prepare("SELECT fetchmetrictext('ServerVersion') AS version;");
  else
  {
    getver.prepare("SELECT pkghead_version AS version "
                   "  FROM pkghead "
                   " WHERE pkghead_name=:name;");
    getver.bindValue(":name", pPkgname);
  }
  getver.exec();

  if (getver.first())
    version = getver.value("version").toString();
  else if (getver.lastError().type() != QSqlError::NoError)
  {
    QSqlError err = getver.lastError();
    errMsg = err.databaseText();
    return -1;
  }

  _selectMql = new MetaSQLQuery("SELECT dict_id "
                                "  FROM ONLY <? literal('tablename') ?> "
                                " WHERE dict_lang_id=<? value('lang') ?> "
                                "   AND COALESCE(dict_country_id, -1)=COALESCE(<? value('country') ?>, -1) "
                                "   AND dict_version=<? value('version') ?>;");

  _updateMql = new MetaSQLQuery("UPDATE <? literal('tablename') ?> "
                                "   SET dict_data=<? value('data') ?> "
                                " WHERE dict_id=<? value('id') ?> "
                                "RETURNING dict_id AS id;");

  _insertMql = new MetaSQLQuery("INSERT INTO <? literal('tablename') ?> "
                                "(dict_lang_id, dict_country_id, "
                                " dict_data, dict_version) "
                                "VALUES (<? value('lang') ?>, <? value('country') ?>, "
                                " <? value('data') ?>, <? value('version') ?>) "
                                "RETURNING dict_id AS id;");

  ParameterList params;
  params.append("tablename", "dict");
  params.append("lang", langid);
  if (!country.isEmpty())
    params.append("country", countryid);
  params.append("data", QVariant(pData));
  params.append("version", version);

  return Loadable::writeToDB(pData, pPkgname, errMsg, params);
}
