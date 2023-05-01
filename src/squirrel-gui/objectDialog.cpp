#include "objectDialog.h"
#include "ui_objectDialog.h"

objectDialog::objectDialog(QString objectType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::objectDialog)
{
    ui->setupUi(this);

    ui->objectTable->setStyleSheet("QHeaderView::section { background-color:#444; color: #fff}");

    QStringList variables, values;

    if (objectType == "subject") {
        variables.append("ID"); values.append("S1234ABC");
        variables.append("alternateIDs"); values.append("");
        variables.append("GUID"); values.append("");
        variables.append("dateOfBirth"); values.append("");
        variables.append("sex"); values.append("");
        variables.append("gender"); values.append("");
        variables.append("ethnicity1"); values.append("");
        variables.append("ethnicity2"); values.append("");
    }
    else if (objectType == "study") {
        variables.append("number"); values.append("");
        variables.append("dateTime"); values.append("");
        variables.append("ageAtStudy"); values.append("");
        variables.append("height"); values.append("");
        variables.append("weight"); values.append("");
        variables.append("modality"); values.append("");
        variables.append("description"); values.append("");
        variables.append("studyUID"); values.append("");
        variables.append("visitType"); values.append("");
        variables.append("dayNumber"); values.append("");
        variables.append("timePoint"); values.append("");
        variables.append("equipment"); values.append("");
        variables.append("virtualPath"); values.append("");
    }
    else if (objectType == "series") {
        variables.append("number"); values.append("");
        variables.append("dateTime"); values.append("");
        variables.append("seriesUID"); values.append("");
        variables.append("description"); values.append("");
        variables.append("protocol"); values.append("");
        variables.append("experimentName"); values.append("");
        variables.append("size"); values.append("");
        variables.append("numFiles"); values.append("");
        variables.append("behSize"); values.append("");
        variables.append("numbehFiles"); values.append("");
    }

    for (int i=0; i<variables.size(); i++) {
        int currentRow = ui->objectTable->rowCount();
        ui->objectTable->setRowCount(currentRow + 1);
        ui->objectTable->setItem(currentRow, 0, new QTableWidgetItem(variables[i]));
        ui->objectTable->setItem(currentRow, 1, new QTableWidgetItem(values[i]));
    }
}


QString objectDialog::GetValue(QString variable) {
    for (int i=0; i<ui->objectTable->rowCount(); i++) {
        if (ui->objectTable->item(i,0)->text() == variable) {
            return ui->objectTable->item(i,1)->text();
        }
    }
    return "";
}


objectDialog::~objectDialog()
{
    delete ui;
}
