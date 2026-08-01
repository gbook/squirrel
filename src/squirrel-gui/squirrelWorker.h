/* ------------------------------------------------------------------------------
  Squirrel GUI squirrelWorker.h
  Copyright (C) 2004 - 2026
  Gregory A Book <gregory.book@hhchealth.org> <gregory.a.book@gmail.com>
  Olin Neuropsychiatry Research Center, Hartford Hospital
  ------------------------------------------------------------------------------
  GPLv3 License:

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  ------------------------------------------------------------------------------ */

#ifndef SQUIRRELWORKER_H
#define SQUIRRELWORKER_H

#include <QObject>
#include <QStringList>
#include "squirrelModel.h"

class squirrel;

/* ------------------------------------------------------------------------------
   squirrelWorker

   Wraps the squirrel library so no library call ever runs on the GUI thread.
   Reading a package unpacks and parses an archive that can be many gigabytes;
   done inline it would freeze the window for minutes.

   Ownership rules, which the rest of the GUI depends on:
     - the squirrel instance is created, used and destroyed ONLY in the thread
       this object was moved to. squirrel holds a QSqlDatabase, and Qt forbids
       using a database connection from a thread other than the one that made it.
     - every slot here is invoked through a queued connection from mainwindow.
     - results cross back as guiPackage snapshots (see squirrelModel.h), never as
       pointers into the library's objects.

   All slots are serialized by the worker thread's event loop, so only one
   operation runs at a time. mainwindow disables the relevant actions while an
   operation is in flight rather than queueing several up.
   ------------------------------------------------------------------------------ */
class squirrelWorker : public QObject
{
    Q_OBJECT

public:
    explicit squirrelWorker(QObject *parent = nullptr);
    ~squirrelWorker();

public slots:
    void OpenPackage(QString packagePath);
    void ClosePackage();
    void ValidatePackage();
    void MergePackages(QStringList inputPaths, QString outputPath, bool testOnly, bool renumberSubjects, int digits);

signals:
    /* an operation started/finished; mainwindow uses these to drive the busy state */
    void OperationStarted(QString description);
    void OperationFinished(bool success, QString message);

    /* a package finished loading. pkg.isOpen is false if nothing is open */
    void PackageLoaded(guiPackage pkg);

    /* text destined for the log pane */
    void LogMessage(QString message);

private:
    /* walks the open package and fills in a snapshot for the GUI thread */
    guiPackage BuildSnapshot();

    /* drains the library's internal log buffer into LogMessage() */
    void DrainLog();

    squirrel *sqrl = nullptr;
    QString currentPath;
};

#endif // SQUIRRELWORKER_H
