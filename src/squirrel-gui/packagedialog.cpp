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


/* ------------------------------------------------------------ */
/* ----- GetValues -------------------------------------------- */
/* ------------------------------------------------------------ */
void packageDialog::GetValues(QString &pkgName, QString &pkgDesc, QDateTime &pkgDate, QString &pkgDirFormat, QString &pkgDataFormat) {
	pkgName = ui->packageName->text();
	pkgDesc = ui->packageDesc->toPlainText();
	pkgDate = ui->packageDate->dateTime();
	pkgDirFormat = ui->packageDirFormat->itemText(ui->packageDirFormat->currentIndex());
	pkgDataFormat = ui->packageDataFormat->itemText(ui->packageDataFormat->currentIndex());
}


/* ------------------------------------------------------------ */
/* ----- SetValues -------------------------------------------- */
/* ------------------------------------------------------------ */
void packageDialog::SetValues(QString pkgName, QString pkgDesc, QDateTime pkgDate, QString pkgDirFormat, QString pkgDataFormat) {
	ui->packageName->setText(pkgName);
	ui->packageDesc->setPlainText(pkgDesc);
	ui->packageDate->setDateTime(pkgDate);
	ui->packageDirFormat->setCurrentIndex(ui->packageDirFormat->findText(pkgDirFormat));
	ui->packageDataFormat->setCurrentIndex(ui->packageDataFormat->findText(pkgDataFormat));
}


/* ------------------------------------------------------------ */
/* ----- SetEditType ------------------------------------------ */
/* ------------------------------------------------------------ */
void packageDialog::SetEditType(bool createNew) {
	/* enable all fields of the package info for new packages */
	if (createNew) {
		ui->packageName->setEnabled(true);
		ui->packageDesc->setEnabled(true);
		ui->packageDate->setEnabled(true);
		ui->packageDirFormat->setEnabled(true);
		ui->packageDataFormat->setEnabled(true);
	}
	/* enable only name, desc for existing packages */
	else {
		ui->packageName->setEnabled(true);
		ui->packageDesc->setEnabled(true);
		ui->packageDate->setDisabled(true);
		ui->packageDirFormat->setDisabled(true);
		ui->packageDataFormat->setDisabled(true);
	}
}

void packageDialog::on_btnSetCurrentDateTime_clicked()
{
	ui->packageDate->setDateTime(QDateTime::currentDateTime());
}

