/* ------------------------------------------------------------------------------
  Squirrel GUI mergeDialog.cpp
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

#include "mergeDialog.h"
#include "ui_mergeDialog.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>

mergeDialog::mergeDialog(QWidget *parent) : QDialog(parent), ui(new Ui::mergeDialog) {
    ui->setupUi(this);

    connect(ui->addButton, &QPushButton::clicked, this, &mergeDialog::AddPackages);
    connect(ui->removeButton, &QPushButton::clicked, this, &mergeDialog::RemoveSelected);
    connect(ui->upButton, &QPushButton::clicked, this, &mergeDialog::MoveUp);
    connect(ui->downButton, &QPushButton::clicked, this, &mergeDialog::MoveDown);
    connect(ui->browseButton, &QPushButton::clicked, this, &mergeDialog::BrowseOutput);
    connect(ui->renumberCheck, &QCheckBox::toggled, this, &mergeDialog::UpdateWidgetStates);
    connect(ui->inputList, &QListWidget::itemSelectionChanged, this, &mergeDialog::UpdateWidgetStates);

    UpdateWidgetStates();
}


mergeDialog::~mergeDialog() {
    delete ui;
}


QStringList mergeDialog::InputPaths() const {
    QStringList paths;
    for (int i = 0; i < ui->inputList->count(); i++)
        paths.append(ui->inputList->item(i)->text());
    return paths;
}


QString mergeDialog::OutputPath() const {
    return ui->outputEdit->text().trimmed();
}


bool mergeDialog::TestOnly() const {
    return ui->testOnlyCheck->isChecked();
}


bool mergeDialog::RenumberSubjects() const {
    return ui->renumberCheck->isChecked();
}


int mergeDialog::Digits() const {
    return ui->digitsSpin->value();
}


/* ------------------------------------------------------------------------------
   AddPackages
   ------------------------------------------------------------------------------ */
void mergeDialog::AddPackages() {

    QStringList files = QFileDialog::getOpenFileNames(this, "Select squirrel packages to merge", QString(), "squirrel packages (*.sqrl *.zip);;All files (*)");

    for (const QString &file : files) {
        /* don't add the same package twice - merging a package with itself
           produces duplicate subject IDs */
        if (ui->inputList->findItems(file, Qt::MatchExactly).isEmpty())
            ui->inputList->addItem(file);
    }

    UpdateWidgetStates();
}


/* ------------------------------------------------------------------------------
   RemoveSelected
   ------------------------------------------------------------------------------ */
void mergeDialog::RemoveSelected() {

    qDeleteAll(ui->inputList->selectedItems());
    UpdateWidgetStates();
}


/* ------------------------------------------------------------------------------
   MoveUp - merge order matters, so the list is manually ordered
   ------------------------------------------------------------------------------ */
void mergeDialog::MoveUp() {

    int row = ui->inputList->currentRow();
    if (row < 1)
        return;

    QListWidgetItem *item = ui->inputList->takeItem(row);
    ui->inputList->insertItem(row - 1, item);
    ui->inputList->setCurrentRow(row - 1);
}


/* ------------------------------------------------------------------------------
   MoveDown
   ------------------------------------------------------------------------------ */
void mergeDialog::MoveDown() {

    int row = ui->inputList->currentRow();
    if ((row < 0) || (row >= ui->inputList->count() - 1))
        return;

    QListWidgetItem *item = ui->inputList->takeItem(row);
    ui->inputList->insertItem(row + 1, item);
    ui->inputList->setCurrentRow(row + 1);
}


/* ------------------------------------------------------------------------------
   BrowseOutput
   ------------------------------------------------------------------------------ */
void mergeDialog::BrowseOutput() {

    QString file = QFileDialog::getSaveFileName(this, "Merged package", ui->outputEdit->text(), "squirrel packages (*.sqrl);;All files (*)");
    if (!file.isEmpty())
        ui->outputEdit->setText(file);
}


/* ------------------------------------------------------------------------------
   UpdateWidgetStates
   ------------------------------------------------------------------------------ */
void mergeDialog::UpdateWidgetStates() {

    bool hasSelection = !ui->inputList->selectedItems().isEmpty();
    ui->removeButton->setEnabled(hasSelection);
    ui->upButton->setEnabled(hasSelection);
    ui->downButton->setEnabled(hasSelection);

    /* the digit count only means anything when subjects are being renumbered */
    bool renumber = ui->renumberCheck->isChecked();
    ui->digitsLabel->setEnabled(renumber);
    ui->digitsSpin->setEnabled(renumber);
}


/* ------------------------------------------------------------------------------
   accept - validate before letting the dialog close
   ------------------------------------------------------------------------------ */
void mergeDialog::accept() {

    if (ui->inputList->count() < 2) {
        QMessageBox::warning(this, "Merge packages", "Select at least two packages to merge.");
        return;
    }

    if (OutputPath().isEmpty()) {
        QMessageBox::warning(this, "Merge packages", "Specify an output package path.");
        return;
    }

    /* MergePackages() would happily overwrite; make that an explicit choice */
    if (!TestOnly() && QFileInfo::exists(OutputPath())) {
        QMessageBox::StandardButton answer = QMessageBox::question(this, "Merge packages", QString("[%1] already exists. Overwrite it?").arg(OutputPath()), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer != QMessageBox::Yes)
            return;
    }

    QDialog::accept();
}
