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

	void SetEditType(bool createNew);
	void GetValues(QString &pkgName, QString &pkgDesc, QDateTime &pkgDate, QString &pkgDirFormat, QString &pkgDataFormat);
	void SetValues(QString pkgName, QString pkgDesc, QDateTime pkgDate, QString pkgDirFormat, QString pkgDataFormat);
private slots:
	void on_btnSetCurrentDateTime_clicked();

private:
	Ui::packageDialog *ui;
};

#endif // PACKAGEDIALOG_H
