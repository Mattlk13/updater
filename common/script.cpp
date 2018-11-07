/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "script.h"

#include <QDebug>
#include <QDomDocument>
#include <QSqlError>

#include "metasql.h"
#include "xsqlquery.h"

#define DEBUG false

QString Script::_sqlerrtxt = TR("The following error was encountered "
                                         "while trying to import %1 into the "
                                         "database:<br><pre>%2<br>%3</pre>");

Script::Script(const QString & name, OnError onError, const QString & comment)
  : _name(name), _comment(comment), _onError(onError), _stripBOM(true)
{
}

Script::Script(const QDomElement & elem, QStringList &msg, QList<bool> &fatal)
  : _onError(Script::Default), _stripBOM(true)
{
  _name = elem.attribute("name");
  if (elem.hasAttribute("file"))
    _name = elem.attribute("file");

  if (elem.hasAttribute("onerror"))
    _onError = nameToOnError(elem.attribute("onerror"));

  _comment = elem.text();

  if (_name.isEmpty())
  {
    msg.append(TR("This script does not have a name."));
    fatal.append(true);
  }
}

Script::~Script()
{
}

QString Script::filename() const
{
  return _name; // _name and _filename are interchangable
}

QDomElement Script::createElement(QDomDocument & doc)
{
  QDomElement elem = doc.createElement("script");

  elem.setAttribute("name", _name);
  elem.setAttribute("file", _name);
  elem.setAttribute("onerror", onErrorToName(_onError));

  if(!_comment.isEmpty())
    elem.appendChild(doc.createTextNode(_comment));

  return elem;
}

QString Script::onErrorToName(OnError onError)
{
  QString str = "Default";
  if(Stop == onError)
    str = "Stop";
  else if(Prompt == onError)
    str = "Prompt";
  else if(Ignore == onError)
    str = "Ignore";
  return str;
}

Script::OnError Script::nameToOnError(const QString & name)
{
  if("Stop" == name)
    return Stop;
  else if("Prompt" == name)
    return Prompt;
  else if("Ignore" == name)
    return Ignore;
  return Default;
}

QStringList Script::onErrorList(bool includeDefault)
{
  QStringList list;
  if(includeDefault)
    list << "Default";
  list << "Stop";
  list << "Prompt";
  list << "Ignore";
  return list;
}

int Script::writeToDB(QByteArray &pData, const QString pAnnotation, ParameterList &pParams, QString &errMsg)
{
  Q_UNUSED(pParams);
  if (DEBUG)
    qDebug() << "Script::writeToDb(" << pData << pAnnotation << "params" << errMsg
             << ") with onError" << _onError;
  if (pData.isEmpty())
  {
    errMsg = TR("The file %1 is empty.").arg(filename());
    return -1;
  }

  cleanData(pData);
  char *data = pData.data();
  XSqlQuery create;
  create.exec(QString::fromLocal8Bit(data));
  if (create.lastError().type() != QSqlError::NoError)
  {
    errMsg = _sqlerrtxt.arg(filename())
                       .arg(create.lastError().databaseText())
                       .arg(create.lastError().driverText());
    return -3;
  }

  return 0;
}

QByteArray Script::cleanData(QByteArray &pData)
{
  if (_stripBOM && pData.left(3) == "\xEF\xBB\xBF")
  {
    pData.remove(0, 3);
    qWarning() << "Found BOM in" << _name << _comment;
  }
  return pData;
}
