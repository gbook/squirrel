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

    void on_actionE_xit_triggered();

    void on_btnAddStudy_clicked();

    void on_btnAddSeries_clicked();

    void on_action_New_package_triggered();

    void on_actionOpen_triggered();

    void on_btnNewPackage_clicked();

    void on_btnEditPackageDetails_clicked();

    void on_btnAddAnalysis_clicked();

    void on_btnAddDrug_clicked();

    void on_btnAddMeasure_clicked();

    void on_btnAddExperiment_clicked();

    void on_btnAddPipeline_clicked();

    void on_btnAddDICOM_clicked();

    void on_action_Save_package_triggered();

    void on_action_Help_triggered();

    void on_action_About_triggered();

    void on_actionValidate_triggered();

    void on_actionClose_triggered();

	void on_btnOpenPackage_clicked();

	void on_btnClosePackage_clicked();

    void on_subjectTree_itemClicked(QTreeWidgetItem *item, int column);

    void on_subjectTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_subjectTree_itemChanged(QTreeWidgetItem *item, int column);

private:
    Ui::MainWindow *ui;

    /* the main squirrel object */
	squirrel *sqrl;

	void OpenPackage();
	bool NewPackage();
    bool ClosePackage();
    bool EditPackageDetails();

    void DisplaySubjectDetails(QString ID);
    void DisplayStudyDetails(QString subjectID, int studyNum);
    void DisplaySeriesDetails(QString subjectID, int studyNum, int seriesNum);

    void AddDICOM();
    void AddSubject();
    void AddStudy();
    void AddSeries();
    void AddDrug();
    void AddMeasure();
    void AddAnalysis();
    void AddExperiment();
    void AddPipeline();

    void RefreshTopInfoTable();
    void RefreshSubjectTable();
    void RefreshPackageDetails();
    void EnableDisableSubjectButtons();
};
#endif // MAINWINDOW_H
