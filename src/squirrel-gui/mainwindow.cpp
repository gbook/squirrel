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
#include "objectDialog.h"
#include "aboutDialog.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <dicom.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->subjectTree->setStyleSheet("QHeaderView::section { background-color:#444; color: #fff}");
    ui->experimentsTable->setStyleSheet("QHeaderView::section { background-color:#444; color: #fff}");
    ui->pipelinesTable->setStyleSheet("QHeaderView::section { background-color:#444; color: #fff}");
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
/* top level menu options                                       */
/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */

void MainWindow::on_actionE_xit_triggered() {
    /* check if unsaved work */
    if (ClosePackage())
        exit(0);
}

void MainWindow::on_action_New_package_triggered() {
    NewPackage();
}

void MainWindow::on_actionOpen_triggered() {
    OpenPackage();
}

void MainWindow::on_action_Save_package_triggered() {

}

void MainWindow::on_action_Help_triggered() {

}

void MainWindow::on_action_About_triggered() {
    About *about = new About();
    about->exec();
}

void MainWindow::on_actionValidate_triggered() {
    OpenPackage();
}

void MainWindow::on_actionClose_triggered() {
    ClosePackage();
}

/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
/* GUI events                                                   */
/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */

void MainWindow::on_btnAddSubject_clicked() {
    AddSubject();
}

void MainWindow::on_btnAddStudy_clicked() {
    AddStudy();
}

void MainWindow::on_btnAddSeries_clicked() {
    AddSeries();
}

void MainWindow::on_btnNewPackage_clicked() {
    NewPackage();
}

void MainWindow::on_btnEditPackageDetails_clicked() {
    EditPackageDetails();
}

void MainWindow::on_btnAddAnalysis_clicked() {
    AddAnalysis();
}

void MainWindow::on_btnAddDrug_clicked() {
    AddDrug();
}

void MainWindow::on_btnAddMeasure_clicked() {
    AddMeasure();
}

void MainWindow::on_btnAddExperiment_clicked() {
    AddExperiment();
}

void MainWindow::on_btnAddPipeline_clicked() {
    AddPipeline();
}

void MainWindow::on_btnAddDICOM_clicked() {
    AddDICOM();
}

void MainWindow::on_btnOpenPackage_clicked() {
    OpenPackage();
}

void MainWindow::on_btnClosePackage_clicked() {
    ClosePackage();
}

void MainWindow::on_subjectTree_itemClicked(QTreeWidgetItem *item, int column) {
    RefreshSubjectInfoTable();
}

void MainWindow::on_subjectTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    RefreshSubjectInfoTable();
}

void MainWindow::on_subjectTree_itemChanged(QTreeWidgetItem *item, int column) {
    RefreshSubjectInfoTable();
}


/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
/* Functions                                                    */
/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/* ----- RefreshTopInfoTable ---------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::RefreshSubjectInfoTable() {
    EnableDisableSubjectButtons();

    /* get selected study */
    if (ui->subjectTree->selectedItems().size() == 1) {
        //ui->txtOutput->appendPlainText("Clicked 1 item on subject tree");

        QTreeWidgetItem *item = ui->subjectTree->selectedItems()[0];
        QString dataCategory = item->data(0, Qt::UserRole).toString();

        qDebug() << "data [" << dataCategory << "]";

        if (dataCategory == "subject") {
            /* display the subject table */
            QString subjectID = item->text(0);
            DisplaySubjectDetails(subjectID);
        }
        else if (dataCategory == "study") {
            /* display the study table */
            QString subjectID = item->parent()->text(0);
            int studyNum = item->text(0).toInt();
            DisplayStudyDetails(subjectID, studyNum);
        }
        else if (dataCategory == "series") {
            /* display the series table */
            QString subjectID = item->parent()->parent()->text(0);
            int studyNum = item->parent()->text(0).toInt();
            int seriesNum = item->parent()->text(0).toInt();
            DisplaySeriesDetails(subjectID, studyNum, seriesNum);
        }
    }
}


