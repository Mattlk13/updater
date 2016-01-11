/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "loaderwindow.h"

#include <QDomDocument>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QMessageBox>
#include <QProcess>
#include <QRegExp>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTimerEvent>
#include <QDateTime>

#include <dbtools.h>
#include <cmdlinemessagehandler.h>
#include <guimessagehandler.h>
#include <gunzip.h>
#include <createfunction.h>
#include <createtable.h>
#include <createtrigger.h>
#include <createview.h>
#include <finalscript.h>
#include <initscript.h>
#include <loadappscript.h>
#include <loadappui.h>
#include <loadcmd.h>
#include <loadimage.h>
#include <loadmetasql.h>
#include <loadpriv.h>
#include <loadreport.h>
#include <package.h>
#include <pkgschema.h>
#include <prerequisite.h>
#include <script.h>
#include <tarfile.h>
#include <xsqlquery.h>

#include "data.h"

#include "xsqlquery.h"

#define DEBUG false

#if defined(Q_OS_WIN32)
#define NOCRYPT
#include <windows.h>
#undef LoadImage
#else
#if defined(Q_OS_MACX)
#include <stdlib.h>
#endif
#endif

extern QString _databaseURL;

QString LoaderWindow::_rollbackMsg(tr("<p><font color='red'>The upgrade has "
                                      "been aborted due to an error and your "
                                      "database was rolled back to the state "
                                      "it was in when the upgrade was "
                                      "initiated.</font><br>"));

class LoaderWindowPrivate
{
  private:
    LoaderWindow *_p;

  public:
    LoaderWindowPrivate(LoaderWindow *parent)
      : _p(parent),
        handler(0)
    {
      setCmdline(false);
    }

    ~LoaderWindowPrivate()
    {
      delete handler;
    }

    void setCmdline(bool p)
    {
      useCmdline = p;
      GuiMessageHandler *g = dynamic_cast<GuiMessageHandler *>(handler);
      if (useCmdline)
      {
        if (g)
          delete handler;
        handler = new CmdLineMessageHandler(_p);
      }
      else
      {
        if (! g)
          delete handler;
        g = new GuiMessageHandler(_p);
        g->setDestination(QtWarningMsg, _p->_text);
        g->setDestination(QtDebugMsg,   _p->_status);
        handler = g;
      }
    }

    QString elapsedTime(QDateTime startTime, QDateTime endTime)
    {
      int elapsed = startTime.secsTo(endTime);
      int sec = elapsed % 60;
      elapsed = (elapsed - sec) / 60;
      int min = elapsed % 60;
      elapsed = (elapsed - min) / 60;
      int hour = elapsed;
      return _p->tr("<p>Total elapsed time is %1h %2m %3s</p>").arg(hour).arg(min).arg(sec);
    }

    int disableTriggers();
    int enableTriggers();

    XAbstractMessageHandler *handler;
    int         dbTimerId;
    bool        multitrans;
    QStringList triggers;      // to be disabled and enabled
    bool        useCmdline;
};

