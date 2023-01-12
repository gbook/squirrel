#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <squirrel.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnAddSubject_clicked();

    void on_packageTree_itemClicked(QTreeWidgetItem *item, int column);

    void on_packageTree_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_actionE_xit_triggered();

    void on_btnAddStudy_clicked();

    void on_btnAddSeries_clicked();

    void on_packageTree_itemSelectionChanged();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
