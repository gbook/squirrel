/* ------------------------------------------------------------------------------
  Squirrel GUI mainwindow.cpp
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QThread>
#include <QTreeWidgetItem>

#include "mergeDialog.h"
#include "squirrelVersion.h"
#include "squirrelWorker.h"
#include "utils.h"

/* what a tree item points at. Stored in the item's data roles so the details
   pane can find the matching struct without keeping raw pointers into
   'package', which is replaced wholesale on every load */
enum treeRole {
    RoleNodeType = Qt::UserRole,
    RoleSubjectIndex,
    RoleStudyIndex,
    RoleSeriesIndex
};

enum nodeType { NodePackage, NodeSubject, NodeStudy, NodeSeries };


mainWindow::mainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::mainWindow) {
    ui->setupUi(this);

    setWindowIcon(QIcon(":/squirrel.ico"));

    ui->objectTree->setColumnWidth(0, 260);
    ui->objectTree->setColumnWidth(1, 300);
    ui->detailsTable->horizontalHeader()->setStretchLastSection(true);
    ui->detailsTable->setColumnWidth(0, 160);
    ui->mainSplitter->setStretchFactor(0, 4);
    ui->mainSplitter->setStretchFactor(1, 1);
    ui->topSplitter->setStretchFactor(0, 3);
    ui->topSplitter->setStretchFactor(1, 2);

    /* status bar: a label plus an indeterminate progress bar shown only while
       an operation is running */
    statusLabel = new QLabel(this);
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0);
    progressBar->setMaximumWidth(180);
    progressBar->setTextVisible(false);
    progressBar->setVisible(false);
    ui->statusBar->addWidget(statusLabel, 1);
    ui->statusBar->addPermanentWidget(progressBar);

    /* ----- worker thread -----
       Every squirrel library call happens over there. See squirrelWorker.h for
       why the library object must not be touched from this thread. */
    workerThread = new QThread(this);
    worker = new squirrelWorker;
    worker->moveToThread(workerThread);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);

    connect(this, &mainWindow::RequestOpen, worker, &squirrelWorker::OpenPackage);
    connect(this, &mainWindow::RequestClose, worker, &squirrelWorker::ClosePackage);
    connect(this, &mainWindow::RequestValidate, worker, &squirrelWorker::ValidatePackage);
    connect(this, &mainWindow::RequestMerge, worker, &squirrelWorker::MergePackages);

    connect(worker, &squirrelWorker::PackageLoaded, this, &mainWindow::PackageLoaded);
    connect(worker, &squirrelWorker::OperationStarted, this, &mainWindow::OperationStarted);
    connect(worker, &squirrelWorker::OperationFinished, this, &mainWindow::OperationFinished);
    connect(worker, &squirrelWorker::LogMessage, this, &mainWindow::AppendLog);

    workerThread->start();

    connect(ui->actionOpen, &QAction::triggered, this, &mainWindow::OpenPackage);
    connect(ui->actionClose, &QAction::triggered, this, &mainWindow::ClosePackage);
    connect(ui->actionExit, &QAction::triggered, this, &mainWindow::close);
    connect(ui->actionValidate, &QAction::triggered, this, &mainWindow::ValidatePackage);
    connect(ui->actionMerge, &QAction::triggered, this, &mainWindow::MergePackages);
    connect(ui->actionClearLog, &QAction::triggered, ui->logText, &QPlainTextEdit::clear);
    connect(ui->actionAbout, &QAction::triggered, this, &mainWindow::About);
    connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->objectTree, &QTreeWidget::itemSelectionChanged, this, &mainWindow::TreeSelectionChanged);

    UpdateActionStates();
    statusLabel->setText("Ready");
}


mainWindow::~mainWindow() {
    delete ui;
}


/* ------------------------------------------------------------------------------
   closeEvent - shut the worker thread down cleanly

   The worker may hold an open package with a temp directory; letting the thread
   run its event loop to completion gives squirrel's destructor a chance to clean
   that up.
   ------------------------------------------------------------------------------ */
