/* ------------------------------------------------------------------------------
  Squirrel GUI mainwindow.h
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include "squirrelModel.h"

class QLabel;
class QProgressBar;
class QThread;
class QTreeWidgetItem;
class squirrelWorker;

namespace Ui { class mainWindow; }

class mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainWindow(QWidget *parent = nullptr);
    ~mainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    /* queued to squirrelWorker on the worker thread */
    void RequestOpen(QString packagePath);
    void RequestClose();
    void RequestValidate();
    void RequestMerge(QStringList inputPaths, QString outputPath, bool testOnly, bool renumberSubjects, int digits);

private slots:
    /* menu/toolbar actions */
    void OpenPackage();
    void ClosePackage();
    void ValidatePackage();
    void MergePackages();
    void About();

    /* worker responses */
    void PackageLoaded(guiPackage pkg);
    void OperationStarted(QString description);
    void OperationFinished(bool success, QString message);
    void AppendLog(QString message);

    /* tree -> details pane */
    void TreeSelectionChanged();

private:
    void PopulateTree(const guiPackage &pkg);
    void ShowDetails(const QVector<guiDetail> &details);
    void SetBusy(bool busy, const QString &description = QString());
    void UpdateActionStates();

    Ui::mainWindow *ui;

    QThread *workerThread = nullptr;
    squirrelWorker *worker = nullptr;

    QLabel *statusLabel = nullptr;
    QProgressBar *progressBar = nullptr;

    /* the GUI thread's copy of the open package. The worker keeps the real
       squirrel object; this is display data only */
    guiPackage package;

    bool busy = false;
};

#endif // MAINWINDOW_H
