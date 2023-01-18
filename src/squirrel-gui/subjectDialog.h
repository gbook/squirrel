#ifndef SUBJECTDIALOG_H
#define SUBJECTDIALOG_H

#include <QDialog>

namespace Ui {
class subjectDialog;
}

class subjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit subjectDialog(QWidget *parent = nullptr);
    ~subjectDialog();

private:
    Ui::subjectDialog *ui;
};

#endif // SUBJECTDIALOG_H
