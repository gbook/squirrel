#ifndef OBJECTDIALOG_H
#define OBJECTDIALOG_H

#include <QDialog>

namespace Ui {
class objectDialog;
}

class objectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit objectDialog(QString objectType, QWidget *parent = nullptr);
    ~objectDialog();

    QString GetValue(QString variable);

private:
    Ui::objectDialog *ui;
};

#endif // OBJECTDIALOG_H
