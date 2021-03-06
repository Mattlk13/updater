/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2019 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef LOADERWINDOW_H
#define LOADERWINDOW_H

class Loadable;
class Package;
class Script;
class TarFile;

#include <QMainWindow>

#include "ui_loaderwindow.h"

class LoaderWindowPrivate;
class XAbstractMessageHandler;

class LoaderWindow : public QMainWindow, public Ui::LoaderWindow
{
    Q_OBJECT

public:
    LoaderWindow(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~LoaderWindow();

    virtual XAbstractMessageHandler *handler() const;

public slots:
    virtual void fileNew();
    virtual void fileOpen();
    virtual void fileExit();
    virtual void helpContents();
    virtual void helpAbout();

    virtual void setCmdline(bool);
    virtual void setDebugPkg(bool);
    virtual bool openFile(QString filename);
    virtual void setWindowTitle();
    virtual bool sStart();

protected:
    Package * _package;
    TarFile * _files;

    QString _filename;
    QString prePkgVer;
    QString preDbVer;

    virtual int  applySql(Script *, const QByteArray);
    virtual int  applyLoadable(Loadable *, const QByteArray);
    virtual void launchBrowser(QWidget *w, const QString &url);
    virtual void timerEvent( QTimerEvent * e );
    virtual void logUpdate(QDateTime startTime, QDateTime endTime);

    static QString _rollbackMsg;

protected slots:
    virtual void languageChange();

private:
    LoaderWindowPrivate *_p;

    friend class LoaderWindowPrivate;

};

#endif // LOADERWINDOW_H
