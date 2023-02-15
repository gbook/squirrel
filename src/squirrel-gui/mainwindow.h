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

    void on_action_New_package_triggered();

    void on_actionOpen_triggered();

    void on_btnNewPackage_clicked();

    void on_btnEditPackageDetails_clicked();

    void on_btnAddAnalysis_clicked();

    void on_btnAddDrug_clicked();

    void on_btnAddMeasure_clicked();

    void on_btnAddExperiment_clicked();

    void on_btnAddPipeline_clicked();

    void on_packageTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_btnAddDICOM_clicked();

private:
    Ui::MainWindow *ui;

    /* the main squirrel object */
    squirrel *sqrl;

    bool NewPackage();
    bool EditPackageDetails();
    void DisplaySubjectDetails(QString ID);

    void RefreshTopInfoTable();
	void RefreshSubjectTable();
	void RefreshPackageDetails();
    void EnableDisableSubjectButtons();
};
#endif // MAINWINDOW_H
