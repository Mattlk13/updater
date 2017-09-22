/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "loadable.h"

#include <QDebug>
#include <QDomDocument>
#include <QRegExp>
#include <QSqlError>
#include <QVariant>     // used by XSqlQuery::value()
#include <limits.h>

#include "metasql.h"
#include "xsqlquery.h"

QRegExp Loadable::trueRegExp("^t(rue)?$",   Qt::CaseInsensitive);
QRegExp Loadable::falseRegExp("^f(alse)?$", Qt::CaseInsensitive);

QString Loadable::_sqlerrtxt = TR("The following error was "
                                  "encountered while trying to import %1 into "
                                  "the database:<br><pre>%2<br>%3</pre>");

Loadable::Loadable(const QString &nodename, const QString &name,
                   const int grade, const bool system, const QString &schema,
                   const QString &comment,
                   const QString &filename)
  : _comment(comment), _grade(grade),       _gradeMql(0),
    _insertMql(0),     _selectMql(0),       _maxMql(0),      _minMql(0),
    _name(name),       _nodename(nodename), _onError(Script::Default),
    _schema(schema),   _stripBOM(true),     _system(system), _updateMql(0)
{
  _filename = (filename.isEmpty() ? name   : filename);
  _schema   = (schema.isEmpty()   ? schema : "public");
}

Loadable::Loadable(const QDomElement &pElem, const bool pSystem,
                   QStringList &pMsg, QList<bool> &pFatal)
  : _grade(0),        _gradeMql(0),
    _insertMql(0),    _selectMql(0),    _maxMql(0), _minMql(0),
    _stripBOM(true),  _system(pSystem), _updateMql(0)
{
  Q_UNUSED(pMsg);
  Q_UNUSED(pFatal);

  _nodename = pElem.nodeName();

  if (pElem.hasAttribute("name"))
    _name   = pElem.attribute("name");

  if (pElem.hasAttribute("grade"))
  {
    if (pElem.attribute("grade").contains("highest", Qt::CaseInsensitive))
      _grade = INT_MAX;
    else if (pElem.attribute("grade").contains("lowest", Qt::CaseInsensitive))
      _grade = INT_MIN;
    else
      _grade = pElem.attribute("grade").toInt();
  }
  else if (pElem.hasAttribute("order"))
  {
    if (pElem.attribute("order").contains("highest", Qt::CaseInsensitive))
      _grade = INT_MAX;
    else if (pElem.attribute("order").contains("lowest", Qt::CaseInsensitive))
      _grade = INT_MIN;
    else
      _grade = pElem.attribute("order").toInt();
  }

  if (pElem.hasAttribute("file"))
    _filename = pElem.attribute("file");
  else
    _filename = _name;

  if (pElem.hasAttribute("schema"))
    _schema = pElem.attribute("schema");

  if (pElem.hasAttribute("onerror"))
    _onError = Script::nameToOnError(pElem.attribute("onerror"));
  else
    _onError = Script::nameToOnError("Stop");

  _comment = pElem.text().trimmed();
}

Loadable::~Loadable()
{
  if (_minMql)    delete _minMql;
  if (_maxMql)    delete _maxMql;
  if (_gradeMql)  delete _gradeMql;
  if (_selectMql) delete _selectMql;
  if (_insertMql) delete _insertMql;
  if (_updateMql) delete _updateMql;
}

QString Loadable::schema() const
{
  return _schema;
}

QDomElement Loadable::createElement(QDomDocument & doc)
{
  QDomElement elem = doc.createElement(_nodename);
  elem.setAttribute("name", _name);
  elem.setAttribute("grade", _grade);
  elem.setAttribute("file", _filename);
  if (! _schema.isEmpty())
    elem.setAttribute("schema", _schema);

  if(!_comment.isEmpty())
    elem.appendChild(doc.createTextNode(_comment));

  return elem;
}