/* ------------------------------------------------------------ */
/* ----- OpenPackage ------------------------------------------ */
/* ------------------------------------------------------------ */
void MainWindow::OpenPackage()
{
    ui->lblStatus->setText("Opening squirrel package...");
    QApplication::setOverrideCursor(Qt::WaitCursor);

    /* create an empty squirrel object */
	sqrl = new squirrel();
    sqrlValid = true;

	/* open a squirrel file */
	QString filename;
    //QString m;
	filename = QFileDialog::getOpenFileName(this, tr("Open squirrel package"), "/", tr("Squirrel packages (*.zip)"));
	sqrl->filePath = filename;
    bool success = sqrl->read(filename);
    ui->txtOutput->appendPlainText(sqrl->GetLog());
	if (success) {
		ui->txtOutput->appendPlainText("Successfully read squirrel file");
        ui->txtOutput->appendHtml("<span style='color: green'><b>Successfully</b> read squirrel file</span>");
	}
	else {
		ui->txtOutput->appendPlainText("Unable to read squirrel file");
	}

	RefreshPackageDetails();
    RefreshSubjectsTable();
    RefreshExperimentsTable();
    RefreshPipelinesTable();

    ui->lblStatus->setText("Finished");
    QApplication::restoreOverrideCursor();

}


/* ------------------------------------------------------------ */
/* ----- NewPackage ------------------------------------------- */
/* ------------------------------------------------------------ */
bool MainWindow::NewPackage() {
    ClosePackage();

	/* create an empty squirrel object */
	sqrl = new squirrel();
    sqrlValid = true;

    packageDialog *packageInfo = new packageDialog();
    if (packageInfo->exec()) {

		packageInfo->SetEditType(true);

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

		packageInfo->SetEditType(false);

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
/* ----- RefreshSubjectTable ---------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::RefreshSubjectsTable() {

    /* clear all existing rows in the treeControl */
    ui->subjectTree->clear();

    QList<squirrelSubject> subjects;
    sqrl->GetSubjectList(subjects);

    ui->tabWidget->setTabText(1, QString("Subjects (%1)").arg(subjects.size()));

    /* iterate through all subjects */
    for (int i=0; i < subjects.size(); i++) {

        squirrelSubject sub = subjects[i];

        qDebug().noquote() << QString("Working on subject %1").arg(sub.ID);

        /* add subject to the tree */
        QTreeWidgetItem *subjectItem = new QTreeWidgetItem();
        subjectItem->setData(0, Qt::UserRole, "subject");
        subjectItem->setText(0, sub.ID);
        subjectItem->setText(1, "subject");
        subjectItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->subjectTree->addTopLevelItem(subjectItem);

        /* iterate through studies */
        for (int j=0; j < sub.studyList.size(); j++) {
            squirrelStudy stud = sub.studyList[j];

            qDebug().noquote() << QString("Working on study %1").arg(stud.dateTime.toString());

            /* add a subject to the data node of the tree */
            QTreeWidgetItem *studyItem = new QTreeWidgetItem();
            studyItem->setData(0, Qt::UserRole, "study");
            studyItem->setText(0, QString("%1").arg(stud.number));
            studyItem->setText(1, "study");

            subjectItem->addChild(studyItem);

            /* iterate through series */
            for (int k=0; k < stud.seriesList.size(); k++) {
                squirrelSeries ser = stud.seriesList[k];

                qDebug().noquote() << QString("Working on series %1").arg(ser.number);

                /* add a subject to the data node of the tree */
                QTreeWidgetItem *seriesItem = new QTreeWidgetItem();
                seriesItem->setData(0, Qt::UserRole, "series");
                seriesItem->setText(0, QString("%1").arg(ser.number));
                seriesItem->setText(1, "series");

                studyItem->addChild(seriesItem);
            }
        }

        /* iterate through the drugs */
        for (int j=0; j < sub.drugList.size(); j++) {
            squirrelDrug drug = sub.drugList[j];

            /* add a subject to the data node of the tree */
            QTreeWidgetItem *drugItem = new QTreeWidgetItem();
            drugItem->setData(0, Qt::UserRole, "drug");
            drugItem->setText(0, QString("%1").arg(drug.drugName));
            drugItem->setText(1, "drug");

            subjectItem->addChild(drugItem);
        }

        /* iterate through the measures */
    }
}


/* ------------------------------------------------------------ */
/* ----- RefreshExperimentsTable ------------------------------ */
/* ------------------------------------------------------------ */
void MainWindow::RefreshExperimentsTable() {

    /* clear all existing rows in the treeControl (except for the header) */
    for (int i=1; i<ui->experimentsTable->rowCount(); i++) {
        ui->experimentsTable->removeRow(i);
    }

    QList<squirrelExperiment> experiments;
    experiments = sqrl->experimentList;

    ui->tabWidget->setTabText(2, QString("Experiments (%1)").arg(experiments.size()));

    if (experiments.size() > 0) {
        /* iterate through all experiments */
        for (int i=0; i < experiments.size(); i++) {

            squirrelExperiment exp = experiments[i];

            qDebug().noquote() << QString("Working on experiment %1").arg(exp.experimentName);

            /* add experiment to the table */
            int currentRow = ui->experimentsTable->rowCount();
            ui->experimentsTable->setRowCount(currentRow + 1);
            ui->experimentsTable->setItem(currentRow, 0, new QTableWidgetItem(exp.experimentName));
            ui->experimentsTable->setItem(currentRow, 1, new QTableWidgetItem(exp.numFiles));
        }
    }
}


/* ------------------------------------------------------------ */
/* ----- RefreshPipelinesTable -------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::RefreshPipelinesTable() {

    /* clear all existing rows in the treeControl (except for the header) */
    for (int i=1; i<ui->pipelinesTable->rowCount(); i++) {
        ui->pipelinesTable->removeRow(i);
    }

    QList<squirrelPipeline> pipelines;
    pipelines = sqrl->pipelineList;

    if (pipelines.size() > 0) {
        ui->tabWidget->setTabText(3, QString("Pipelines (%1)").arg(pipelines.size()));

        /* iterate through all pipelines */
        for (int i=0; i < pipelines.size(); i++) {

            squirrelPipeline pip = pipelines[i];

            qDebug().noquote() << QString("Working on pipeline %1").arg(pip.pipelineName);

            /* add pipeline to the table */
            int currentRow = ui->pipelinesTable->rowCount();
            ui->pipelinesTable->setRowCount(currentRow + 1);
            ui->pipelinesTable->setItem(currentRow, 0, new QTableWidgetItem(pip.pipelineName));
            ui->pipelinesTable->setItem(currentRow, 1, new QTableWidgetItem(pip.createDate.toString()));
        }
    }
}