void mainWindow::closeEvent(QCloseEvent *event) {

    if (busy) {
        QMessageBox::StandardButton answer = QMessageBox::question(this, "squirrel", "An operation is still running. Quit anyway?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }

    if (workerThread != nullptr) {
        workerThread->quit();
        workerThread->wait();
    }

    event->accept();
}


/* ------------------------------------------------------------------------------
   OpenPackage
   ------------------------------------------------------------------------------ */
void mainWindow::OpenPackage() {

    QString file = QFileDialog::getOpenFileName(this, "Open squirrel package", QString(), "squirrel packages (*.sqrl *.zip);;All files (*)");
    if (file.isEmpty())
        return;

    ui->logText->appendPlainText(QString("----- Opening %1 -----").arg(file));
    emit RequestOpen(file);
}


/* ------------------------------------------------------------------------------
   ClosePackage
   ------------------------------------------------------------------------------ */
void mainWindow::ClosePackage() {
    emit RequestClose();
}


/* ------------------------------------------------------------------------------
   ValidatePackage
   ------------------------------------------------------------------------------ */
void mainWindow::ValidatePackage() {
    ui->logText->appendPlainText("----- Validating -----");
    emit RequestValidate();
}


/* ------------------------------------------------------------------------------
   MergePackages - runs modify::MergePackages(), the same call behind
   'squirrel merge'
   ------------------------------------------------------------------------------ */
void mainWindow::MergePackages() {

    mergeDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    ui->logText->appendPlainText(QString("----- %1 -----").arg(dialog.TestOnly() ? "Merge (test only)" : "Merge"));
    emit RequestMerge(dialog.InputPaths(), dialog.OutputPath(), dialog.TestOnly(), dialog.RenumberSubjects(), dialog.Digits());
}


/* ------------------------------------------------------------------------------
   About
   ------------------------------------------------------------------------------ */
void mainWindow::About() {

    QMessageBox::about(this, "About squirrel",
        QString("<b>squirrel</b><br><br>"
                "Utilities build %1.%2.%3<br>"
                "squirrel library %4.%5<br><br>"
                "Copyright (C) 2004 - 2026 Gregory A Book<br>"
                "Olin Neuropsychiatry Research Center, Hartford Hospital<br><br>"
                "Released under the GPLv3 license.")
            .arg(UTIL_VERSION_MAJ).arg(UTIL_VERSION_MIN).arg(UTIL_BUILD_NUM)
            .arg(SQUIRREL_VERSION_MAJ).arg(SQUIRREL_VERSION_MIN));
}


/* ------------------------------------------------------------------------------
   PackageLoaded - a package finished loading (or was closed)
   ------------------------------------------------------------------------------ */
void mainWindow::PackageLoaded(guiPackage pkg) {

    package = pkg;
    PopulateTree(package);
    UpdateActionStates();

    if (package.isOpen)
        setWindowTitle(QString("%1 - squirrel").arg(QFileInfo(package.path).fileName()));
    else
        setWindowTitle("squirrel");
}


/* ------------------------------------------------------------------------------
   OperationStarted
   ------------------------------------------------------------------------------ */
void mainWindow::OperationStarted(QString description) {
    SetBusy(true, description);
}


/* ------------------------------------------------------------------------------
   OperationFinished
   ------------------------------------------------------------------------------ */
void mainWindow::OperationFinished(bool success, QString message) {

    SetBusy(false);

    if (!message.isEmpty()) {
        statusLabel->setText(message);
        ui->logText->appendPlainText(message);
    }

    if (!success && !message.isEmpty())
        QMessageBox::warning(this, "squirrel", message);
}


/* ------------------------------------------------------------------------------
   AppendLog
   ------------------------------------------------------------------------------ */
void mainWindow::AppendLog(QString message) {
    ui->logText->appendPlainText(message);
}


/* ------------------------------------------------------------------------------
   PopulateTree
   ------------------------------------------------------------------------------ */
void mainWindow::PopulateTree(const guiPackage &pkg) {

    ui->objectTree->clear();
    ui->detailsTable->setRowCount(0);

    if (!pkg.isOpen)
        return;

    QTreeWidgetItem *packageItem = new QTreeWidgetItem(ui->objectTree);
    packageItem->setText(0, pkg.name.isEmpty() ? QFileInfo(pkg.path).fileName() : pkg.name);
    packageItem->setText(1, pkg.description);
    packageItem->setText(2, QString("%1 subject(s), %2").arg(pkg.subjects.size()).arg(utils::HumanReadableSize(pkg.unzipSize)));
    packageItem->setData(0, RoleNodeType, NodePackage);

    for (int s = 0; s < pkg.subjects.size(); s++) {
        const guiSubject &subject = pkg.subjects.at(s);

        QTreeWidgetItem *subjectItem = new QTreeWidgetItem(packageItem);
        subjectItem->setText(0, subject.id);
        subjectItem->setText(1, subject.enrollmentGroup);
        subjectItem->setText(2, QString("%1 study(s)").arg(subject.studies.size()));
        subjectItem->setData(0, RoleNodeType, NodeSubject);
        subjectItem->setData(0, RoleSubjectIndex, s);

        for (int t = 0; t < subject.studies.size(); t++) {
            const guiStudy &study = subject.studies.at(t);

            QTreeWidgetItem *studyItem = new QTreeWidgetItem(subjectItem);
            studyItem->setText(0, QString("Study %1").arg(study.number));
            studyItem->setText(1, study.description);
            studyItem->setText(2, QString("%1, %2 series").arg(study.modality).arg(study.series.size()));
            studyItem->setData(0, RoleNodeType, NodeStudy);
            studyItem->setData(0, RoleSubjectIndex, s);
            studyItem->setData(0, RoleStudyIndex, t);

            for (int r = 0; r < study.series.size(); r++) {
                const guiSeries &series = study.series.at(r);

                QTreeWidgetItem *seriesItem = new QTreeWidgetItem(studyItem);
                seriesItem->setText(0, QString("Series %1").arg(series.number));
                seriesItem->setText(1, series.description.isEmpty() ? series.protocol : series.description);
                seriesItem->setText(2, QString("%1 file(s), %2").arg(series.fileCount).arg(utils::HumanReadableSize(series.size)));
                seriesItem->setData(0, RoleNodeType, NodeSeries);
                seriesItem->setData(0, RoleSubjectIndex, s);
                seriesItem->setData(0, RoleStudyIndex, t);
                seriesItem->setData(0, RoleSeriesIndex, r);
            }
        }
    }

    /* expand only the package node - a large package has thousands of series
       and expanding everything makes the tree unusable */
    packageItem->setExpanded(true);
    ui->objectTree->setCurrentItem(packageItem);
}


/* ------------------------------------------------------------------------------
   TreeSelectionChanged - show the selected object in the details pane
   ------------------------------------------------------------------------------ */
void mainWindow::TreeSelectionChanged() {

    QTreeWidgetItem *item = ui->objectTree->currentItem();
    if (item == nullptr) {
        ui->detailsTable->setRowCount(0);
        return;
    }

    int type = item->data(0, RoleNodeType).toInt();
    int s = item->data(0, RoleSubjectIndex).toInt();
    int t = item->data(0, RoleStudyIndex).toInt();
    int r = item->data(0, RoleSeriesIndex).toInt();

    /* guard every index: the tree is rebuilt asynchronously when a new package
       loads, and a stale selection must not index into the new snapshot */
    switch (type) {
        case NodePackage:
            ShowDetails(package.details());
            break;

        case NodeSubject:
            if ((s >= 0) && (s < package.subjects.size()))
                ShowDetails(package.subjects.at(s).details());
            break;

        case NodeStudy:
            if ((s >= 0) && (s < package.subjects.size()) && (t >= 0) && (t < package.subjects.at(s).studies.size()))
                ShowDetails(package.subjects.at(s).studies.at(t).details());
            break;

        case NodeSeries:
            if ((s >= 0) && (s < package.subjects.size()) && (t >= 0) && (t < package.subjects.at(s).studies.size())) {
                const guiStudy &study = package.subjects.at(s).studies.at(t);
                if ((r >= 0) && (r < study.series.size()))
                    ShowDetails(study.series.at(r).details());
            }
            break;

        default:
            ui->detailsTable->setRowCount(0);
            break;
    }
}


/* ------------------------------------------------------------------------------
   ShowDetails
   ------------------------------------------------------------------------------ */
void mainWindow::ShowDetails(const QVector<guiDetail> &details) {

    ui->detailsTable->setRowCount(details.size());

    for (int i = 0; i < details.size(); i++) {
        QTableWidgetItem *name = new QTableWidgetItem(details.at(i).name);
        QTableWidgetItem *value = new QTableWidgetItem(details.at(i).value);
        name->setFlags(name->flags() & ~Qt::ItemIsEditable);
        value->setFlags(value->flags() & ~Qt::ItemIsEditable);
        ui->detailsTable->setItem(i, 0, name);
        ui->detailsTable->setItem(i, 1, value);
    }
}


/* ------------------------------------------------------------------------------
   SetBusy
   ------------------------------------------------------------------------------ */
void mainWindow::SetBusy(bool b, const QString &description) {

    busy = b;
    progressBar->setVisible(b);

    if (b && !description.isEmpty())
        statusLabel->setText(description);

    UpdateActionStates();
}


/* ------------------------------------------------------------------------------
   UpdateActionStates

   The worker runs one operation at a time, so actions are disabled while it is
   working rather than allowing requests to pile up in its event queue.
   ------------------------------------------------------------------------------ */
void mainWindow::UpdateActionStates() {

    ui->actionOpen->setEnabled(!busy);
    ui->actionMerge->setEnabled(!busy);
    ui->actionClose->setEnabled(!busy && package.isOpen);
    ui->actionValidate->setEnabled(!busy && package.isOpen);
}
