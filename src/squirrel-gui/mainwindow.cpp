/* ------------------------------------------------------------------------------
  squirrel-gui mainwindow.cpp
  Copyright (C) 2004 - 2023
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
#include "packagedialog.h"
#include "subjectDialog.h"
#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->packageTree->setStyleSheet("QHeaderView::section { background-color:#444; color: #fff}");
    ui->experimentsTable->setStyleSheet("QHeaderView::section { background-color:#444; color: #fff}");

    /* create an empty squirrel object */
    sqrl = new squirrel();
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
    delete ui;
}


/* ------------------------------------------------------------ */
/* ----- on_btnAddSubject_clicked ----------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief MainWindow::on_btnAddSubject_clicked
 */
void MainWindow::on_btnAddSubject_clicked()
{
    subjectDialog *subjectInfo = new subjectDialog();
    if (subjectInfo->exec()) {
    }

    /* add the subject to the squirrel object */
    squirrelSubject sqrlSubject;
    sqrlSubject.ID = "S1234ABC";

    sqrl->addSubject(sqrlSubject);

    /* add a subject to the data node of the tree */
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, "S1234ABC");
    item->setData(0, Qt::EditRole, "subject");
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    ui->packageTree->addTopLevelItem(item);

    delete item;
}


/* ------------------------------------------------------------ */
/* ----- on_packageTree_itemClicked --------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief MainWindow::on_packageTree_itemClicked
 * @param item
 * @param column
 */
void MainWindow::on_packageTree_itemClicked(QTreeWidgetItem *item, int column)
{
    RefreshTopInfoTable();
}


/* ------------------------------------------------------------ */
/* ----- on_packageTree_itemDoubleClicked --------------------- */
/* ------------------------------------------------------------ */
void MainWindow::on_packageTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    /* display the variable details in a new dialog box so they can be edited */

}


/* ------------------------------------------------------------ */
/* ----- on_actionE_xit_triggered ----------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::on_actionE_xit_triggered()
{
    /* check if unsaved work */

    /* then exit */
    exit(0);
}


/* ------------------------------------------------------------ */
/* ----- on_btnAddStudy_clicked ------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::on_btnAddStudy_clicked()
{
    /* get selected subject */
    if (ui->packageTree->selectedItems().size() == 1) {
        QTreeWidgetItem *item = ui->packageTree->selectedItems()[0];
        if (item->text(0) == "subject") {
            /* add a subject to the data node of the tree */
            QTreeWidgetItem *newItem = new QTreeWidgetItem();
            newItem->setText(1, "StudyN");
            newItem->setData(0, Qt::EditRole, "study");

            item->addChild(newItem);
            item->setExpanded(true);
        }
    }
}


/* ------------------------------------------------------------ */
/* ----- on_btnAddSeries_clicked ------------------------------ */
/* ------------------------------------------------------------ */
void MainWindow::on_btnAddSeries_clicked()
{
    /* get selected study */
    if (ui->packageTree->selectedItems().size() == 1) {
        QTreeWidgetItem *item = ui->packageTree->selectedItems()[0];
        if (item->text(0) == "study") {
            /* add a subject to the data node of the tree */
            QTreeWidgetItem *newItem = new QTreeWidgetItem();
            newItem->setText(0, "seriesID");
            newItem->setText(1, "series");

            item->addChild(newItem);
            item->setExpanded(true);
        }
    }
}


/* ------------------------------------------------------------ */
/* ----- on_packageTree_itemSelectionChanged ------------------ */
/* ------------------------------------------------------------ */
void MainWindow::on_packageTree_itemSelectionChanged()
{
    RefreshTopInfoTable();
}


/* ------------------------------------------------------------ */
/* ----- on_action_New_package_triggered ---------------------- */
/* ------------------------------------------------------------ */
void MainWindow::on_action_New_package_triggered()
{
    NewPackage();
}


/* ------------------------------------------------------------ */
/* ----- NewPackage ------------------------------------------- */
/* ------------------------------------------------------------ */
bool MainWindow::NewPackage() {
    packageDialog *packageInfo = new packageDialog();
    if (packageInfo->exec()) {
        QString pkgName;
        QString pkgDesc;
        QDateTime pkgDate;
        QString pkgDirFormat;
        QString pkgDataFormat;

        packageInfo->GetValues(pkgName, pkgDesc, pkgDate, pkgDirFormat, pkgDataFormat);
        sqrl->name = pkgName;
        sqrl->description = pkgDesc;
        sqrl->datetime = pkgDate;
        sqrl->subjectDirFormat = pkgDirFormat;
        sqrl->studyDirFormat = pkgDirFormat;
        sqrl->seriesDirFormat = pkgDirFormat;
        sqrl->dataFormat = pkgDataFormat;

        RefreshPackageDetails();
        return true;
    }
    return false;
}


/* ------------------------------------------------------------ */
/* ----- EditPackageDetails ----------------------------------- */
/* ------------------------------------------------------------ */
bool MainWindow::EditPackageDetails() {
    packageDialog *packageInfo = new packageDialog();

    packageInfo->SetValues(sqrl->name, sqrl->description, sqrl->datetime, sqrl->subjectDirFormat, sqrl->dataFormat);

    if (packageInfo->exec()) {
        QString pkgName;
        QString pkgDesc;
        QDateTime pkgDate;
        QString pkgDirFormat;
        QString pkgDataFormat;

        packageInfo->GetValues(pkgName, pkgDesc, pkgDate, pkgDirFormat, pkgDataFormat);
        sqrl->name = pkgName;
        sqrl->description = pkgDesc;
        sqrl->datetime = pkgDate;
        sqrl->subjectDirFormat = pkgDirFormat;
        sqrl->studyDirFormat = pkgDirFormat;
        sqrl->seriesDirFormat = pkgDirFormat;
        sqrl->dataFormat = pkgDataFormat;

        RefreshPackageDetails();
        return true;
    }
    return false;
}


