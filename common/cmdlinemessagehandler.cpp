/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

// std c++ libraries
#include <cstdlib>
#include <iostream>
#include <string>

#include "cmdlinemessagehandler.h"

#include <QDebug>
#include <QTextDocument>

class CmdLineMessageHandlerPrivate
{
  public:
    CmdLineMessageHandlerPrivate()
    {
      if (buttonMap.isEmpty()) {
        buttonMap.insert(QMessageBox::Ok,              QObject::tr("Ok"));
        buttonMap.insert(QMessageBox::Open,            QObject::tr("Open"));
        buttonMap.insert(QMessageBox::Save,            QObject::tr("Save"));
        buttonMap.insert(QMessageBox::Cancel,          QObject::tr("Cancel"));
        buttonMap.insert(QMessageBox::Close,           QObject::tr("Close"));
        buttonMap.insert(QMessageBox::Discard,         QObject::tr("Discard"));
        buttonMap.insert(QMessageBox::Apply,           QObject::tr("Apply"));
        buttonMap.insert(QMessageBox::Reset,           QObject::tr("Reset"));
        buttonMap.insert(QMessageBox::RestoreDefaults, QObject::tr("Restore Defaults"));
        buttonMap.insert(QMessageBox::Help,            QObject::tr("Help"));
        buttonMap.insert(QMessageBox::SaveAll,         QObject::tr("SaveAll"));
        buttonMap.insert(QMessageBox::Yes,             QObject::tr("Yes"));
        buttonMap.insert(QMessageBox::YesToAll,        QObject::tr("Yes To All"));
        buttonMap.insert(QMessageBox::No,              QObject::tr("No"));
        buttonMap.insert(QMessageBox::NoToAll,         QObject::tr("No To All"));
        buttonMap.insert(QMessageBox::Abort,           QObject::tr("Abort"));
        buttonMap.insert(QMessageBox::Retry,           QObject::tr("Retry"));
        buttonMap.insert(QMessageBox::Ignore,          QObject::tr("Ignore"));
      }
    }

    static QMap<QMessageBox::StandardButton, QString> buttonMap;
};

QMap<QMessageBox::StandardButton, QString> CmdLineMessageHandlerPrivate::buttonMap;

/* By default, all messages will be displayed in QMessageBoxes. Override this
   with calls to setDestination(QtMsgType, QWidget*).
 */
CmdLineMessageHandler::CmdLineMessageHandler(QObject *parent)
  : XAbstractMessageHandler(parent)
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
  QTextDocument tmpdoc;
  tmpdoc.setHtml(description);
  std::string msg = tmpdoc.toPlainText().trimmed().toStdString();
  switch (type)
  {
    case QtDebugMsg:     std::cout << "Debug:"    << msg << "\n"; break;
    case QtFatalMsg:     std::cout << "Error:"    << msg << "\n"; break;
    case QtWarningMsg:
    case QtCriticalMsg:
    default:             std::cout << msg << "\n"; break;
  }
}

QMessageBox::StandardButton CmdLineMessageHandler::question(const QString &question, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
  QMap<int, QMessageBox::StandardButton> choice;
  int defaultIdx = 0;
  int idx = 1;
  QTextDocument tmpdoc;
  tmpdoc.setHtml(question);
  std::string msg = tmpdoc.toPlainText().trimmed().toStdString();
  foreach (const QMessageBox::StandardButton &key, _p->buttonMap.keys())
  {
    if (buttons & key)
    {
      choice.insert(idx, key);
      if (key == defaultButton)
        defaultIdx = idx;
      idx++;
    }
  }

  QString qsprompt;
  foreach (int key, choice.keys())
  {
    qsprompt = qsprompt.append("\n")
                       .append(tr("%1:\t%2)")
                           .arg(key)
                           .arg(_p->buttonMap.value(choice.value(key))));
  }
  qsprompt = qsprompt.append("\n")
                     .append(tr("[%1]").arg(defaultIdx));

  int selection = -1;
  std::string input;
  while (! choice.contains(selection))
  {
    std::cout << msg << "\n" << qsprompt.toStdString() << " ";

    std::cin >> input;
    selection = input.empty() ? defaultIdx : atoi(input.c_str());
  }

  return choice.value(selection);
}
