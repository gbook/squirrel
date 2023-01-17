#ifndef PACKAGEDIALOG_H
#define PACKAGEDIALOG_H

#include <QDialog>

namespace Ui {
class packageDialog;
}

class packageDialog : public QDialog
{
	Q_OBJECT

public:
	explicit packageDialog(QWidget *parent = nullptr);
	~packageDialog();

	void GetValues(QString &pkgName, QString &pkgDesc, QDateTime &pkgDate, QString &pkgDirFormat, QString &pkgDataFormat);
	void SetValues(QString pkgName, QString pkgDesc, QDateTime pkgDate, QString pkgDirFormat, QString pkgDataFormat);
private:
	Ui::packageDialog *ui;
};

#endif // PACKAGEDIALOG_H