/* ------------------------------------------------------------ */
/* ----- RefreshPackageDetails -------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::RefreshPackageDetails() {
    ui->lblPackagePath->setText(sqrl->filePath);
    ui->lblPackageName->setText(sqrl->name);
    ui->lblPackageDesc->setText(sqrl->description);
    ui->lblPackageDate->setText(sqrl->datetime.toString());
    ui->lblPackageDirFormat->setText(sqrl->subjectDirFormat);
    ui->lblPackageDataFormat->setText(sqrl->dataFormat);
	ui->lblTempPath->setText(sqrl->GetTempDir());
}


/* ------------------------------------------------------------ */
/* ----- EnableDisableSubjectButtons -------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::EnableDisableSubjectButtons() {
    /* get selected study */
    if (ui->packageTree->selectedItems().size() == 1) {
        QTreeWidgetItem *item = ui->packageTree->selectedItems()[0];
        QString dataCategory = item->data(0, Qt::EditRole).toString();

        /* start by disabling all buttons */
        //ui->btnAddSubject->setDisabled(true);
        ui->btnAddStudy->setDisabled(true);
        ui->btnAddSeries->setDisabled(true);
        ui->btnAddDrug->setDisabled(true);
        ui->btnAddMeasure->setDisabled(true);
        ui->btnAddAnalysis->setDisabled(true);

        if (dataCategory == "subject") {
            /* enable AddStudy, AddDrug, AddMeasure buttons */
            ui->btnAddStudy->setEnabled(true);
            ui->btnAddDrug->setEnabled(true);
            ui->btnAddMeasure->setEnabled(true);
        }
        else if (dataCategory == "study") {
            /* enable the AddSeries, AddAnalysis buttons */
            ui->btnAddSeries->setEnabled(true);
        }
        else if (dataCategory == "series") {
            /* enable series related buttons */

        }
    }
}


/* ------------------------------------------------------------ */
/* ----- on_actionOpen_triggered ------------------------------ */
/* ------------------------------------------------------------ */
void MainWindow::on_actionOpen_triggered()
{
    /* open a squirrel file */
    QString filename;
    filename = QFileDialog::getOpenFileName(this, tr("Open squirrel package"), "/", tr("Squirrel packages (*.zip)"));
    sqrl->filePath = filename;
	if (sqrl->read(sqrl->filePath)) {
		qDebug() << "Successfuly read squirrel file";
	}
	else {
		qDebug() << "Unable to read squirrel file";
	}
    RefreshPackageDetails();
}


/* ------------------------------------------------------------ */
/* ----- on_btnNewPackage_clicked ----------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::on_btnNewPackage_clicked()
{
    NewPackage();
}


/* ------------------------------------------------------------ */
/* ----- on_btnEditPackageDetails_clicked --------------------- */
/* ------------------------------------------------------------ */
void MainWindow::on_btnEditPackageDetails_clicked()
{
    EditPackageDetails();
}


/* ------------------------------------------------------------ */
/* ----- DisplaySubjectDetails -------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::DisplaySubjectDetails(QString ID) {

    /* find subject */
    squirrelSubject sqrlSubject;
    if (sqrl->GetSubject(ID, sqrlSubject)) {

        /* load subject details */
        QStringList variables, values;
        variables.append("ID"); values.append(sqrlSubject.ID);
        variables.append("alternateIDs"); values.append(sqrlSubject.alternateIDs.join(","));
        variables.append("GUID"); values.append(sqrlSubject.GUID);
        variables.append("dateOfBirth"); values.append(sqrlSubject.dateOfBirth.toString());
        variables.append("sex"); values.append(sqrlSubject.sex);
        variables.append("gender"); values.append(sqrlSubject.gender);
        variables.append("ethnicity1"); values.append(sqrlSubject.ethnicity1);
        variables.append("ethnicity2"); values.append(sqrlSubject.ethnicity2);

        /* clear table */
        ui->subjectDetailsTable->setRowCount(0);

        for (int i=0; i<variables.size(); i++) {
            int currentRow = ui->subjectDetailsTable->rowCount();
            ui->subjectDetailsTable->setRowCount(currentRow + 1);
            ui->subjectDetailsTable->setItem(currentRow, 0, new QTableWidgetItem(variables[i]));
            ui->subjectDetailsTable->setItem(currentRow, 1, new QTableWidgetItem(values[i]));
        }
    }
    else {
        /* subject not found */
    }
}

void MainWindow::on_btnAddAnalysis_clicked()
{

}


void MainWindow::on_btnAddDrug_clicked()
{

}


void MainWindow::on_btnAddMeasure_clicked()
{

}


void MainWindow::on_btnAddExperiment_clicked()
{

}


void MainWindow::on_btnAddPipeline_clicked()
{

}


void MainWindow::on_packageTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    RefreshTopInfoTable();
}

void MainWindow::RefreshTopInfoTable() {
    EnableDisableSubjectButtons();

    /* get selected study */
    if (ui->packageTree->selectedItems().size() == 1) {
        QTreeWidgetItem *item = ui->packageTree->selectedItems()[0];
        QString dataCategory = item->text(1);
        QString ID = item->text(0);

        qDebug() << "data [" << dataCategory << "]  subjectID [" << ID << "]";

        if (dataCategory == "subject") {
            /* display the subject table */
            DisplaySubjectDetails(ID);
        }
        else if (dataCategory == "study") {
            /* display the study table */

        }
        else if (dataCategory == "series") {
            /* display the series table */

        }
    }
}
