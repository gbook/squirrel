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
#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
    /* check to see what type of object was selected */
    if (ui->packageTree->selectedItems().size() == 1) {
        QTreeWidgetItem *item = ui->packageTree->selectedItems()[0];
        //qDebug() << item->data(0,Qt::EditRole);
        if (item->data(0, Qt::EditRole) == "subject") {
            item->setExpanded(true);
        }
    }

}


/* ------------------------------------------------------------ */
/* ----- on_packageTree_itemDoubleClicked --------------------- */
/* ------------------------------------------------------------ */
void MainWindow::on_packageTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    /* display the variable details in a new dialog box so they can be edited */

    /* get selected study */
    if (ui->packageTree->selectedItems().size() == 1) {
        QTreeWidgetItem *item = ui->packageTree->selectedItems()[0];
        QString dataCategory = item->data(0, Qt::EditRole).toString();

        if (dataCategory == "subject") {
            /* display the subject table */

        }
        else if (dataCategory == "study") {
            /* display the study table */

        }
        else if (dataCategory == "series") {
            /* display the series table */

        }
    }
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
        //qDebug() << item->data(0,Qt::EditRole);
        if (item->data(0, Qt::EditRole) == "subject") {
            /* add a subject to the data node of the tree */
            QTreeWidgetItem *newItem = new QTreeWidgetItem();
            newItem->setText(1, "Study");
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
        //qDebug() << item->data(0,Qt::EditRole);
        if (item->data(0, Qt::EditRole) == "study") {
            /* add a subject to the data node of the tree */
            QTreeWidgetItem *newItem = new QTreeWidgetItem();
            newItem->setText(2, "Series");
            newItem->setData(0, Qt::EditRole, "series");

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
	EnableDisableSubjectButtons();

    /* get selected study */
    if (ui->packageTree->selectedItems().size() == 1) {
        QTreeWidgetItem *item = ui->packageTree->selectedItems()[0];
		QString dataCategory = item->data(0, Qt::EditRole).toString().trimmed();
		QString subjectID = item->text(0);

        if (dataCategory == "subject") {
            /* display the subject table */
			DisplaySubjectDetails(subjectID);
        }
        else if (dataCategory == "study") {
            /* display the study table */

        }
        else if (dataCategory == "series") {
            /* display the series table */

        }
    }
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
	sqrl->read(sqrl->filePath);
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
	int subjIndex(-1);
	for (int i=0; i<sqrl->subjectList.size(); i++) {
		if (sqrl->subjectList[i].ID == ID) {
			subjIndex = i;
			break;
		}
	}

	/* load subject details */
	QStringList variables, values;
	variables.append("ID"); values.append(sqrl->subjectList[subjIndex].ID);
	variables.append("alternateIDs"); values.append(sqrl->subjectList[subjIndex].alternateIDs.join(","));
	variables.append("GUID"); values.append(sqrl->subjectList[subjIndex].GUID);
	variables.append("dateOfBirth"); values.append(sqrl->subjectList[subjIndex].dateOfBirth.toString());
	variables.append("sex"); values.append(sqrl->subjectList[subjIndex].sex);
	variables.append("gender"); values.append(sqrl->subjectList[subjIndex].gender);
	variables.append("ethnicity1"); values.append(sqrl->subjectList[subjIndex].ethnicity1);
	variables.append("ethnicity2"); values.append(sqrl->subjectList[subjIndex].ethnicity2);

	/* clear table */
	ui->subjectDetailsTable->setRowCount(0);

	for (int i=0; i<variables.size(); i++) {
		int currentRow = ui->subjectDetailsTable->rowCount();
		ui->subjectDetailsTable->setRowCount(currentRow + 1);
		ui->subjectDetailsTable->setItem(currentRow, 0, new QTableWidgetItem(variables[i]));
		ui->subjectDetailsTable->setItem(currentRow, 1, new QTableWidgetItem(values[i]));
	}
}
