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

//#include <string>
#include <QString>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QtSql>
#include "squirrelSubject.h"
#include "squirrelStudy.h"
#include "squirrelSeries.h"
#include "squirrelExperiment.h"
#include "squirrelPipeline.h"
#include "squirrelObservation.h"
#include "squirrelIntervention.h"
#include "squirrelGroupAnalysis.h"
#include "squirrelDataDictionary.h"
#include "squirrelVersion.h"

enum FileMode { NewPackage, ExistingPackage };
enum PrintingType { IDList, Details, CSV, Tree };
typedef QPair<QString, QString> QStringPair;
typedef QList<QStringPair> pairList;

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

    bool Read();
    bool Write(bool writeLog);
    bool Validate();
    QString Print();
    void SetPackagePath(QString p) { packagePath = p; }
    QString GetPackagePath();
    void SetFileMode(FileMode m) { fileMode = m; } /*!< Set the file mode to either NewPackage or ExistingPackage */
    void SetDebugSQL(bool d) { debugSQL = d; }
    void SetOverwritePackage(bool o) { overwritePackage = o; }
    void SetQuickRead(bool q) { quickRead = q; }
    //bool totalArchiveSizeCallback(qint64 val);
    //bool progressCallback(qint64 val);

    /* package JSON elements */
    QDateTime Datetime;         /*!< datetime the package was created */
    QString Changes;            /*!< any changes since last package release */
    QString DataFormat;         /*!< orig, anon, anonfull, nift3d, nifti3dgz, nifti4d, nifti4dgz */
    QString Description;        /*!< detailed description of the package */
    QString License;            /*!< a data usage license */
    QString NiDBversion;        /*!< NiDB version that wrote this package */
    QString Notes;              /*!< JSON string of notes (may contain JSON sub-elements of 'import', 'merge', 'export') */
    QString PackageFormat;      /*!< 'squirrel' */
    QString PackageName;        /*!< name of the package */
    QString Readme;             /*!< a README */
    QString SeriesDirFormat;    /*!< orig, seq */
    QString SquirrelBuild;      /*!< squirrel build */
    QString SquirrelVersion;    /*!< squirrel version */
    QString StudyDirFormat;     /*!< orig, seq */
    QString SubjectDirFormat;   /*!< orig, seq */

    /* get list(s) of objects */
    QList<squirrelExperiment> GetAllExperiments();
    QList<squirrelPipeline> GetAllPipelines();
    QList<squirrelSubject> GetAllSubjects();
    QList<squirrelStudy> GetStudies(qint64 subjectRowID);
    QList<squirrelSeries> GetSeries(qint64 studyRowID);
    QList<squirrelAnalysis> GetAnalyses(qint64 studyRowID);
    QList<squirrelObservation> GetObservations(qint64 subjectRowID);
    QList<squirrelIntervention> GetInterventions(qint64 subjectRowID);
    QList<squirrelGroupAnalysis> GetAllGroupAnalyses();
    QList<squirrelDataDictionary> GetAllDataDictionaries();

    /* get numbers of objects */
    qint64 GetFileCount();
    qint64 GetObjectCount(QString object);

    /* find objects, return rowID */
    qint64 FindSubject(QString id);
    qint64 FindStudy(QString subjectID, int studyNum);
    qint64 FindStudyByUID(QString studyUID);
    qint64 FindSeries(QString subjectID, int studyNum, int seriesNum);
    qint64 FindSeriesByUID(QString seriesUID);
    qint64 FindAnalysis(QString subjectID, int studyNum, QString analysisName);
    qint64 FindExperiment(QString experimentName);
    qint64 FindPipeline(QString pipelineName);
    qint64 FindGroupAnalysis(QString groupAnalysisName);
    qint64 FindDataDictionary(QString dataDictionaryName);

    /* remove objects */
    bool RemoveSubject(qint64 subjectRowID);
    bool RemoveStudy(qint64 studyRowID);
    bool RemoveSeries(qint64 seriesRowID);
    bool RemoveObservation(qint64 observationRowID);
    bool RemoveIntervention(qint64 InterventionRowID);
    bool RemoveAnalysis(qint64 analysisRowID);
    bool RemoveExperiment(qint64 experimentRowID);
    bool RemovePipeline(qint64 pipelineRowID);
    bool RemoveGroupAnalysis(qint64 groupAnalysisRowID);
    bool RemoveDataDictionary(qint64 dataDictionaryRowID);

    bool AddStagedFiles(QString objectType, qint64 rowid, QStringList files);

    /* requence the subject data */
    void ResequenceSubjects();
    void ResequenceStudies(qint64 subjectRowID);
    void ResequenceSeries(qint64 studyRowID);

    /* package information */
    qint64 GetUnzipSize();

    /* validation functions */
    QString GetTempDir();
    bool IsValid() { return isValid; }
    bool OkToDelete() { return isOkToDelete; }

    /* functions to read special files */
    QHash<QString, QString> ReadParamsFile(QString f);

    /* logging */
    void Log(QString s, QString func);
    void Debug(QString s, QString func="");
    QString GetLog() { return log; }
    QString GetLogBuffer();
    bool GetDebug() { return debug; }
    bool GetDebugSQL() { return debugSQL; }
    bool quiet=false;

    /* printing of information to console */
    QString PrintPackage();
    QString PrintSubjects(PrintingType printType=PrintingType::IDList);
    QString PrintStudies(qint64 subjectRowID, bool details=false);
    QString PrintSeries(qint64 studyRowID, bool details=false);
    QString PrintExperiments(bool details=false);
    QString PrintPipelines(bool details=false);
    QString PrintGroupAnalyses(bool details=false);
    QString PrintDataDictionary(bool details=false);

    QSqlDatabase db;

private:
    bool MakeTempDir(QString &dir);
    bool DeleteTempDir(QString dir);
    bool DatabaseConnect();
    bool InitializeDatabase();
    bool ExtractFileFromArchive(QString archivePath, QString filePath, QString &fileContents);
    bool CompressDirectoryToArchive(QString dir, QString archivePath, QString &m);
    bool AddFilesToArchive(QStringList filePaths, QStringList compressedFilePaths, QString archivePath, QString &m);
    bool RemoveDirectoryFromArchive(QString compressedDirPath, QString archivePath, QString &m);
    bool UpdateMemoryFileToArchive(QString file, QString compressedFilePath, QString archivePath, QString &m);
    bool Get7zipLibPath();

    QString workingDir;
    QString logfile;
    QStringList msgs; /* squirrel messages to be passed back through the squirrel library */
    QString log;
    QString logBuffer;
    QString packagePath;
    QString p7zipLibPath;

    FileMode fileMode;

    bool debug;
    bool debugSQL;
    bool overwritePackage;
    bool isValid;
    bool isOkToDelete;
    bool quickRead; /* set true to skip reading of the params.json files */
};

#endif // SQUIRREL_H