LoaderWindow::LoaderWindow(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : QMainWindow(parent, fl)
{
  setupUi(this);
  setObjectName(name);
  _p = new LoaderWindowPrivate(this);

  (void)statusBar();

  _p->multitrans = false;
  _package = 0;
  _files = 0;
  _p->dbTimerId = startTimer(60000);
  fileNew();

  setWindowTitle();
}

LoaderWindow::~LoaderWindow()
{
  delete _p;
}

void LoaderWindow::languageChange()
{
  retranslateUi(this);
}

XAbstractMessageHandler *LoaderWindow::handler() const
{
  return _p->handler;
}

void LoaderWindow::fileNew()
{
  // we don't actually create files here but we are using this as the
  // stub to unload and properly setup the UI to respond correctly to
  // having no package currently loaded.
  if(_package != 0)
  {
    delete _package;
    _package = 0;
  }

  if(_files != 0)
  {
    delete _files;
    _files = 0;
  }

  _pkgname->setText(tr("No Package is currently loaded."));

  _status->clear();
  _status->setEnabled(false);

  _progress->setValue(0);
  _progress->setEnabled(false);

  _text->clear();
  _text->setEnabled(false);

  _start->setEnabled(false);
}

bool LoaderWindow::openFile(QString pfilename)
{
  fileNew();
  
  QFileInfo fi(pfilename);
  if (fi.filePath().isEmpty())
    return false;
    
  QByteArray data = gunzipFile(fi.filePath());
  if(data.isEmpty())
  {
    _p->handler->message(QtFatalMsg,
                         tr("<p>The file %1 appears to be empty or it is not "
                            "compressed in the expected format.")
                         .arg(fi.filePath()));
    return false;
  }

  _files = new TarFile(data);
  if(!_files->isValid())
  {
    _p->handler->message(QtFatalMsg,
                         tr("<p>The file %1 does not appear to contain a valid "
                            "update package (not a valid TAR file?).")
                         .arg(fi.filePath()));
    delete _files;
    _files = 0;
    return false;
  }

  // find the content file
  QStringList list = _files->_list.keys();
  QString contentFile = QString::null;
  QStringList contentsnames;
  contentsnames << "package.xml" << "contents.xml";
  for (int i = 0; i < contentsnames.size() && contentFile.isNull(); i++)
  {
    foreach (QString mit, list)
    {
      QFileInfo fi(mit);
      if(fi.fileName() == contentsnames.at(i))
      {
        if(!contentFile.isNull())
        {
          _p->handler->message(QtFatalMsg,
                               tr("<p>Multiple %1 files found in %2. "
                                  "Currently only packages containing a single "
                                  "content.xml file are supported.")
                               .arg(contentsnames.at(i)).arg(fi.filePath()));
          delete _files;
          _files = 0;
          return false;
        }
        contentFile = mit;
      }
    }
  }

  QStringList msgList;
  QList<bool> fatalList;

  if(contentFile.isNull())
  {
    _p->handler->message(QtFatalMsg,
                         tr("<p>No %1 file was found in package %2.")
                         .arg(contentsnames.join(" or ")).arg(fi.filePath()));
    delete _files;
    _files = 0;
    return false;
  }
  else if (! contentFile.endsWith(contentsnames.at(0)))
  {
    qDebug("Deprecated Package Format: Packages for this version of "
           "the Updater should have their contents described by a file "
           "named %s. The current package being loaded uses an outdated "
           "file name %s.",
           qPrintable(contentsnames.at(0)), qPrintable(contentFile));
  }

  QByteArray docData = _files->_list[contentFile];
  QDomDocument doc;
  QString errMsg;
  int errLine, errCol;
  if(!doc.setContent(docData, &errMsg, &errLine, &errCol))
  {
    _p->handler->message(QtFatalMsg,
                         tr("<p>There was a problem reading the %1 file in "
                            "this package.<br>%2<br>Line %3, Column %4")
                         .arg(contentFile).arg(errMsg)
                         .arg(errLine).arg(errCol));
    delete _files;
    _files = 0;
    return false;
  }

  _text->clear();
  _text->setEnabled(true);

  QString delayedWarning;
  _package = new Package(doc.documentElement(), msgList, fatalList, _p->handler);
  if (msgList.size() > 0)
  {
    bool fatal = false;
    if (DEBUG)
      qDebug("LoaderWindow::fileOpen()  i fatal msg");
    for (int i = 0; i < msgList.size(); i++)
    {
      _p->handler->message(QtWarningMsg,
                  QString("<br><font color='%1'>%2</font>")
                    .arg(fatalList.at(i) ? "red" : "orange")
                    .arg(msgList.at(i)));
      fatal = fatal || fatalList.at(i);
      if (DEBUG)
        qDebug("LoaderWindow::fileOpen() %2d %5d %s",
               i, fatalList.at(i), qPrintable(msgList.at(i)));
    }
    if (fatal)
    {
      _p->handler->message(QtWarningMsg,
          tr("<p><font color='red'>The %1 file appears "
                       "to be invalid.</font></p>").arg(contentFile));
      return false;
    }
    else
      delayedWarning = tr("<p><font color='orange'>The %1 file "
                          "seems to have problems. You should contact %2 "
                          "before proceeding.</font></p>")
                      .arg(contentFile)
                      .arg(_package->developer().isEmpty() ?
                           tr("the package developer") : _package->developer());
  }

  _pkgname->setText(tr("Package %1 (%2)").arg(_package->id()).arg(fi.filePath()));

  _progress->setValue(0);
  _progress->setMaximum( _package->_privs.size()
                       + _package->_metasqls.size()
                       + _package->_reports.size()
                       + _package->_appuis.size()
                       + _package->_appscripts.size()
                       + _package->_cmds.size()
                       + _package->_images.size()
                       + _package->_prerequisites.size()
                       + _package->_initscripts.size()
                       + _package->_scripts.size()
                       + _package->_functions.size()
                       + _package->_tables.size()
                       + _package->_triggers.size()
                       + _package->_views.size()
                       + _package->_finalscripts.size()
                       + 2);
  _progress->setEnabled(true);
  if (DEBUG)
    qDebug("LoaderWindow::fileOpen() progress initialized to max %d",
           _progress->maximum());

  _status->setEnabled(true);
  _p->handler->message(QtWarningMsg, "<h3>Checking Prerequisites...</h3>");
  bool allOk = true;

  QString str;
  XSqlQuery qry;
  if (_package->_prerequisites.size() > 0)
  {
    foreach (Prerequisite *i, _package->_prerequisites)
    {
      _p->handler->message(QtWarningMsg, tr("checking %1<br/>").arg(i->name()));
      if (! i->met(errMsg, _p->handler))
      {
        allOk = false;
        str = QString("<font size='+1' color='red'><b>Failed</b></font>");
        if (! errMsg.isEmpty())
         str += tr("<p>%1</p>").arg(errMsg);

        QStringList strlist = i->providerList();
        if (! strlist.isEmpty())
        {
          str += tr("<b>Requires:</b>");
          str += "<ul>";
          foreach (QString slit, strlist)
            str += tr("<li>%1: %2</li>").arg(i->provider(slit).package(), i->provider(slit).info());
          str += "</ul>";
        }
        
        _p->handler->message(QtWarningMsg, str);
        if (DEBUG)
          qDebug("%s", qPrintable(str));
      }
    }
  }

  if (! allOk)
  {
    _p->handler->message(QtFatalMsg,
                         tr("<p>One or more prerequisite checks <b>FAILED</b>. "
                            "These prerequisites must be satisified before continuing.</p>"));
    return false;
  }

  _p->handler->message(QtDebugMsg, tr("<p>Prerequisite Checks completed.</p>"));
  if (delayedWarning.isEmpty())
    _p->handler->message(QtWarningMsg,
        tr("<h2><font color='green'>Ready to Start update!</font></h2>"));
  else
  {
    _p->handler->message(QtWarningMsg, tr("<h2>Ready to Start update!</h2>"));
    _p->handler->message(QtWarningMsg, delayedWarning);
  }
  _p->handler->message(QtWarningMsg,
      tr("<p><b>NOTE</b>: Have you backed up your database? If not, you should "
                   "backup your database now. It is good practice to backup a database "
                   "before updating it.</p><hr/>"));

  _start->setEnabled(true);
  return true;
}

void LoaderWindow::fileOpen()
{
  fileNew();
  
  QSettings settings("xTuple.com", "Updater");
  QString path = settings.value("LastDirectory").toString();

  QString filename = QFileDialog::getOpenFileName(this,
                                                  tr("Open Package"), path,
                                                  tr("Package Files (*.gz);;All Files (*.*)"));

  if (! openFile(filename))
    return;
    
  QFileInfo fi(filename);
  settings.setValue("LastDirectory", fi.path());
}


void LoaderWindow::fileExit()
{
  qApp->closeAllWindows();
}


void LoaderWindow::helpContents()
{
  launchBrowser(this, "http://www.xtuple.org/UpdaterDoc");
}

// TODO: put in a generic place and use both from there or use WebKit instead
void LoaderWindow::launchBrowser(QWidget * w, const QString & url)
{
#if defined(Q_OS_WIN32)
  // Windows - let the OS do the work
  QT_WA( {
      ShellExecute(w->winId(), 0, (TCHAR*)url.utf16(), 0, 0, SW_SHOWNORMAL );
    } , {
      ShellExecuteA( w->winId(), 0, url.toLocal8Bit(), 0, 0, SW_SHOWNORMAL );
    } );
#else
  QString b(getenv("BROWSER"));
  QStringList browser;
  if (! b.isEmpty())
    browser = b.split(':');

#if defined(Q_OS_MACX)
  browser.append("/usr/bin/open");
#else
  // append this on linux just as a good guess
  browser.append("/usr/bin/firefox");
  browser.append("/usr/bin/mozilla");
#endif
  foreach (QString app, browser) {
    if(app.contains("%s")) {
      app.replace("%s", url);
    } else {
      app += " " + url;
    }
    app.replace("%%", "%");
    QProcess *proc = new QProcess(w);
    QStringList args = app.split(QRegExp(" +"));
    QString appname = args.takeFirst();

    proc->start(appname, args);
    if (proc->waitForStarted() &&
        proc->waitForFinished())
      return;

    _p->handler->message(QtFatalMsg,
                         tr("<p>Before you can run a web browser you must "
                            "set the environment variable BROWSER to point "
                            "to the browser executable.") );
  }
#endif  // if not windows
}

void LoaderWindow::helpAbout()
{
  QMessageBox::about(this, Updater::name,
    tr("<p>Apply update packages to your xTuple ERP database."
       "<p>Version %1</p>"
       "<p>%2</p>"
       "All Rights Reserved")
    .arg(Updater::version).arg(Updater::copyright));
}

void LoaderWindow::timerEvent( QTimerEvent * e )
{
  if(e->timerId() == _p->dbTimerId)
  {
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection, false);
    if(db.isValid())
      XSqlQuery qry("SELECT CURRENT_DATE;");
    // if we are not connected then we have some problems!
  }
}