int Loadable::writeToDB(QByteArray &pData, const QString pPkgname,
                        QString &errMsg, ParameterList &pParams)
{
  cleanData(pData);
  const char *fileContent = pData.data();

  pParams.append("name",   _name);
  pParams.append("type",   _pkgitemtype);
  pParams.append("source", QString::fromLocal8Bit(fileContent));
  pParams.append("notes",  _comment);

  // alter the name of the loadable's table if necessary
  QString destschema = "public";
  QString prefix;
  if (_schema.isEmpty()        &&   pPkgname.isEmpty())
    ;   // leave it alone
  else if (_schema.isEmpty()   && ! pPkgname.isEmpty())
  {
    prefix = pPkgname + ".pkg";
    destschema = pPkgname;
  }
  else if ("public" == _schema &&   pPkgname.isEmpty())
    ;   // leave it alone
  else if ("public" == _schema && ! pPkgname.isEmpty())
    prefix = "public.";
  else if (! _schema.isEmpty())
  {
    prefix = _schema + ".pkg";
    destschema = _schema;
  }

  if (! prefix.isEmpty())
  {
    pParams.append("pkgname", destschema);

    // yuck - no Parameter::operator==(Parameter&) and no replace()
    QString tablename = pParams.value("tablename").toString();
    for (int i = 0; i < pParams.size(); i++)
    {
      if (pParams.at(i).name() == "tablename")
      {
        pParams.takeAt(i);
        pParams.append("tablename", prefix + tablename);
        break;
      }
    }
  }

  if (_minMql && _minMql->isValid() && _grade == INT_MIN)
  {
    XSqlQuery minOrder = _minMql->toQuery(pParams);
    if (minOrder.first())
      _grade = minOrder.value(0).toInt();
    else if (minOrder.lastError().type() != QSqlError::NoError)
    {
      QSqlError err = minOrder.lastError();
      errMsg = _sqlerrtxt.arg(_filename).arg(err.driverText()).arg(err.databaseText());
      return -3;
    }
    else
      _grade = 0;
  }
  else if (_maxMql && _maxMql->isValid() && _grade == INT_MAX)
  {
    XSqlQuery maxOrder = _maxMql->toQuery(pParams);
    if (maxOrder.first())
      _grade = maxOrder.value(0).toInt();
    else if (maxOrder.lastError().type() != QSqlError::NoError)
    {
      QSqlError err = maxOrder.lastError();
      errMsg = _sqlerrtxt.arg(_filename).arg(err.driverText()).arg(err.databaseText());
      return -4;
    }
    else
      _grade = 0;
  }

  pParams.append("grade", _grade);

  if (_gradeMql && _gradeMql->isValid())
  {
    XSqlQuery grade;
    grade = _gradeMql->toQuery(pParams);

    if (grade.first())
      _grade = grade.value(0).toInt();
    else if (grade.lastError().type() != QSqlError::NoError)
    {
      QSqlError err = grade.lastError();
      errMsg = _sqlerrtxt.arg(_filename).arg(err.driverText()).arg(err.databaseText());
      return -5;
    }

    for (int i = 0; i < pParams.size(); i++)
    {
      if (pParams.at(i).name() == "grade")
      {
        pParams.takeAt(i);
        pParams.append("grade", _grade);
        break;
      }
    }
  }

  XSqlQuery select;
  int itemid = -1;
  select = _selectMql->toQuery(pParams);

  if (select.first())
    itemid = select.value(0).toInt();
  else if (select.lastError().type() != QSqlError::NoError)
  {
    QSqlError err = select.lastError();
    errMsg = _sqlerrtxt.arg(_filename).arg(err.driverText()).arg(err.databaseText());
    return -5;
  }
  pParams.append("id", itemid);

  XSqlQuery upsert;
  if (itemid >= 0)
    upsert = _updateMql->toQuery(pParams);
  else
    upsert = _insertMql->toQuery(pParams);

  if (upsert.first())
    itemid = upsert.value("id").toInt();
  else if (upsert.lastError().type() != QSqlError::NoError)
  {
    QSqlError err = upsert.lastError();
    errMsg = _sqlerrtxt.arg(_filename)
                .arg(err.driverText())
                .arg(err.databaseText());
    return -7;
  }

  return itemid;
}

QByteArray Loadable::cleanData(QByteArray &pData)
{
  if (_stripBOM && pData.left(3) == "\xEF\xBB\xBF")
  {
    pData.remove(0, 3);
    qWarning() << "Found BOM in" << _name << _comment;
  }
  return pData;
}
