/* ------------------------------------------------------------------------------
  Squirrel squirrel.sql.h
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
#include <QString>

/* Notes about this file:
 *
    SQLite does not support multiple statements for each query. So instead,
    each table must have it's own statement. This isn't too bad because we only
    have 12 tables to create.

    Also, SQLite's datatype and table creation syntax is much simpler than
    regular SQL
 */

QString tableStagedFiles = QString("CREATE TABLE StagedFiles"
    "StagedFileRowID INTEGER PRIMARY KEY,"
    "ObjectType TEXT,"
    "ObjectRowID INTEGER,"
    "FileSize INTEGER,"
    "StagedPath TEXT,"
    "FinalDirectory TEXT,"
    "UNIQUE(ObjectRowID, ObjectType, StagedPath) )");

QString tableAnalysis = QString("CREATE TABLE Analysis ("
    "AnalysisRowID INTEGER PRIMARY KEY,"
    "StudyRowID INTEGER,"
    "PipelineRowID INTEGER,"
    "PipelineVersion INTEGER,"
    "ClusterStartDate TEXT,"
    "ClusterEndDate TEXT,"
    "StartDate TEXT,"
    "EndDate TEXT,"
    "SetupTime INTEGER,"
    "RunTime INTEGER,"
    "NumSeries INTEGER,"
    "Status TEXT,"
    "Successful INTEGER,"
    "Size INTEGER,"
    "NumFiles INTEGER,"
    "Hostname TEXT,"
    "StatusMessage TEXT,"
    "VirtualPath TEXT,"
    "UNIQUE(StudyRowID, PipelineRowID, PipelineVersion))");

QString tableDataDictionary = QString("CREATE TABLE DataDictionary ("
    "DataDictionaryRowID INTEGER PRIMARY KEY,"
    "NumFiles INTEGER,"
    "Size INTEGER,"
    "VirtualPath TEXT)");

QString tableDataDictionaryItems = QString("CREATE TABLE DataDictionaryItems ("
    "DataDictionaryItemRowID INTEGER PRIMARY KEY,"
    "DataDictionaryRowID INTEGER,"
    "VariableType TEXT,"
    "VariableName TEXT,"
    "VariableDescription TEXT,"
    "KeyValue TEXT,"
    "ExpectedTimepoints INTEGER,"
    "RangeLow REAL,"
    "RangeHigh REAL)");

QString tableDrug = QString("CREATE TABLE Drug ("
    "DrugRowID INTEGER PRIMARY KEY,"
    "SubjectRowID INTEGER,"
    "DrugName TEXT,"
    "DateStart TEXT,"
    "DateEnd TEXT,"
    "DateRecordEntry TEXT,"
    "DoseString TEXT,"
    "DoseAmount TEXT,"
    "DoseFrequency TEXT,"
    "AdministrationRoute TEXT,"
    "DrugClass TEXT,"
    "DoseKey TEXT,"
    "DoseUnit TEXT,"
    "FrequencyModifer TEXT,"
    "FrequencyValue REAL,"
    "FrequencyUnit TEXT,"
    "Description TEXT,"
    "Rater TEXT,"
    "Notes TEXT)");

QString tableExperiment = QString("CREATE TABLE Experiment ("
    "ExperimentRowID INTEGER PRIMARY KEY,"
    "ExperimentName TEXT UNIQUE,"
    "Size INTEGER DEFAULT 0,"
    "NumFiles INTEGER DEFAULT 0,"
    "VirtualPath TEXT)");

QString tableGroupAnalysis = QString("CREATE TABLE GroupAnalysis ("
    "GroupAnalysisRowID INTEGER PRIMARY KEY,"
    "GroupAnalysisName TEXT UNIQUE,"
    "Description TEXT,"
    "Datetime TEXT,"
    "NumFiles INTEGER,"
    "Size INTEGER,"
    "VirtualPath TEXT)");

QString tableMeasure = QString("CREATE TABLE Measure ("
    "MeasureRowID INTEGER PRIMARY KEY,"
    "SubjectRowID INTEGER,"
    "MeasureName TEXT,"
    "DateStart TEXT,"
    "DateEnd TEXT,"
    "InstrumentName TEXT,"
    "Rater TEXT,"
    "Notes TEXT,"
    "Value TEXT,"
    "Duration INTEGER,"
    "DateRecordEntry TEXT,"
    "Description TEXT)");

QString tablePackage = QString("CREATE TABLE Package ("
    "PackageRowID INTEGER PRIMARY KEY,"
    "Name TEXT NOT NULL UNIQUE,"
    "Description TEXT,"
    "Datetime TEXT,"
    "SubjectDirFormat TEXT DEFAULT 'orig',"
    "StudyDirFormat TEXT DEFAULT 'orig',"
    "SeriesDirFormat TEXT DEFAULT 'orig',"
    "PackageDataFormat TEXT DEFAULT 'orig',"
    "License TEXT,"
    "Readme TEXT,"
    "Changes TEXT,"
    "Notes TEXT)");