/* ------------------------------------------------------------ */
/* ----- EnableDisableSubjectButtons -------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::EnableDisableSubjectButtons() {
    /* get selected study */
    if (ui->subjectTree->selectedItems().size() == 1) {
        QTreeWidgetItem *item = ui->subjectTree->selectedItems()[0];
        QString dataCategory = item->data(0, Qt::UserRole).toString();

        /* start by disabling all buttons */
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


/* ------------------------------------------------------------ */
/* ----- DisplayStudyDetails ---------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::DisplayStudyDetails(QString subjectID, int studyNum) {

    /* find subject */
    squirrelStudy sqrlStudy;
    if (sqrl->GetStudy(subjectID, studyNum, sqrlStudy)) {

        /* load subject details */
        QStringList variables, values;
        variables.append("number"); values.append(QString("%1").arg(sqrlStudy.number));
        variables.append("dateTime"); values.append(sqrlStudy.dateTime.toString("yyyy-MM-dd hh:mm:ss"));
        variables.append("ageAtStudy"); values.append(QString("%1").arg(sqrlStudy.ageAtStudy));
        variables.append("height"); values.append(QString("%1").arg(sqrlStudy.height));
        variables.append("weight"); values.append(QString("%1").arg(sqrlStudy.weight));
        variables.append("modality"); values.append(sqrlStudy.modality);
        variables.append("description"); values.append(sqrlStudy.description);
        variables.append("studyUID"); values.append(sqrlStudy.studyUID);
        variables.append("visitType"); values.append(sqrlStudy.visitType);
        variables.append("dayNumber"); values.append(sqrlStudy.dayNumber);
        variables.append("timePoint"); values.append(sqrlStudy.timePoint);
        variables.append("equipment"); values.append(sqrlStudy.equipment);

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


/* ------------------------------------------------------------ */
/* ----- DisplaySeriesDetails --------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::DisplaySeriesDetails(QString subjectID, int studyNum, int seriesNum) {

    /* find subject */
    squirrelSeries sqrlSeries;
    if (sqrl->GetSeries(subjectID, studyNum, seriesNum, sqrlSeries)) {

        /* load subject details */
        QStringList variables, values;
        variables.append("number"); values.append(QString("%1").arg(sqrlSeries.number));
        variables.append("dateTime"); values.append(sqrlSeries.dateTime.toString("yyyy-MM-dd hh:mm:ss"));
        variables.append("seriesUID"); values.append(sqrlSeries.seriesUID);
        variables.append("description"); values.append(sqrlSeries.description);
        variables.append("protocol"); values.append(sqrlSeries.protocol);
        variables.append("numFiles"); values.append(QString("%1").arg(sqrlSeries.numFiles));
        variables.append("size"); values.append(QString("%1").arg(sqrlSeries.size));
        variables.append("numBehFiles"); values.append(QString("%1").arg(sqrlSeries.numBehFiles));
        variables.append("behSize"); values.append(QString("%1").arg(sqrlSeries.behSize));
        variables.append("params"); values.append("...");
        variables.append("stagedFiles"); values.append("...");
        variables.append("stagedBehFiles"); values.append("...");
        variables.append("experimentList"); values.append("...");
        variables.append("virtualPath"); values.append(sqrlSeries.virtualPath);

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
        /* series not found */
    }
}


/* ------------------------------------------------------------ */
/* ----- ClosePackage ----------------------------------------- */
/* ------------------------------------------------------------ */
bool MainWindow::ClosePackage() {

    bool retVal = false;

    if (sqrl->okToDelete()) {
        ui->txtOutput->appendHtml("<b>Closing package</b> - deleting tmp directory [" + sqrl->GetTempDir() + "]");
        delete sqrl;
        sqrlValid = false;
        retVal = true;
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("This squirrel package is not saved");
        msgBox.setInformativeText("Do you want to save this package?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                // "Save" was clicked - save if the package was valid and an existing. prompt for filename if the package was new
                retVal = true;
                break;
            case QMessageBox::Discard:
                // "Dont Save" was clicked - delete the package
                ui->txtOutput->appendHtml("Not saving package. Deleting tmp directory [" + sqrl->GetTempDir() + "]");
                delete sqrl;
                sqrlValid = false;
                retVal = true;
                break;
            case QMessageBox::Cancel:
                // "Cancel" was clicked - leave the function without doing anything
                retVal = false;
                break;
            default:
                // should never be reached
                break;
        }
    }

    /* clean up the GUI */
    if (retVal) {

        /* clear package info */
        ui->lblPackagePath->clear();
        ui->lblPackageName->clear();
        ui->lblPackageDesc->clear();
        ui->lblPackageDate->clear();
        ui->lblPackageDirFormat->clear();
        ui->lblPackageDataFormat->clear();
        ui->lblTempPath->clear();

        /* clear subject tree */
        ui->subjectTree->clear();

        /* clear experiments table */
        for (int i=1; i<ui->experimentsTable->rowCount(); i++) {
            ui->experimentsTable->removeRow(i);
        }

        /* clear pipelines table */
        for (int i=1; i<ui->pipelinesTable->rowCount(); i++) {
            ui->pipelinesTable->removeRow(i);
        }
    }

    return retVal;
}


/* ------------------------------------------------------------ */
/* ----- AddDICOM --------------------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::AddDICOM() {
    QString dicomdir;
    dicomdir = QFileDialog::getExistingDirectory(this, tr("Select DICOM directory"), "/");

    if (dicomdir != "") {
        ui->lblStatus->setText("<font color='blue'>Reading DICOM directory</font>");
        qApp->processEvents();
        QApplication::setOverrideCursor(Qt::WaitCursor);

        /* create the dicom object to read the DICOM dir */
        dicom dcm;
        dcm.LoadToSquirrel(dicomdir, "", sqrl);

        QApplication::restoreOverrideCursor();
        ui->lblStatus->setText("Done reading DICOM directory");

        RefreshSubjectsTable();
    }
}


