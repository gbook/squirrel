#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief MainWindow::on_btnAddSubject_clicked
 */
void MainWindow::on_btnAddSubject_clicked()
{
    /* add the subject to the squirrel object */

    /* add a subject to the data node of the tree */
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, "New Subject");
    //item->setText(1, "");
    //item->setText(2, "Series");
    item->setData(0, Qt::EditRole, "subject");
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    ui->packageTree->addTopLevelItem(item);
}

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


void MainWindow::on_actionE_xit_triggered()
{
    /* check if unsaved work */

    /* then exit */
    exit(0);
}


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


void MainWindow::on_packageTree_itemSelectionChanged()
{
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


void MainWindow::on_action_New_package_triggered()
{
    /* create a new blank squirrel object */
    sqrl = new squirrel();
    //sqrl->
}

