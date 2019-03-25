/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2019 by OpenMFG LLC, d/b/a xTuple.
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

#include "updaterdata.h"
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
  bool    acceptDefaults  = false;

  QApplication app(argc, argv);
  app.addLibraryPath(".");
#ifndef Q_OS_MAC
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
                 " [ -autorun [ -D ] ]",
                 argv[0]);
        return 0;
      }
      else if (argument.startsWith("-databaseURL=", Qt::CaseInsensitive))
      {
        QString protocol;
        haveDatabaseURL = true;
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
      {
        autoRunArg = true;
      }
      else if (argument == "-D")
      {
        acceptDefaults = true;
      }
    }
  }

  LoaderWindow * mainwin = new LoaderWindow();
  mainwin->setDebugPkg(debugpkg);
  mainwin->setCmdline(autoRunArg);
  handler = mainwin->handler();
  handler->setAcceptDefaults(autoRunArg && acceptDefaults);

  ParameterList params;
  params.append("earliest", "9.3.0");
  params.append("latest", "11.0.0");
  params.append("name",      Updater::name);
  params.append("copyright", Updater::copyright);
  params.append("version",   Updater::version);
  params.append("build",     Updater::build);
  if (username.length() > 0)
    params.append("username",  username);
  params.append("password",  passwd);

  if (haveDatabaseURL)
    params.append("databaseURL", _databaseURL.toLatin1().data());
  
  if (autoRunArg)
  {
    if (!haveDatabaseURL)
    {
      buildDatabaseURL(_databaseURL, "psql", hostName, dbName, port);
      params.append("databaseURL", _databaseURL.toLatin1().data());
    }
    params.append("cmd");
    params.append("login");
  }

  login2 newdlg(0, "", true);
  newdlg.set(params, 0);

  if (!QSqlDatabase::database(QSqlDatabase::defaultConnection, false).isOpen() && autoRunArg)
  {
    handler->message(QtFatalMsg,
                     QObject::tr("Unable to connect to the database "
                                 "with the given information."));
    return 1;
  }

  if (!autoRunArg)
  {
    if (newdlg.exec() == QDialog::Rejected)
      return 2;

    _databaseURL = newdlg._databaseURL;
    username     = newdlg._user;
  }

  Updater::loggedIn = true;
  mainwin->setWindowTitle();

  QSqlQuery set("SET standard_conforming_strings TO true;");
  if (set.lastError().type() != QSqlError::NoError)
    handler->message(QtWarningMsg,
                     QObject::tr("Unable to set standard_conforming_strings. "
                                 "Updates may fail with unexpected errors."));

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
