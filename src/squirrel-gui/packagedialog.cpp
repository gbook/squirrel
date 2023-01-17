#include "packagedialog.h"
#include "ui_packagedialog.h"

packageDialog::packageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::packageDialog)
{
	ui->setupUi(this);
}

packageDialog::~packageDialog()
{
	delete ui;
}

void packageDialog::GetValues(QString &pkgName, QString &pkgDesc, QDateTime &pkgDate, QString &pkgDirFormat, QString &pkgDataFormat) {
	pkgName = ui->packageName->text();
	pkgDesc = ui->packageDesc->toPlainText();
	pkgDate = ui->packageDate->dateTime();
	pkgDirFormat = ui->packageDirFormat->itemText(ui->packageDirFormat->currentIndex());
	pkgDataFormat = ui->packageDataFormat->itemText(ui->packageDataFormat->currentIndex());
}


void packageDialog::SetValues(QString pkgName, QString pkgDesc, QDateTime pkgDate, QString pkgDirFormat, QString pkgDataFormat) {
	ui->packageName->setText(pkgName);
	ui->packageDesc->setPlainText(pkgDesc);
	ui->packageDate->setDateTime(pkgDate);
	ui->packageDirFormat->setCurrentIndex(ui->packageDirFormat->findText(pkgDirFormat));
	ui->packageDataFormat->setCurrentIndex(ui->packageDataFormat->findText(pkgDataFormat));
}