// used only in LoaderWindow::sStart()
struct dbobj {
  QString header;
  QString footer;
  QList<Script*>   scriptlist;
  QList<Loadable*> loadablelist;

  dbobj(QString h, QString s, QList<Script*>   l) : header(h), footer(s), scriptlist(l)   {}
  dbobj(QString h, QString s, QList<Loadable*> l) : header(h), footer(s), loadablelist(l) {}
};

bool LoaderWindow::sStart()
{
  bool returnValue = false;

  _start->setEnabled(false);

  QDateTime startTime = QDateTime::currentDateTime();
  _p->handler->message(QtWarningMsg,
      tr("<p>Starting Update at %1</p>").arg(startTime.toString()));

  QString prefix = QString::null;
  if(!_package->id().isEmpty())
    prefix = _package->id() + "/";

  XSqlQuery qry;
  qry.exec("begin;");

  PkgSchema schema(_package->name(),
                   tr("Schema to hold contents of %1").arg(_package->name()));
  QString errMsg;
  int pkgid = -1;
  if (! _package->name().isEmpty())
  {
    pkgid = _package->writeToDB(errMsg);
    if (pkgid >= 0)
      _p->handler->message(QtWarningMsg, tr("Saving Package Header was successful."));
    else
    {
      _p->handler->message(QtWarningMsg, errMsg);
      qry.exec("rollback;");
      _p->handler->message(QtWarningMsg, _rollbackMsg);
      return false;
    }

    if (schema.create(errMsg) >= 0 && schema.setPath(errMsg) >= 0)
      _p->handler->message(QtWarningMsg, tr("Saving Schema for Package was successful."));
    else
    {
      _p->handler->message(QtWarningMsg, errMsg);
      qry.exec("rollback;");
      _p->handler->message(QtWarningMsg, _rollbackMsg);
    }
  }

  int ignoredErrCnt = 0;
  int tmpReturn     = 0;

  if (_package->_initscripts.size() > 0)
  {
    _p->handler->message(QtWarningMsg, tr("<h3>Applying initialization scripts...</h3>"));
    foreach (Script *i, _package->_initscripts)
    {
      _p->handler->message(QtDebugMsg, tr("applying %1<br/>").arg(i->filename()));
      tmpReturn = applySql(i, _files->_list[prefix + i->filename()]);
      if (tmpReturn < 0)
      {
        qry.exec("ROLLBACK;");
        _p->handler->message(QtWarningMsg, _rollbackMsg);
        return false;
      }
      else
        ignoredErrCnt += tmpReturn;
    }
    _p->handler->message(QtWarningMsg, tr("<p>Finished initialization scripts</p>"));
    if (DEBUG)
      qDebug("LoaderWindow::sStart() progress %d out of %d",
             _progress->value(), _progress->maximum());
  }

  if (_p->disableTriggers() < 0)
  {
    qry.exec("ROLLBACK;");
    _p->handler->message(QtWarningMsg, _rollbackMsg);
    return false;
  }

  if (_package->_privs.size() > 0)
  {
    _p->handler->message(QtWarningMsg, tr("<h3>Loading Privileges...</h3>"));
    foreach (Loadable *i, _package->_privs)
    {
      tmpReturn = applyLoadable(i, _files->_list[prefix + i->filename()]);
      if (tmpReturn < 0) {
        qry.exec("ROLLBACK;");
        _p->handler->message(QtWarningMsg, _rollbackMsg);
        return false;
      }
      else
        ignoredErrCnt += tmpReturn;
    }
    _p->handler->message(QtWarningMsg, tr("<p>Finished Privileges</p>"));
    if (DEBUG)
      qDebug("LoaderWindow::sStart() progress %d out of %d",
             _progress->value(), _progress->maximum());
  }

  QList<dbobj> scriptobjs;
  scriptobjs
    << dbobj(tr("Applying database scripts..."),    tr("Finished database scripts"),     _package->_scripts)
    << dbobj(tr("Loading Function definitions..."), tr("Finished Function definitions"), _package->_functions)
    << dbobj(tr("Loading Table definitions..."),    tr("Finished Table definitions"),    _package->_tables)
    << dbobj(tr("Loading Trigger definitions..."),  tr("Finished Trigger definitions"),  _package->_triggers)
    << dbobj(tr("Loading View definitions..."),     tr("Finished View definitions"),     _package->_views)
    ;

  foreach (dbobj objdesc, scriptobjs)
  {
    if (objdesc.scriptlist.size() > 0)
    {
      _p->handler->message(QtWarningMsg, tr("<h3>%1</h3>").arg(objdesc.header));
      foreach(Script *i, objdesc.scriptlist)
      {
        _p->handler->message(QtDebugMsg, tr("applying %1<br/>").arg(i->filename()));
        tmpReturn = applySql(i, _files->_list[prefix + i->filename()]);
        if (tmpReturn < 0) {
          qry.exec("ROLLBACK;");
          _p->handler->message(QtWarningMsg, _rollbackMsg);
          return false;
        }
        else
          ignoredErrCnt += tmpReturn;
      }
      _p->handler->message(QtWarningMsg, tr("<p>%1</p>").arg(objdesc.footer));
    }
  }

  QList<dbobj> loadableobjs;
  loadableobjs
    << dbobj(tr("Loading MetaSQL statements..."),   tr("Finished MetaSQL statements"),   _package->_metasqls)
    << dbobj(tr("Loading Report definitions..."),   tr("Finished Report definitions"),   _package->_reports)
    << dbobj(tr("Loading User Interface forms..."), tr("Finished User Interface forms"), _package->_appuis)
    << dbobj(tr("Loading Application scripts..."),  tr("Finished Application scripts"),  _package->_appscripts)
    << dbobj(tr("Loading Images..."),               tr("Finished loading Images"),       _package->_images)
    ;
  foreach (dbobj objdesc, loadableobjs)
  {
    if (objdesc.loadablelist.size() > 0)
    {
      _p->handler->message(QtWarningMsg, tr("<h3>%1</h3>").arg(objdesc.header));
      foreach (Loadable *i, objdesc.loadablelist)
      {
        _p->handler->message(QtDebugMsg, tr("applying %1<br/>").arg(i->filename()));
        tmpReturn = applyLoadable(i, _files->_list[prefix + i->filename()]);
        if (tmpReturn < 0) {
          qry.exec("ROLLBACK;");
          _p->handler->message(QtWarningMsg, _rollbackMsg);
          return false;
        }
        else
          ignoredErrCnt += tmpReturn;
      }
      _p->handler->message(QtWarningMsg, tr("<p>%1</p>").arg(objdesc.footer));
    }
    if (DEBUG)
      qDebug("LoaderWindow::sStart() progress %d out of %d", _progress->value(), _progress->maximum());
  }

  if (_package->_cmds.size() > 0)
  {
    _p->handler->message(QtWarningMsg, tr("<h3>Loading Custom Commands...</h3>"));
    if (! _package->system() &&
        (! qry.exec("ALTER TABLE pkgcmd DISABLE TRIGGER pkgcmdaltertrigger;") ||
         ! qry.exec("ALTER TABLE pkgcmdarg DISABLE TRIGGER pkgcmdargaltertrigger;")))
    {
      qry.exec("ROLLBACK;");
      _p->handler->message(QtWarningMsg, _rollbackMsg);
      return false;
    }
    foreach (Loadable *i, _package->_cmds)
    {
      tmpReturn = applyLoadable(i, _files->_list[prefix + i->filename()]);
      if (tmpReturn < 0) {
        qry.exec("ROLLBACK;");
        _p->handler->message(QtWarningMsg, _rollbackMsg);
        return false;
      }
      else
        ignoredErrCnt += tmpReturn;
    }
    XSqlQuery qry("SELECT updateCustomPrivs();");
    _p->handler->message(QtWarningMsg, tr("<p>Finished Custom Commands</p>"));
    if (DEBUG)
      qDebug("LoaderWindow::sStart() progress %d out of %d",
             _progress->value(), _progress->maximum());
  }

  if (_package->_prerequisites.size() > 0)
  {
    _p->handler->message(QtWarningMsg, tr("<h3>Loading Package Dependencies...</h3>"));
    foreach (Prerequisite *i, _package->_prerequisites)
    {
      if (i->type() == Prerequisite::Dependency)
      {
        _p->handler->message(QtDebugMsg, tr("applying dependency %1<br/>").arg(i->name()));
        if (i->writeToDB(_package->name(), errMsg) < 0)
        {
          _p->handler->message(QtWarningMsg, errMsg);
          qry.exec("rollback;");
          _p->handler->message(QtWarningMsg, _rollbackMsg);
          return false;
        }
      }
      _progress->setValue(_progress->value() + 1);
    }
    _p->handler->message(QtWarningMsg, tr("<p>Completed updating dependencies.</p>"));
    if (DEBUG)
      qDebug("LoaderWindow::sStart() progress %d out of %d",
             _progress->value(), _progress->maximum());
  }

  if (_p->enableTriggers() < 0)
  {
    qry.exec("ROLLBACK;");
    _p->handler->message(QtWarningMsg, _rollbackMsg);
    return false;
  }

  if (_package->_finalscripts.size() > 0)
  {
    _p->handler->message(QtWarningMsg, tr("<h3>Applying final cleanup scripts...</h3>"));
    foreach (Script *i, _package->_finalscripts)
    {
      _p->handler->message(QtDebugMsg, tr("applying %1<br/>").arg(i->filename()));
      tmpReturn = applySql(i, _files->_list[prefix + i->filename()]);
      if (tmpReturn < 0)
        return false;
      else
        ignoredErrCnt += tmpReturn;
    }
    _p->handler->message(QtWarningMsg, tr("<p>Finished final cleanup</p>"));
    if (DEBUG)
      qDebug("LoaderWindow::sStart() progress %d out of %d",
             _progress->value(), _progress->maximum());
  }

  _progress->setValue(_progress->value() + 1);

  if (_alwaysrollback->isChecked())
  {
    qry.exec("rollback;");
    _p->handler->message(QtWarningMsg, tr("<h2>The Update has been rolled back as requested.</h2>"));
    returnValue = true;
  }
  else if (ignoredErrCnt > 0 &&
           _p->handler->question(tr("<h2>One or more errors were ignored while "
                                    "processing this Package. Are you sure you "
                                    "want to commit these changes?</h2><p>If you "
                                    "answer 'No' then this import will be rolled "
                                    "back.</p>"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::Yes)
  {
    qry.exec("commit;");
    _p->handler->message(QtWarningMsg,
        tr("<h2>The Update is now complete but errors were ignored!</h2>"));

    QDateTime endTime = QDateTime::currentDateTime();
    _p->handler->message(QtWarningMsg,
        tr("<p>Completed Update at %1</p>").arg(endTime.toString()));
    _p->handler->message(QtWarningMsg, _p->elapsedTime(startTime, endTime));
    _progress->setValue(_progress->maximum());
    returnValue = true;
  }
  else if (ignoredErrCnt > 0)
  {
    qry.exec("rollback;");
    _p->handler->message(QtWarningMsg, _rollbackMsg);
    returnValue = false;
  }
  else
  {
    qry.exec("commit;");
    _p->handler->message(QtWarningMsg, tr("<h2>The Update is now complete!</h2>"));

    QDateTime endTime = QDateTime::currentDateTime();
    _p->handler->message(QtWarningMsg,
        tr("<p>Completed Update at %1</p>").arg(endTime.toString()));
    _p->handler->message(QtWarningMsg, _p->elapsedTime(startTime, endTime));
    _progress->setValue(_progress->maximum());
    returnValue = true;
    if (_p->useCmdline)
    {
      fileExit();       // need this so the app will quit its event loop
      return returnValue;
    }
  }

  if (DEBUG)
    qDebug("LoaderWindow::sStart() progress %d out of %d after commit",
           _progress->value(), _progress->maximum());

  if (! _package->system() && schema.clearPath(errMsg) < 0)
  {
    _p->handler->message(QtWarningMsg,
        tr("<p><font color='orange'>The update completed "
                     "successfully but there was an error resetting "
                     "the schema path:</font></p><pre>%1</pre>"
                     "<p>Quit the updater and start it "
                     "again if you want to apply another update.</p>"));

  }

  return returnValue;
}

void LoaderWindow::setCmdline(bool useCmdline)
{
  _p->setCmdline(useCmdline);
}

void LoaderWindow::setDebugPkg(bool p)
{
  _alwaysrollback->setVisible(p);
  _alwaysrollback->setEnabled(p);
}

int LoaderWindow::applySql(Script *pscript, const QByteArray psql)
{
  if (DEBUG)
    qDebug("LoaderWindow::applySql() - running script %s in file %s",
           qPrintable(pscript->name()), qPrintable(pscript->filename()));

  XSqlQuery qry;
  bool again     = false;
  int  returnVal = 0;
  do {
    QString message;
    qry.exec("SAVEPOINT updaterFile;");
    if (pscript->onError() == Script::Default)
      pscript->setOnError(Script::Stop);

    ParameterList params;
    int scriptreturn = pscript->writeToDB(psql, _package->name(), params, message);
    if (scriptreturn == -1)
    {
      _p->handler->message(QtWarningMsg,
          tr("<font color='%1'>%2</font><br>")
                    .arg("orange")
                    .arg(message));
    }
    else if (scriptreturn < 0)
    {
      bool fatal = ! (pscript->onError() == Script::Ignore);
      _p->handler->message(QtWarningMsg,
          tr("<p><font color='%1'>%2</font><br>")
                    .arg(fatal ? "red" : "orange")
                    .arg(message));
      qry.exec("ROLLBACK TO updaterFile;");

      switch (pscript->onError())
      {
        case Script::Stop:
          if (DEBUG)
            qDebug("LoaderWindow::applySql() taking Script::Stop branch");
          qry.exec("rollback;");
          _p->handler->message(QtWarningMsg, _rollbackMsg);
          return scriptreturn;
          break;

        case Script::Ignore:
          if (DEBUG)
            qDebug("LoaderWindow::applySql() taking Script::Ignore branch");
          _p->handler->message(QtWarningMsg,
              tr("<font color='orange'><b>IGNORING</b> the above "
                           "errors and skipping script %1.</font><br>")
                          .arg(pscript->filename()));
          returnVal++;
          break;

        case Script::Prompt:
          if (DEBUG)
            qDebug("LoaderWindow::applySql() taking Script::Prompt branch");
        default:
          if (DEBUG)
            qDebug("LoaderWindow::applySql() taking default branch");
          switch(_p->handler->question(
                tr("<pre>%1.</pre><p>Please select the action "
                   "that you would like to take.").arg(message),
                QMessageBox::Retry|QMessageBox::Ignore|QMessageBox::Abort,
                QMessageBox::Retry))
          {
            case QMessageBox::Retry:
              _p->handler->message(QtWarningMsg, tr("RETRYING..."));
              again = true;
              break;
            case QMessageBox::Ignore:
              _p->handler->message(QtWarningMsg,
                  tr("<font color='orange'><b>IGNORING</b> the "
                               "above errors at user request and "
                               "skipping script %1.</font><br>")
                              .arg(pscript->filename()) );
              again = false;
              returnVal++;
              break;
            case QMessageBox::Abort:
            default:
              qry.exec("rollback;");
              _p->handler->message(QtWarningMsg, _rollbackMsg);
              return scriptreturn;
              break;
          }
      }
    }
    else
      _p->handler->message(QtWarningMsg,
          tr("Import of %1 was successful.").arg(pscript->filename()));
  } while (again);

  qry.exec("RELEASE SAVEPOINT updaterFile;");

  _progress->setValue(_progress->value() + 1);

  return returnVal;
}

// similar to applySql but Loadable::writeDoDB() returning -1 is a real error
int LoaderWindow::applyLoadable(Loadable *pscript, const QByteArray psql)
{
  if (DEBUG)
    qDebug("LoaderWindow::applyLoadable(%s in %s, %s)",
           qPrintable(pscript->name()), qPrintable(pscript->filename()),
           psql.data());

  XSqlQuery qry;
  bool again     = false;
  int  returnVal = 0;
  do {
    QString message;

    qry.exec("SAVEPOINT updaterFile;");
    if (pscript->onError() == Script::Default)
      pscript->setOnError(Script::Stop);

    int scriptreturn = pscript->writeToDB(psql, _package->name(), message);
    if (scriptreturn < 0)
    {
      bool fatal = ! (pscript->onError() == Script::Ignore);
      _p->handler->message(QtWarningMsg,
          tr("<br><font color='%1'>%2</font><br>")
                    .arg(fatal ? "red" : "orange")
                    .arg(message));
      qry.exec("ROLLBACK TO updaterFile;");

      switch (pscript->onError())
      {
        case Script::Stop:
          if (DEBUG)
            qDebug("LoaderWindow::applyLoadable() taking Script::Stop branch");
          qry.exec("rollback;");
          _p->handler->message(QtWarningMsg, _rollbackMsg);
          return scriptreturn;
          break;

        case Script::Ignore:
          if (DEBUG)
            qDebug("LoaderWindow::applyLoadable() taking Script::Ignore branch");
          _p->handler->message(QtWarningMsg,
              tr("<font color='orange'><b>IGNORING</b> the above "
                           "errors and skipping script %1.</font><br>")
                          .arg(pscript->filename()));
          returnVal++;
          break;

        case Script::Prompt:
          if (DEBUG)
            qDebug("LoaderWindow::applyLoadable() taking Script::Prompt branch");
        default:
          if (DEBUG)
            qDebug("LoaderWindow::applyLoadable() taking default branch");
          switch(_p->handler->question(
                tr("<pre>%1.</pre><p>Please select the action "
                   "that you would like to take.").arg(message),
                QMessageBox::Retry|QMessageBox::Ignore|QMessageBox::Abort,
                QMessageBox::Retry))
          {
            case QMessageBox::Retry:
              _p->handler->message(QtWarningMsg, tr("RETRYING..."));
              again = true;
              break;
            case QMessageBox::Ignore:
              _p->handler->message(QtWarningMsg,
                  tr("<font color='orange'><b>IGNORING</b> the "
                               "above errors at user request and "
                               "skipping script %1.</font><br>")
                              .arg(pscript->filename()) );
              again = false;
              returnVal++;
              break;
            case QMessageBox::Abort:
            default:
              qry.exec("rollback;");
              _p->handler->message(QtWarningMsg, _rollbackMsg);
              return scriptreturn;
              break;
          }
      }
    }
    else
      _p->handler->message(QtWarningMsg,
          tr("Import of %1 was successful.").arg(pscript->filename()));
  } while (again);

  qry.exec("RELEASE SAVEPOINT updaterFile;");

  _progress->setValue(_progress->value() + 1);

  return returnVal;
}

int LoaderWindowPrivate::disableTriggers()
{
  QString schema;

  QMap<QString, QList<Loadable *> > loadables;
  loadables.insert("priv",      _p->_package->_privs);
  loadables.insert("metasql",   _p->_package->_metasqls);
  loadables.insert("report",    _p->_package->_reports);
  loadables.insert("uiform",    _p->_package->_appuis);
  loadables.insert("script",    _p->_package->_appscripts);
  loadables.insert("image",     _p->_package->_images);

  if (_p->_package->_metasqls.size() > 0)
    triggers.append("public.metasql");

  foreach (QString key, loadables.keys())
  {
    foreach (Loadable *i, loadables.value(key))
    {
      schema = i->schema();
      if (schema.isEmpty() && ! _p->_package->system() && ! triggers.contains("pkg" + key))
        triggers.append("pkg" + key);
      else if (! schema.isEmpty() && "public" != schema && ! triggers.contains(schema + ".pkg" + key))
        triggers.append(schema + ".pkg" + key);
    }
  }

  foreach (Loadable *i, _p->_package->_cmds)
  {
    schema = i->schema();
    if (schema.isEmpty() && ! _p->_package->system() &&
        ! triggers.contains("pkgcmd"))
    {
      triggers.append("pkgcmd");
      triggers.append("pkgcmdarg");
    }
    else if (! schema.isEmpty() && "public" != schema &&
             ! triggers.contains(schema + ".pkgcmd"))
    {
      triggers.append(schema + ".pkgcmd");
      triggers.append(schema + ".pkgcmdarg");
    }
  }

  QRegExp beforeDot(".*\\.");
  QString empty;
  for (int i = 0; i < triggers.size(); i++)
  {
    QString triggername(triggers.at(i));
    triggername.replace(beforeDot, empty);
    XSqlQuery disableq(QString("ALTER TABLE %1 DISABLE TRIGGER %2altertrigger;")
                       .arg(triggers.at(i)) .arg(triggername));
    disableq.exec();
    if (disableq.lastError().type() != QSqlError::NoError)
    {
      handler->message(QtWarningMsg,
          _p->tr("<br><font color='red'>Could not disable %1 trigger:"
                       "<pre>%2</pre></font><br>")
                    .arg(triggers.at(i))
                    .arg(disableq.lastError().text()));
      return -1;
    }
  }

  return triggers.size();
}

int LoaderWindowPrivate::enableTriggers()
{
  QRegExp beforeDot(".*\\.");
  QString empty;
  for (int i = triggers.size() - 1; i >= 0; i--)
  {
    QString triggername(triggers.at(i));
    triggername.replace(beforeDot, empty);
    XSqlQuery enableq(QString("ALTER TABLE %1 ENABLE TRIGGER %2altertrigger;")
                       .arg(triggers.at(i)) .arg(triggername));
    enableq.exec();
    if (enableq.lastError().type() != QSqlError::NoError)
    {
      handler->message(QtWarningMsg,
          _p->tr("<br><font color='red'>Could not enable %1 trigger:"
                       "<pre>%2</pre></font><br>")
                    .arg(triggers.at(i))
                    .arg(enableq.lastError().text()));
      return -1;
    }
  }

  return triggers.size();
}

void LoaderWindow::setWindowTitle()
{
  QString name;

  XSqlQuery _q;
  _q.exec( "SELECT metric_value, CURRENT_USER AS username "
           "FROM metric "
           "WHERE (metric_name='DatabaseName')" );
  if (_q.first())
  {
    if (_q.value("metric_value").toString().isEmpty())
      name = tr("Unnamed Database");
    else
      name = _q.value("metric_value").toString();

    QString server;
    QString protocol;
    QString database;
    QString port;
    parseDatabaseURL(_databaseURL, protocol, server, database, port);

    QMainWindow::setWindowTitle( tr("%1 %2 - %3 on %4/%5 AS %6")
                               .arg(Updater::name)
                               .arg(Updater::version)
                               .arg(name)
                               .arg(server)
                               .arg(database)
                               .arg(_q.value("username").toString()) );
  }
  else
    QMainWindow::setWindowTitle(tr("%1 %2").arg(Updater::name).arg(Updater::version));
}