/* ------------------------------------------------------------ */
/* ----- AddSubject ------------------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::AddSubject() {

    objectDialog *subjectInfo = new objectDialog("subject");
    if (subjectInfo->exec()) {

        /* create an empty squirrel object if it doesn't exist already */
        if (!sqrlValid) {
            sqrl = new squirrel();
            sqrlValid = true;
        }

        /* add the subject to the squirrel object */
        squirrelSubject sqrlSubject;
        sqrlSubject.ID = subjectInfo->GetValue("ID");
        sqrlSubject.alternateIDs = subjectInfo->GetValue("alternateIDs").split(",");
        sqrlSubject.GUID = subjectInfo->GetValue("GUID");
        sqrlSubject.dateOfBirth.fromString(subjectInfo->GetValue("dateOfBirth"), "yyyy-MM-dd");
        sqrlSubject.sex = subjectInfo->GetValue("sex");
        sqrlSubject.gender = subjectInfo->GetValue("gender");
        sqrlSubject.ethnicity1 = subjectInfo->GetValue("ethnicity1");
        sqrlSubject.ethnicity2 = subjectInfo->GetValue("ethnicity2");
        sqrl->addSubject(sqrlSubject);
    }

    RefreshSubjectsTable();
}


/* ------------------------------------------------------------ */
/* ----- AddStudy --------------------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::AddStudy() {
    /* get selected item */
    if (ui->subjectTree->selectedItems().size() == 1) {

        /* make sure we're adding this study to an existing subject */
        QTreeWidgetItem *item = ui->subjectTree->selectedItems()[0];
        if (item->data(0,Qt::UserRole) == "subject") {

            /* get squirrel subject */
            squirrelSubject sqrlSubject;
            if (sqrl->GetSubject(item->text(0), sqrlSubject)) {

                /* subject exists. show the study object dialog and add the study */
                objectDialog *studyInfo = new objectDialog("study");
                if (studyInfo->exec()) {

                    /* add the subject to the squirrel object */
                    squirrelStudy sqrlStudy;
                    sqrlStudy.number = studyInfo->GetValue("number").toInt();
                    sqrlStudy.dateTime.fromString(studyInfo->GetValue("dateTime"), "yyyy-MM-dd hh:mm:ss");
                    sqrlStudy.ageAtStudy = studyInfo->GetValue("ageAtStudy").toDouble();
                    sqrlStudy.height = studyInfo->GetValue("height").toDouble();
                    sqrlStudy.weight = studyInfo->GetValue("weight").toDouble();
                    sqrlStudy.modality = studyInfo->GetValue("modality");
                    sqrlStudy.description = studyInfo->GetValue("description");
                    sqrlStudy.studyUID = studyInfo->GetValue("studyUID");
                    sqrlStudy.visitType = studyInfo->GetValue("visitType");
                    sqrlStudy.dayNumber = studyInfo->GetValue("dayNumber");
                    sqrlStudy.timePoint = studyInfo->GetValue("timePoint");
                    sqrlStudy.equipment = studyInfo->GetValue("equipment");
                    sqrlStudy.virtualPath = studyInfo->GetValue("virtualPath");

                    sqrlSubject.addStudy(sqrlStudy);
                }
            }

            RefreshSubjectsTable();
        }
    }
}