QString tableParams = QString("CREATE TABLE Params ("
    "ParamRowID INTEGER PRIMARY KEY,"
    "SeriesRowID INTEGER,"
    "ParamKey TEXT,"
    "ParamValue TEXT,"
    "UNIQUE(ParamKey, ParamValue))");

QString tablePipeline = QString("CREATE TABLE Pipeline ("
    "PipelineRowID INTEGER PRIMARY KEY,"
    "PipelineName TEXT,"
    "Description TEXT,"
    "Datetime TEXT,"
    "Level INTEGER,"
    "PrimaryScript TEXT,"
    "SecondaryScript TEXT,"
    "Version INTEGER ,"
    "CompleteFiles TEXT,"
    "DataCopyMethod TEXT,"
    "DependencyDirectory TEXT,"
    "DependencyLevel TEXT,"
    "DependencyLinkType TEXT,"
    "DirStructure TEXT,"
    "Directory TEXT,"
    "GroupName TEXT,"
    "GroupType TEXT,"
    "Notes TEXT,"
    "ResultScript TEXT,"
    "TempDir TEXT,"
    "FlagUseProfile INTEGER,"
    "FlagUseTempDir INTEGER,"
    "ClusterType TEXT,"
    "ClusterUser TEXT,"
    "ClusterQueue TEXT,"
    "ClusterSubmitHost TEXT,"
    "NumConcurrentAnalysis INTEGER,"
    "MaxWallTime INTEGER,"
    "SubmitDelay INTEGER,"
    "VirtualPath TEXT,"
    "UNIQUE(Name, Version))");

QString tablePipelineDataStep = QString("CREATE TABLE PipeplineDataStep ("
    "DataStepRowID INTEGER PRIMARY KEY,"
    "PipelineRowID INTEGER,"
    "AssociationType TEXT,"
    "BehDir TEXT,"
    "BehFormat TEXT,"
    "DataFormat TEXT,"
    "ImageType TEXT,"
    "DataLevel TEXT,"
    "Location TEXT,"
    "Modality TEXT,"
    "NumBOLDReps TEXT,"
    "NumImagesCriteria TEXT,"
    "StepOrder INTEGER,"
    "Protocol TEXT,"
    "SeriesCriteria TEXT,"
    "FlagEnabled INTEGER,"
    "FlagOptional INTEGER,"
    "FlagGzip INTEGER,"
    "FlagPreserveSeriesNum INTEGER,"
    "FlagPrimaryProtocol INTEGER,"
    "FlagUsePhaseDir INTEGER,"
    "FlagUseSeries INTEGER)");

QString tableSeries = QString("CREATE TABLE Series ("
    "SeriesRowID INTEGER PRIMARY KEY,"
    "StudyRowID INTEGER,"
    "SeriesNum INTEGER,"
    "Datetime TEXT,"
    "SeriesUID TEXT,"
    "Description TEXT,"
    "Protocol TEXT,"
    "ExperimentRowID INTEGER,"
    "Size INTEGER DEFAULT 0,"
    "NumFiles INTEGER DEFAULT 0,"
    "BehSize INTEGER DEFAULT 0,"
    "BehNumFiles INTEGER DEFAULT 0,"
    "VirtualPath TEXT,"
    "UNIQUE(StudyRowID, SeriesNum))");

QString tableStudy = QString("CREATE TABLE Study ("
    "StudyRowID INTEGER PRIMARY KEY,"
    "SubjectRowID INTEGER,"
    "StudyNumber INTEGER ,"
    "Datetime TEXT,"
    "Age REAL DEFAULT 0.0,"
    "Height REAL DEFAULT 0.0,"
    "Weight REAL DEFAULT 0.0,"
    "Modality TEXT,"
    "Description TEXT,"
    "StudyUID TEXT,"
    "VisitType TEXT,"
    "DayNumber INTEGER DEFAULT 0,"
    "Timepoint INTEGER DEFAULT 0,"
    "Equipment TEXT,"
    "VirtualPath TEXT,"
    "UNIQUE(SubjectRowID, StudyNumber))");

QString tableSubject = QString("CREATE TABLE Subject ("
    "SubjectRowID INTEGER PRIMARY KEY,"
    "ID TEXT NOT NULL UNIQUE,"
    "AltIDs TEXT,"
    "GUID TEXT,"
    "DateOfBirth TEXT,"
    "Sex TEXT,"
    "Gender TEXT,"
    "Ethnicity1 TEXT,"
    "Ethnicity2 TEXT,"
    "VirtualPath TEXT)");
