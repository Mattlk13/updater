/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "cmdlinemessagehandler.h"

#include <QDebug>
#include <string> // std c++ library!

class CmdLineMessageHandlerPrivate
{
  public:
    CmdLineMessageHandlerPrivate()
    {
    }
};

/* By default, all messages will be displayed in QMessageBoxes. Override this
   with calls to setDestination(QtMsgType, QWidget*).
 */
CmdLineMessageHandler::CmdLineMessageHandler(QObject *parent)
  : QAbstractMessageHandler(parent)
{
  _p = new CmdLineMessageHandlerPrivate();
}

CmdLineMessageHandler::~CmdLineMessageHandler()
{
  delete _p;
}

void CmdLineMessageHandler::handleMessage(QtMsgType type,
                   const QString &description,
                   const QUrl    &identifier,
                   const QSourceLocation &sourceLocation)
{
  Q_UNUSED(identifier);
  Q_UNUSED(sourceLocation);
  switch (type)
  {
    case QtDebugMsg:     qDebug()    << description; break;
    case QtWarningMsg:   qWarning()  << description; break;
    case QtFatalMsg:     
      {
        const char *str = description.toStdString().c_str();
        qFatal(str);
        break;
      }
    case QtCriticalMsg:
    default:             qCritical() << description; break;
  }
}
