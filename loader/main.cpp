/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QIcon>

#include <dbtools.h>
#include <login2.h>
#include <parameter.h>
#include <xsqlquery.h>

#include "data.h"
#include "loaderwindow.h"
#include "xabstractmessagehandler.h"

QString _databaseURL = "";

int main(int argc, char* argv[])
{
  QSqlDatabase db;
  QString dbName;
  QString hostName;
  QString passwd;
  QString pkgfile;
  QString port;
  QString username;
  XAbstractMessageHandler *handler;
  bool    autoRunArg      = false;
  bool    autoRunCheck    = false;
  bool    debugpkg        = false;
  bool    haveDatabaseURL = false;

  QApplication app(argc, argv);
  app.addLibraryPath(".");
#ifndef Q_WS_MACX
  app.setWindowIcon(QIcon(":/images/updater-32x32.png"));
#endif

  if (argc > 1)
  {
    for (int intCounter = 1; intCounter < argc; intCounter++)
    {
      QString argument(argv[intCounter]);

      if (argument.startsWith("-help", Qt::CaseInsensitive))
      {
        qWarning("%s [ -databaseURL=PSQL7://hostname:port/databasename ]"
                 " [ -h hostname ]"
                 " [ -p port ]"
                 " [ -d databasename ]"
                 " [ -U username | -username=username ]"
                 " [ -passwd=databasePassword ]"
                 " [ -debug ]"
                 " [ -file=updaterFile.gz | -f updaterFile.gz ]"
                 " [ -autorun ]",
                 argv[0]);
        return 0;
      }
      else if (argument.startsWith("-databaseURL=", Qt::CaseInsensitive))
      {
        QString protocol;
        haveDatabaseURL = TRUE;
        _databaseURL    = argument.right(argument.length() - 13);
        parseDatabaseURL(_databaseURL, protocol, hostName, dbName, port);
      }
      else if (argument == "-h")
      {
        hostName = argv[++intCounter];
      }
      else if (argument == "-p")
      {
        port = argv[++intCounter];
      }
      else if (argument == "-d")
      {
        dbName = argv[++intCounter];
      }
      else if (argument == "-U")
      {
        username = argv[++intCounter];
      }
      else if (argument.startsWith("-username=", Qt::CaseInsensitive))
      {
        username = argument.right(argument.length() - 10);
      }
      else if (argument.startsWith("-passwd=", Qt::CaseInsensitive))
      {
        passwd = argument.right(argument.length() - 8);
      }
      else if (argument.toLower() == "-debug")
      {
        debugpkg = true;
      }
      else if (argument == "-f")
      {
        pkgfile = argv[++intCounter];
      }
      else if (argument.startsWith("-file=", Qt::CaseInsensitive))
      {
        pkgfile = argument.right(argument.size() - argument.indexOf("=") - 1);
      }
      else if (argument.toLower() == "-autorun")
        autoRunArg = true;
    }
  }

  LoaderWindow * mainwin = new LoaderWindow();
  mainwin->setDebugPkg(debugpkg);
  mainwin->setCmdline(autoRunArg);
  handler = mainwin->handler();

  db = QSqlDatabase::addDatabase("QPSQL7");
  if (!db.isValid())
  {
    handler->message(QtFatalMsg,
                     QObject::tr("Unable to load the database driver. "
                                 "Please contact your systems administrator."));
    return 1;
  }

  db.setDatabaseName(dbName);
  db.setUserName(username);
  db.setPassword(passwd);
  db.setHostName(hostName);
  db.setPort(port.toInt());

  if (!db.open() && autoRunArg)
  {
    handler->message(QtFatalMsg,
                     QObject::tr("Unable to connect to the database "
                                 "with the given information."));
    return 1;
  }

  if (! db.isOpen())
  {
    ParameterList params;
    params.append("name",      _name);
    params.append("copyright", _copyright.toAscii().data());
    params.append("version",   _version.toAscii().data());
    params.append("build",     __DATE__ " " __TIME__); // use C++ string concat
    params.append("username", username);

    if (haveDatabaseURL)
      params.append("databaseURL", _databaseURL.toAscii().data());

    login2 newdlg(0, "", TRUE);
    newdlg.set(params, 0);

    if (newdlg.exec() == QDialog::Rejected)
      return 2;
    else
    {
      _databaseURL = newdlg._databaseURL;
      username     = newdlg._user;
    }

    QSqlQuery su;
    su.prepare("SELECT rolsuper FROM pg_roles WHERE (rolname=:user);");
    su.bindValue(":user", username);
    su.exec();
    if (su.first())
    {
      if (! su.value(0).toBool() &&
          handler->question(QObject::tr("You are not logged in as a "
                                        "database super user. The update "
                                        "may fail. Are you sure you want "
                                        "to continue?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::No)
        return 3;
    }
    else if (su.lastError().type() != QSqlError::NoError &&
             handler->question(QObject::tr("<p>The application received a database "
                                           "error while trying to check the user "
                                           "status of %1. Would you like to try to "
                                           "update anyway?</p><pre>%2</pre>")
                            .arg(username, su.lastError().databaseText()),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::No)
      return 4;
  }

  if (! pkgfile.isEmpty())
  {
    autoRunCheck = mainwin->openFile(pkgfile);
  }

  if (autoRunArg)
  {
    bool successful = autoRunCheck && ! pkgfile.isEmpty();
    if (successful)
    {
      successful = mainwin->sStart();
    }
    if (successful)     // not else if
      return 0;
    else
    {
#ifdef Q_OS_WIN32
      mainwin->show();
#else
      qWarning("%s", qPrintable(mainwin->_text->toPlainText()));
      return 5;
#endif
    }
  }
  else
    mainwin->show();

  return app.exec();
}