/* ------------------------------------------------------------ */
/* ----- AddSeries -------------------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::AddSeries() {
    /* get selected study */
    if (ui->subjectTree->selectedItems().size() == 1) {
        QTreeWidgetItem *item = ui->subjectTree->selectedItems()[0];
        if (item->text(0) == "study") {

            /* add a subject to the data node of the tree */
            QTreeWidgetItem *newItem = new QTreeWidgetItem();
            newItem->setText(0, "seriesID");
            newItem->setData(0, Qt::UserRole, "series");

            item->addChild(newItem);

            RefreshSubjectsTable();
        }
    }
}


/* ------------------------------------------------------------ */
/* ----- AddDrug ---------------------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::AddDrug() {
    /* get selected subject */
    if (ui->subjectTree->selectedItems().size() == 1) {
        QTreeWidgetItem *item = ui->subjectTree->selectedItems()[0];
        if (item->data(0,Qt::UserRole) == "subject") {

            /* add a drug to the node */
            QTreeWidgetItem *newItem = new QTreeWidgetItem();
            newItem->setData(0, Qt::UserRole, "drug");
            newItem->setText(0, "Drug");

            item->addChild(newItem);

            RefreshSubjectsTable();
        }
    }
}


/* ------------------------------------------------------------ */
/* ----- AddMeasure ------------------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::AddMeasure() {
    /* get selected subject */
    if (ui->subjectTree->selectedItems().size() == 1) {
        QTreeWidgetItem *item = ui->subjectTree->selectedItems()[0];
        if (item->data(0,Qt::UserRole) == "subject") {
            QString subjectID = item->text(0);

            squirrelSubject sqrlSubject;
            sqrl->GetSubject(subjectID, sqrlSubject);

            squirrelMeasure sqrlMeasure;
            sqrlMeasure.measureName = "Measure";

            sqrlSubject.addMeasure(sqrlMeasure);

            /* add a measure to the node */
            //QTreeWidgetItem *newItem = new QTreeWidgetItem();
            //newItem->setData(0, Qt::UserRole, "measure");
            //newItem->setText(0, "Measure");

            //item->addChild(newItem);

            RefreshSubjectsTable();
        }
    }
}


/* ------------------------------------------------------------ */
/* ----- AddAnalysis ------------------------------------------ */
/* ------------------------------------------------------------ */
void MainWindow::AddAnalysis() {
}


/* ------------------------------------------------------------ */
/* ----- AddExperiment ---------------------------------------- */
/* ------------------------------------------------------------ */
void MainWindow::AddExperiment() {
}


/* ------------------------------------------------------------ */
/* ----- AddPipeline ------------------------------------------ */
/* ------------------------------------------------------------ */
void MainWindow::AddPipeline() {
}
