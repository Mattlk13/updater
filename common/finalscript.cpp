/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2019 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "finalscript.h"

#include <QDomDocument>
#include <QSqlError>

#define DEBUG false

FinalScript::FinalScript(const QString & name, OnError onError, const QString & comment)
  : Script(name, onError, comment)
{
  _nodename = "finalscript";
}

FinalScript::FinalScript(const QDomElement & elem, QStringList &msg, QList<bool> &fatal)
{
  _nodename = "finalscript";
  _name = elem.attribute("name");
  if (elem.hasAttribute("file"))
    _name = elem.attribute("file");
  _onError = nameToOnError(elem.attribute("onerror"));
  _comment = elem.text();

  if (_name.isEmpty())
  {
    msg.append(TR("This finalscript does not have a file or name attribute."));
    fatal.append(true);
  }
}

FinalScript::~FinalScript()
{
}

QDomElement FinalScript::createElement(QDomDocument & doc)
{
  QDomElement elem = doc.createElement(_nodename);

  elem.setAttribute("file", _name);
  elem.setAttribute("onerror", onErrorToName(_onError));

  if(!_comment.isEmpty())
    elem.appendChild(doc.createTextNode(_comment));

  return elem;
}
