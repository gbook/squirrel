#include "aboutDialog.h"
#include "ui_aboutDialog.h"
#include <squirrelVersion.h>

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

    ui->lblBuildDate->setText(QString("%1 %2").arg(__DATE__).arg(__TIME__));
    ui->lblSquirrelVersion->setText(QString("%1.%2").arg(SQUIRREL_VERSION_MAJ).arg(SQUIRREL_VERSION_MIN));
    ui->lblCpp->setText(QString("%1").arg(__cplusplus));
    ui->lblCompiled->setText(QString("%1").arg(QT_VERSION_STR));
    ui->lblRuntime->setText(QString("%1").arg(qVersion()));
    ui->lblBuildSystem->setText(QString("%1").arg(QSysInfo::buildAbi()));

}

About::~About()
{
    delete ui;
}
