/* ------------------------------------------------------------------------------
  Squirrel squirrel.h
  Copyright (C) 2004 - 2024
  Gregory A Book <gregory.book@hhchealth.org> <gregory.a.book@gmail.com>
  Olin Neuropsychiatry Research Center, Hartford Hospital
  ------------------------------------------------------------------------------
  GPLv3 License:

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  ------------------------------------------------------------------------------ */

#ifndef SQUIRREL_H
#define SQUIRREL_H

#include <QString>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QtSql>
#include "squirrelSubject.h"
#include "squirrelExperiment.h"
#include "squirrelPipeline.h"
#include "squirrelMeasure.h"
#include "squirrelDrug.h"
#include "squirrelGroupAnalysis.h"
#include "squirrelDataDictionary.h"
#include "squirrelVersion.h"

/**
 * @brief The squirrel class
 *
 * provides a complete class to read, write, and validate squirrel files
 */
class squirrel
{
public:
    squirrel(bool dbg=false, bool q=false);
    ~squirrel();

    bool read(QString filename, bool headerOnly, bool validateOnly=false);
    bool write(QString outpath, QString &filepath);
    bool validate();
    void print();

    bool addSubject(squirrelSubject subj);
    bool addPipeline(squirrelPipeline pipe);
    //bool addExperiment(squirrelExperiment exp);
    bool removeSubject(QString ID);

    /* JSON elements */
    QDateTime datetime;         /*!< datetime the package was created */
    QString description;        /*!< detailed description of the package */
    QString name;               /*!< name of the package */
    QString NiDBversion;        /*!< NiDB version that wrote this package */
    QString version;            /*!< squirrel version */
    QString format;             /*!< 'dir' or 'zip' */
    QString subjectDirFormat;   /*!< orig, seq */
    QString studyDirFormat;     /*!< orig, seq */
    QString seriesDirFormat;    /*!< orig, seq */
    QString dataFormat;         /*!< orig, anon, anonfull, nift3d, nifti3dgz, nifti4d, nifti4dgz */
    QString license;            /*!< a data usage license */
    QString readme;             /*!< a README */
    QString changes;            /*!< any changes since last package release */
    QString notes;              /*!< JSON string of notes (may contain JSON sub-elements of 'import', 'merge', 'export') */

    /* lib variables */
    QString filePath;           /*!< full path to the zip file */

    /* package information */
    qint64 GetUnzipSize();
    qint64 GetNumFiles();
    qint64 GetNumSubjects();
    qint64 GetNumStudies();
    qint64 GetNumSeries();
    qint64 GetNumMeasures();
    qint64 GetNumDrugs();
    qint64 GetNumAnalyses();
    qint64 GetNumExperiments();
    qint64 GetNumPipelines();
    qint64 GetNumGroupAnalyses();
    qint64 GetNumDataDictionaryItems();

    /* subject, pipeline, and experiment data */
    QList<squirrelSubject> subjectList; /*!< List of subjects within this package */
    QList<squirrelPipeline> pipelineList; /*!< List of pipelines within this package */
    QList<squirrelExperiment> experimentList; /*!< List of experiments within this package */
    QList<squirrelGroupAnalysis> groupAnalysisList; /*!< List of groupAnalyses within this package */

    /* data dictionary (just a single object, not array) */
    squirrelDataDictionary dataDictionary;

    /* searching/retrieval, get index */
    int GetSubjectIndex(QString ID);
    int GetStudyIndex(QString ID, int studyNum);
    int GetSeriesIndex(QString ID, int studyNum, int seriesNum);
    int GetExperimentIndex(QString experimentName);
    int GetPipelineIndex(QString pipelineName);

    /* searching/retrieval functions - get copies */
    bool GetSubject(QString ID, squirrelSubject &sqrlSubject);
    bool GetStudy(QString ID, int studyNum, squirrelStudy &sqrlStudy);
    bool GetSeries(QString ID, int studyNum, int seriesNum, squirrelSeries &sqrlSeries);
    bool GetSubjectList(QList<squirrelSubject> &subjects);
    bool GetStudyList(QString ID, QList<squirrelStudy> &studies);
    bool GetSeriesList(QString ID, int studyNum, QList<squirrelSeries> &series);
    bool GetDrugList(QString ID, QList<squirrelDrug> &drugs);
    bool GetMeasureList(QString ID, QList<squirrelMeasure> &measures);
    bool GetAnalysis(QString ID, int studyNum, QString pipelineName, squirrelAnalysis &sqrlAnalysis);
    bool GetPipeline(QString pipelineName, squirrelPipeline &sqrlPipeline);
    bool GetExperiment(QString experimentName, squirrelExperiment &sqrlExperiment);
    QList<int> GetExperimentIDList();

    /* validation functions */
    QString GetTempDir();
    bool valid() { return isValid; }
    bool okToDelete() { return isOkToDelete; }

    /* functions to manipulate, add files */
    bool AddSeriesFiles(QString ID, int studyNum, int seriesNum, QStringList files, QString destDir="");
    bool AddAnalysisFiles(QString ID, int studyNum, QString pipelineName, QStringList files, QString destDir="");
    bool AddPipelineFiles(QString pipelineName, QStringList files, QString destDir="");
    bool AddExperimentFiles(QString experimentName, QStringList files, QString destDir="");
    bool AddGroupAnalysisFiles(QString experimentName, QStringList files, QString destDir="");

    /* functions to read special files */
    QHash<QString, QString> ReadParamsFile(QString f);

    /* logging */
    void Log(QString s, QString func, bool dbg=false);
    QString GetLog() { return log; }
    bool GetDebug() { return debug; }
    bool quiet=false;

    /* printing of information to console */
    void PrintPackage();
    void PrintSubjects(bool details=false);
    void PrintStudies(QString subjectID, bool details=false);
    void PrintSeries(QString subjectID, int studyNum, bool details=false);
    void PrintExperiments(bool details=false);
    void PrintPipelines(bool details=false);
    void PrintGroupAnalyses(bool details=false);
    void PrintDataDictionary(bool details=false);

private:
    bool MakeTempDir(QString &dir);
    bool DatabaseConnect();
    bool InitializeDatabase();

    QString workingDir;
    QString logfile;
    QStringList msgs; /* squirrel messages, to be passed back upon writing (or reading) through the squirrel library */
    QString log;

    QSqlDatabase db;
    //QString schema;

    bool debug;
    bool isValid;
    bool isOkToDelete;
};

#endif // SQUIRREL_H
