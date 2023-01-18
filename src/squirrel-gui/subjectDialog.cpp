#include "subjectDialog.h"
#include "ui_subjectDialog.h"

subjectDialog::subjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::subjectDialog)
{
    ui->setupUi(this);

    QBrush my_brush;
    QColor red(Qt::red);
    my_brush.setColor(red);

    ui->subjectTable->setStyleSheet("QHeaderView::section { background-color:#444; color: #fff}");

    QStringList variables, values;
    variables.append("ID"); values.append("S1234ABC");
    variables.append("alternateIDs"); values.append("");
    variables.append("GUID"); values.append("");
    variables.append("dateOfBirth"); values.append("");
    variables.append("sex"); values.append("");
    variables.append("gender"); values.append("");
    variables.append("ethnicity1"); values.append("");
    variables.append("ethnicity2"); values.append("");

    for (int i=0; i<variables.size(); i++) {
        int currentRow = ui->subjectTable->rowCount();
        ui->subjectTable->setRowCount(currentRow + 1);
        ui->subjectTable->setItem(currentRow, 0, new QTableWidgetItem(variables[i]));
        ui->subjectTable->setItem(currentRow, 1, new QTableWidgetItem(values[i]));
    }
}

subjectDialog::~subjectDialog()
{
    delete ui;
}
