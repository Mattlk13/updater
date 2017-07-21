#include "loadqm.h"
#include <QDomElement>
#include <QSqlError>
#include <QVariant>
#include <limits.h>

#include <iostream>
#include <QString>



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

int LoadQm::writeToDB(const QByteArray &pdata, const QString pkgname, QString &errMsg)
{
	QString extension_name = "";
	QString country = "";
	QString lang = "";


	QStringList directory_chop = _filename.split("/");
	QString formatted_filename = directory_chop[directory_chop.size() - 1];

	QStringList file_parts = formatted_filename.split(".");
	QStringList locale_parts = file_parts[1].split("_");
	//file_parts[0] == "extension", file_parts[1] == "lang_country"
	//locale_parts[0] == "lang", locale_parts[1] == "country"

	extension_name = file_parts[0];
	lang = locale_parts[0];
	if (locale_parts.size() > 1) {
		country = locale_parts[1];
	}



        QString version;
        if (pkgname.isEmpty())
        {
          XSqlQuery getver("SELECT fetchmetrictext('ServerVersion') AS version;");
          if (getver.first())
            version = getver.value("version").toString();
          else if (getver.lastError().type() != QSqlError::NoError)
          {
            QSqlError err = getver.lastError();
            errMsg = err.databaseText();
            return -1;
          }
        }
        else
        {
          XSqlQuery getver;
          getver.prepare("SELECT pkghead_version "
                         "  FROM pkghead "
                         " WHERE pkghead_name=:name;");
          getver.bindValue(":name", pkgname);
          getver.exec();
          if (getver.first())
            version = getver.value("pkghead_version").toString();
          else if (getver.lastError().type() != QSqlError::NoError)
          {
            QSqlError err = getver.lastError();
            errMsg = err.databaseText();
            return -1;
          }
        }

	//give the 0 pointer a MetaSQLQuery value

        _selectMql = new MetaSQLQuery("SELECT qm_id "
                                      "  FROM qm "
                                      " WHERE qm_extension_name=<? value('extension_nm') ?> "
                                      "   AND qm_lang=<? value('lang') ?> "
                                      "   AND qm_country=<? value('country') ?>;");

        _updateMql = new MetaSQLQuery("UPDATE qm "
                                      "   SET qm_data=<? value('qm_data') ?>, "
                                      "       qm_version=<? value('version') ?> "
                                      "RETURNING qm_id AS id;");

	_insertMql = new MetaSQLQuery("INSERT INTO qm "
                                      "(qm_extension_name, qm_lang, "
                                      " qm_country, qm_data, "
                                      " qm_version) "
                                      "VALUES (<? value('extension_nm') ?>, <? value('lang') ?>, "
                                      "        <? value('country') ?>, <? value('qm_data') ?>, "
                                      "        <? value('version') ?>) "
                                      "RETURNING qm_id AS id;");

	//assign params
	ParameterList params;
	params.append("extension_nm", extension_name);
	params.append("lang", lang);
	params.append("country", country);
	params.append("qm_data", QVariant(pdata));
        params.append("version", version);


	//let Loadable::writeToDB handle the execution of the _insertMql query
	return Loadable::writeToDB(pdata, pkgname, errMsg, params);




}
