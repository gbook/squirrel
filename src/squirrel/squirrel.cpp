/* ------------------------------------------------------------------------------
  Squirrel squirrel.cpp
  Copyright (C) 2004 - 2023
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

#include "squirrel.h"
#include "squirrelImageIO.h"
#include "utils.h"

/* ------------------------------------------------------------ */
/* ----- squirrel --------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::squirrel constructor
 * @param dbg true for debug logging
 * @param q true to turn off all output
 */
squirrel::squirrel(bool dbg, bool q)
{
    datetime = QDateTime::currentDateTime();
    description = "Uninitialized squirrel package";
    name = "Squirrel package";
    version = QString("%1.%2").arg(SQUIRREL_VERSION_MAJ).arg(SQUIRREL_VERSION_MIN);
    format = "squirrel";
    subjectDirFormat = "orig";
    studyDirFormat = "orig";
    seriesDirFormat = "orig";
    dataFormat = "nifti4dgz";
    isOkToDelete = true;
    debug = dbg;
    quiet = q;

    MakeTempDir(workingDir);
    Log(QString("Created squirrel object. Working dir [%1]").arg(workingDir), __FUNCTION__);
}


/* ------------------------------------------------------------ */
/* ----- ~squirrel -------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::~squirrel
 */
squirrel::~squirrel()
{
    if (isValid && (workingDir.size() > 20)) {
        QString m;
        RemoveDir(workingDir, m);
        Log(QString("Removed working directory [%1]. Message [%2]").arg(workingDir).arg(m), __FUNCTION__);
    }
    Log("Deleting squirrel object", __FUNCTION__);
}


/* ------------------------------------------------------------ */
/* ----- read ------------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Reads a squirrel package from disk
 * @param filepath Full filepath of the package
 * @param headerOnly true if only reading the header
 * @param validateOnly true if validating the package
 * @return
 */
bool squirrel::read(QString filepath, bool headerOnly, bool validateOnly) {

    /* set the package path */
    filePath = filepath;

    if (validateOnly)
        Log(QString("Validating [%1]").arg(filepath), __FUNCTION__);
    else
        Log(QString("Reading squirrel file [%1]. Using working directory [%2]").arg(filepath).arg(workingDir), __FUNCTION__);

    /* check if file exists */
    if (!FileExists(filepath)) {
        Log(QString("File %1 does not exist").arg(filepath), __FUNCTION__);
        return false;
    }

    /* get listing of the zip the file, check if the squirrel.json exists in the root */
    QString systemstring;
    #ifdef Q_OS_WINDOWS
        systemstring = QString("\"C:/Program Files/7-Zip/7z.exe\" l \"" + filepath + "\"");
    #else
        systemstring = "unzip -l " + filepath;
    #endif
    QString output = SystemCommand(systemstring, true);
    Log(output, __FUNCTION__);
    if (!output.contains("squirrel.json")) {
        Log(QString("File " + filepath + " does not appear to be a squirrel package"), __FUNCTION__);
        return false;
    }

    /* get the header .json file (either by unzipping or extracting only the file) */
    QString jsonStr;
    if (headerOnly) {
        //Print("Reading header only");
        #ifdef Q_OS_WINDOWS
            systemstring = QString("\"C:/Program Files/7-Zip/7z.exe\" x \"" + filepath + "\" -o\"" + workingDir + "\" squirrel.json -y");
            Log(systemstring, __FUNCTION__, true);
            output = SystemCommand(systemstring, true);
            /* read from .json file */
            jsonStr = ReadTextFileToString(workingDir + "/squirrel.json");
            //Print(jsonStr);
        #else
            systemstring = QString("unzip -p " + filepath + " squirrel.json");
            output = SystemCommand(systemstring, true);
        #endif
    }
    else {
        /* unzip the .zip to the working dir */
        #ifdef Q_OS_WINDOWS
            systemstring = QString("\"C:/Program Files/7-Zip/7z.exe\" x \"" + filepath + "\" -o\"" + workingDir + "\" -y");
        #else
            systemstring = QString("unzip " + filepath + " -d " + workingDir);
        #endif
        output = SystemCommand(systemstring, true);
        Log(output, __FUNCTION__, true);

        /* read from .json file */
        jsonStr = ReadTextFileToString(workingDir + "/squirrel.json");
    }

    /* get the JSON document and root object */
    QJsonDocument d = QJsonDocument::fromJson(jsonStr.toUtf8());
    QJsonObject root = d.object();

    /* get the package info */
    QJsonValue pkgVal = root.value("package");
    QJsonObject pkgObj = pkgVal.toObject();
    description = pkgObj["description"].toString();
    datetime.fromString(pkgObj["datetime"].toString());
    name = pkgObj["name"].toString();

    /* get the data object, and check for any subjects */
    QJsonArray jsonSubjects;
    if (root.contains("data")) {
        QJsonValue dataVal = root.value("data");
        QJsonObject dataObj = dataVal.toObject();
        jsonSubjects = dataObj["subjects"].toArray();
        Log(QString("Found [%1] subjects").arg(jsonSubjects.size()), __FUNCTION__);
    }
    else if (root.contains("subjects")) {
        jsonSubjects = root["subjects"].toArray();
        Log(QString("NOTICE: Found [%1] subjects in the root of the JSON. (This is a slightly malformed squirrel file, but I'll accept it)").arg(jsonSubjects.size()), __FUNCTION__);
    }

    /* loop through and read any subjects */
    for (auto v : jsonSubjects) {
        QJsonObject jsonSubject = v.toObject();

        squirrelSubject sqrlSubject;

        sqrlSubject.ID = jsonSubject["SubjectID"].toString();
        sqrlSubject.alternateIDs = jsonSubject["AlternateIDs"].toVariant().toStringList();
        sqrlSubject.GUID = jsonSubject["GUID"].toString();
        sqrlSubject.dateOfBirth.fromString(jsonSubject["DateOfBirth"].toString(), "yyyy-MM-dd");
        sqrlSubject.sex = jsonSubject["Sex"].toString();
        sqrlSubject.gender = jsonSubject["Gender"].toString();
        sqrlSubject.ethnicity1 = jsonSubject["Ethnicity1"].toString();
        sqrlSubject.ethnicity2 = jsonSubject["Ethnicity2"].toString();
        sqrlSubject.virtualPath = jsonSubject["VirtualPath"].toString();

        Log(QString("Reading subject [%1]").arg(sqrlSubject.ID), __FUNCTION__);

        /* loop through and read all studies */
        QJsonArray jsonStudies = jsonSubject["studies"].toArray();
        for (auto v : jsonStudies) {
            QJsonObject jsonStudy = v.toObject();
            squirrelStudy sqrlStudy;

            sqrlStudy.number = jsonStudy["StudyNumber"].toInt();
            sqrlStudy.dateTime.fromString(jsonStudy["StudyDatetime"].toString(), "yyyy-MM-dd hh:mm:ss");
            sqrlStudy.ageAtStudy = jsonStudy["AgeAtStudy"].toDouble();
            sqrlStudy.height = jsonStudy["Height"].toDouble();
            sqrlStudy.weight = jsonStudy["Weight"].toDouble();
            sqrlStudy.modality = jsonStudy["Modality"].toString();
            sqrlStudy.description = jsonStudy["Description"].toString();
            sqrlStudy.studyUID = jsonStudy["StudyUID"].toString();
            sqrlStudy.visitType = jsonStudy["VisitType"].toString();
            sqrlStudy.dayNumber = jsonStudy["DayNumber"].toString();
            sqrlStudy.timePoint = jsonStudy["TimePoint"].toString();
            sqrlStudy.equipment = jsonStudy["Equipment"].toString();
            sqrlStudy.virtualPath = jsonStudy["VirtualPath"].toString();

            /* loop through and read all series */
            QJsonArray jsonSeries = jsonStudy["series"].toArray();
            for (auto v : jsonSeries) {
                QJsonObject jsonSeries = v.toObject();
                squirrelSeries sqrlSeries;

                sqrlSeries.number = jsonSeries["SeriesNumber"].toInteger();
                sqrlSeries.dateTime.fromString(jsonSeries["SeriesDatetime"].toString(), "yyyy-MM-dd hh:mm:ss");
                sqrlSeries.seriesUID = jsonSeries["SeriesUID"].toString();
                sqrlSeries.description = jsonSeries["Description"].toString();
                sqrlSeries.protocol = jsonSeries["Protocol"].toString();
                sqrlSeries.experimentNames = jsonSeries["ExperimentNames"].toString().split(",");
                sqrlSeries.size = jsonSeries["Size"].toInteger();
                sqrlSeries.numFiles = jsonSeries["NumFiles"].toInteger();
                sqrlSeries.behSize = jsonSeries["BehSize"].toInteger();
                sqrlSeries.numBehFiles = jsonSeries["BehNumFiles"].toInteger();
                sqrlSeries.virtualPath = jsonSeries["VirtualPath"].toString();

                /* read any params from the data/Subject/Study/Series/params.json file */
                if (!headerOnly)
                    sqrlSeries.params = ReadParamsFile(QString("%1/data/%2/%3/%4/params.json").arg(workingDir).arg(sqrlSubject.ID).arg(sqrlStudy.number).arg(sqrlSeries.number));

                /* add this series to the study */
                if (sqrlStudy.addSeries(sqrlSeries)) {
                    Log(QString("Added series [%1]").arg(sqrlSeries.number), __FUNCTION__);
                }
            }

            /* loop through and read all analyses */
            QJsonArray jsonAnalyses = jsonStudy["analyses"].toArray();
            for (auto v : jsonAnalyses) {
                QJsonObject jsonAnalyses = v.toObject();
                squirrelAnalysis sqrlAnalysis;

                sqrlAnalysis.pipelineName = jsonAnalyses["PipelineName"].toString();
                sqrlAnalysis.pipelineVersion = jsonAnalyses["PipelineVersion"].toInt();
                sqrlAnalysis.clusterStartDate.fromString(jsonAnalyses["ClusterStartDate"].toString(), "yyyy-MM-dd hh:mm:ss");
                sqrlAnalysis.clusterEndDate.fromString(jsonAnalyses["ClusterEndDate"].toString(), "yyyy-MM-dd hh:mm:ss");
                sqrlAnalysis.startDate.fromString(jsonAnalyses["StartDate"].toString(), "yyyy-MM-dd hh:mm:ss");
                sqrlAnalysis.endDate.fromString(jsonAnalyses["EndDate"].toString(), "yyyy-MM-dd hh:mm:ss");
                sqrlAnalysis.setupTime = jsonAnalyses["RunTime"].toInteger();
                sqrlAnalysis.runTime = jsonAnalyses["RunTime"].toInteger();
                sqrlAnalysis.numSeries = jsonAnalyses["NumSeries"].toInt();
                sqrlAnalysis.status = jsonAnalyses["Status"].toString();
                sqrlAnalysis.successful = jsonAnalyses["Successful"].toBool();
                sqrlAnalysis.size = jsonAnalyses["Size"].toInteger();
                sqrlAnalysis.hostname = jsonAnalyses["Hostname"].toString();
                sqrlAnalysis.status = jsonAnalyses["Status"].toString();
                sqrlAnalysis.lastMessage = jsonAnalyses["StatusMessage"].toString();
                sqrlAnalysis.virtualPath = jsonAnalyses["VirtualPath"].toString();

                /* add this analysis to the study */
                if (sqrlStudy.addAnalysis(sqrlAnalysis)) {
                    Log(QString("Added analysis [%1]").arg(sqrlAnalysis.pipelineName), __FUNCTION__);
                }
            }

            /* add this study to the subject */
            if (sqrlSubject.addStudy(sqrlStudy)) {
                Log(QString("Added study [%1]").arg(sqrlStudy.number), __FUNCTION__);
            }
        }

        /* read all measures */
        QJsonArray jsonMeasures = jsonSubject["measures"].toArray();
        Log(QString("Reading [%1] measures").arg(jsonMeasures.size()), __FUNCTION__);
        for (auto v : jsonMeasures) {
            QJsonObject jsonMeasure = v.toObject();
            squirrelMeasure sqrlMeasure;

            sqrlMeasure.measureName = jsonMeasure["MeasureName"].toString();
            sqrlMeasure.dateStart.fromString(jsonMeasure["DateStart"].toString(), "yyyy-MM-dd hh:mm:ss");
            sqrlMeasure.dateEnd.fromString(jsonMeasure["DateEnd"].toString(), "yyyy-MM-dd hh:mm:ss");
            sqrlMeasure.instrumentName = jsonMeasure["InstrumentName"].toString();
            sqrlMeasure.rater = jsonMeasure["Rater"].toString();
            sqrlMeasure.notes = jsonMeasure["Notes"].toString();
            sqrlMeasure.value = jsonMeasure["Value"].toString();
            sqrlMeasure.description = jsonMeasure["Description"].toString();
            sqrlMeasure.duration = jsonMeasure["Duration"].toDouble();

            sqrlSubject.addMeasure(sqrlMeasure);
        }

        /* read all drugs */
        QJsonArray jsonDrugs = jsonSubject["drugs"].toArray();
        Log(QString("Reading [%1] drugs").arg(jsonDrugs.size()), __FUNCTION__);
        for (auto v : jsonDrugs) {
            QJsonObject jsonDrug = v.toObject();
            squirrelDrug sqrlDrug;

            sqrlDrug.drugName = jsonDrug["DrugName"].toString();
            sqrlDrug.dateStart.fromString(jsonDrug["DateStart"].toString(), "yyyy-MM-dd hh:mm:ss");
            sqrlDrug.dateEnd.fromString(jsonDrug["DateEnd"].toString(), "yyyy-MM-dd hh:mm:ss");
            sqrlDrug.doseString = jsonDrug["DoseString"].toString();
            sqrlDrug.doseAmount = jsonDrug["DoseAmount"].toDouble();
            sqrlDrug.doseFrequency = jsonDrug["DoseFrequency"].toString();
            sqrlDrug.route = jsonDrug["AdministrationRoute"].toString();
            sqrlDrug.drugClass = jsonDrug["DrugClass"].toString();
            sqrlDrug.doseKey = jsonDrug["DoseKey"].toString();
            sqrlDrug.doseUnit = jsonDrug["DoseUnit"].toString();
            sqrlDrug.frequencyModifier = jsonDrug["FrequencyModifier"].toString();
            sqrlDrug.frequencyValue = jsonDrug["FrequencyValue"].toDouble();
            sqrlDrug.frequencyUnit = jsonDrug["FrequencyUnit"].toString();
            sqrlDrug.description = jsonDrug["Description"].toString();
            sqrlDrug.rater = jsonDrug["Rater"].toString();
            sqrlDrug.notes = jsonDrug["Notes"].toString();
            sqrlDrug.dateRecordEntry.fromString(jsonDrug["DateRecordEntry"].toString(), "yyyy-MM-dd hh:mm:ss");

            sqrlSubject.addDrug(sqrlDrug);
        }

        /* add the subject */
        if (addSubject(sqrlSubject)) {
            Log(QString("Added subject [" + sqrlSubject.ID + "]"), __FUNCTION__);
        }
    }

    /* read all experiments */
    QJsonArray jsonExperiments;
    jsonExperiments = root["experiments"].toArray();
    Log(QString("Reading [%1] experiments").arg(jsonExperiments.size()), __FUNCTION__);
    for (auto v : jsonExperiments) {
        QJsonObject jsonExperiment = v.toObject();
        squirrelExperiment sqrlExperiment;

        sqrlExperiment.experimentName = jsonExperiment["ExperimentName"].toString();
        sqrlExperiment.numFiles = jsonExperiment["NumFiles"].toInt();
        sqrlExperiment.size = jsonExperiment["Size"].toInt();
        sqrlExperiment.virtualPath = jsonExperiment["VirtualPath"].toString();

        experimentList.append(sqrlExperiment);
    }

    /* read all pipelines */
    QJsonArray jsonPipelines;
    jsonPipelines = root["pipelines"].toArray();
    Log(QString("Reading [%1] pipelines").arg(jsonPipelines.size()), __FUNCTION__);
    for (auto v : jsonPipelines) {
        QJsonObject jsonPipeline = v.toObject();
        squirrelPipeline sqrlPipeline;

        sqrlPipeline.clusterType = jsonPipeline["ClusterType"].toString();
        sqrlPipeline.clusterUser = jsonPipeline["ClusterUser"].toString();
        sqrlPipeline.clusterQueue = jsonPipeline["ClusterQueue"].toString();
        sqrlPipeline.clusterSubmitHost = jsonPipeline["ClusterSubmitHost"].toString();
        sqrlPipeline.createDate.fromString(jsonPipeline["CreateDate"].toString(), "yyyy-MM-dd hh:mm:ss");
        sqrlPipeline.dataCopyMethod = jsonPipeline["DataCopyMethod"].toString();
        sqrlPipeline.depDir = jsonPipeline["DepDir"].toString();
        sqrlPipeline.depLevel = jsonPipeline["DepLevel"].toString();
        sqrlPipeline.depLinkType = jsonPipeline["DepLinkType"].toString();
        sqrlPipeline.description = jsonPipeline["Description"].toString();
        sqrlPipeline.dirStructure = jsonPipeline["DirStructure"].toString();
        sqrlPipeline.directory = jsonPipeline["Directory"].toString();
        sqrlPipeline.group = jsonPipeline["Group"].toString();
        sqrlPipeline.groupType = jsonPipeline["GroupType"].toString();
        sqrlPipeline.level = jsonPipeline["Level"].toString();
        sqrlPipeline.maxWallTime = jsonPipeline["MaxWallTime"].toInt();
        sqrlPipeline.pipelineName = jsonPipeline["PipelineName"].toString();
        sqrlPipeline.notes = jsonPipeline["Notes"].toString();
        sqrlPipeline.numConcurrentAnalyses = jsonPipeline["NumConcurrentAnalyses"].toInt();
        sqrlPipeline.parentPipelines = jsonPipeline["ParentPipelines"].toString().split(",");
        sqrlPipeline.resultScript = jsonPipeline["ResultScript"].toString();
        sqrlPipeline.submitDelay = jsonPipeline["SubmitDelay"].toInt();
        sqrlPipeline.tmpDir = jsonPipeline["TempDir"].toString();
        sqrlPipeline.flags.useProfile = jsonPipeline["UseProfile"].toBool();
        sqrlPipeline.flags.useTmpDir = jsonPipeline["UseTempDir"].toBool();
        sqrlPipeline.version = jsonPipeline["Version"].toInt();
        sqrlPipeline.primaryScript = jsonPipeline["PrimaryScript"].toString();
        sqrlPipeline.secondaryScript = jsonPipeline["SecondaryScript"].toString();
        sqrlPipeline.virtualPath = jsonPipeline["VirtualPath"].toString();

        QJsonArray jsonCompleteFiles;
        jsonCompleteFiles = jsonPipeline["CompleteFiles"].toArray();
        for (auto v : jsonCompleteFiles) {
            sqrlPipeline.completeFiles.append(v.toString());
        }

        /* read the pipeline data steps */
        QJsonArray jsonDataSteps;
        jsonDataSteps = jsonPipeline["dataSteps"].toArray();
        for (auto v : jsonDataSteps) {
            QJsonObject jsonDataStep = v.toObject();
            dataStep ds;
            ds.associationType = jsonDataStep["AssociationType"].toString();
            ds.behDir = jsonDataStep["BehDir"].toString();
            ds.behFormat = jsonDataStep["BehFormat"].toString();
            ds.dataFormat = jsonDataStep["DataFormat"].toString();
            ds.imageType = jsonDataStep["ImageType"].toString();
            ds.datalevel = jsonDataStep["DataLevel"].toString();
            ds.location = jsonDataStep["Location"].toString();
            ds.modality = jsonDataStep["Modality"].toString();
            ds.numBOLDreps = jsonDataStep["NumBOLDreps"].toString();
            ds.numImagesCriteria = jsonDataStep["NumImagesCriteria"].toString();
            ds.order = jsonDataStep["Order"].toInt();
            ds.protocol = jsonDataStep["Protocol"].toString();
            ds.seriesCriteria = jsonDataStep["SeriesCriteria"].toString();
            ds.protocol = jsonDataStep["Protocol"].toString();
            ds.flags.enabled = jsonDataStep["Enabled"].toBool();
            ds.flags.optional = jsonDataStep["Optional"].toBool();
            ds.flags.gzip = jsonDataStep["Gzip"].toBool();
            ds.flags.usePhaseDir = jsonDataStep["UsePhaseDir"].toBool();
            ds.flags.useSeries = jsonDataStep["UseSeries"].toBool();
            ds.flags.preserveSeries = jsonDataStep["PreserveSeries"].toBool();
            ds.flags.primaryProtocol = jsonDataStep["PrimaryProtocol"].toBool();
            sqrlPipeline.dataSteps.append(ds);
        }
        pipelineList.append(sqrlPipeline);
    }

    /* If we're only validating: delete the tmpdir if it exists */
    if (validateOnly) {
        if (DirectoryExists(workingDir)) {
            Log(QString("Temporary export dir [" + workingDir + "] exists and will be deleted"), __FUNCTION__);
            QString m;
            if (!RemoveDir(workingDir, m))
                Log(QString("Error [" + m + "] removing directory [" + workingDir + "]"), __FUNCTION__);
        }
    }

    return true;
}


/* ------------------------------------------------------------ */
/* ----- write ------------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::write Writes a squirrel package using stored information
 * @param outpath full path to the output squirrel .zip file
 * @param dataFormat if converting from DICOM, write the data in the specified format
 *                   - 'orig' - Perform no conversion of DICOM images (not recommended as it retains PHI)
 *                   - 'anon' - Anonymize DICOM files (light anonymization: remove PHI, but not ID or dates)
 *                   - 'anonfull' - Anonymize DICOM files (full anonymization)
 *                   - 'nifti4d' - Attempt to convert any convertable images to Nifti 4D
 *                   - 'nifti4dgz' - Attempt to convert any convertable images to Nifti 4D gzip [DEFAULT]
 *                   - 'nidti3d' - Attempt to convert any convertable images to Nifti 3D
 *                   - 'nifti3dgz' - Attempt to convert any convertable images to Nifti 3D gzip
 * @param subjectDirFormat directory structure of the subject data
 *                  - 'orig' - Use the subjectID for subject directories [DEFAULT]
 *                  - 'seq' - Use sequentially generated numbers for subject directories
 * @param studyDirFormat directory structure of the subject data
 *                  - 'orig' - Use the studyNum for study directories [DEFAULT]
 *                  - 'seq' - Use sequentially generated numbers for study directories
 * @param seriesDirFormat directory structure of the subject data
 *                  - 'orig' - Use the seriesNum for series directories [DEFAULT]
 *                  - 'seq' - Use sequentially generated numbers for series directories
 * @return true if package was successfully written, false otherwise
 */
bool squirrel::write(QString outpath, QString &filepath) {

    /* create the log file */
    QFileInfo finfo(outpath);
    logfile = QString(finfo.absolutePath() + "/squirrel-" + CreateLogDate() + ".log");

    Log(QString("Writing squirrel package. Working dir [%1]. Outpath [%2]. logfile [%3]").arg(workingDir).arg(outpath).arg(logfile), __FUNCTION__);

    /* ----- 1) write data. And set the relative paths in the objects ----- */
    /* iterate through subjects */
    for (int i=0; i < subjectList.size(); i++) {

        squirrelSubject sub = subjectList[i];

        QString subjDir;
        if (subjectDirFormat == "orig") {
            subjDir = sub.ID;
        }
        else {
            subjDir = QString("%1").arg(i+1); /* start the numbering at 1 instead of 0 */
        }
        Log(QString("Writing subject [%1]").arg(subjDir), __FUNCTION__, true);

        subjDir.replace(QRegularExpression("[^a-zA-Z0-9 _-]", QRegularExpression::CaseInsensitiveOption), "");
        QString vPath = QString("data/%1").arg(subjDir);
        subjectList[i].virtualPath = vPath;

        /* iterate through studies */
        for (int j=0; j < sub.studyList.size(); j++) {

            squirrelStudy stud = sub.studyList[j];

            QString studyDir;
            if (studyDirFormat == "orig")
                studyDir = QString("%1").arg(stud.number);
            else
                studyDir = QString("%1").arg(j+1); /* start the numbering at 1 instead of 0 */

            studyDir.replace(QRegularExpression("[^a-zA-Z0-9 _-]", QRegularExpression::CaseInsensitiveOption), "");
            QString vPath = QString("data/%1/%2").arg(subjDir).arg(studyDir);
            subjectList[i].studyList[j].virtualPath = vPath;

            Log(QString("Writing study [%1]").arg(studyDir), __FUNCTION__);

            /* iterate through series */
            for (int k=0; k < stud.seriesList.size(); k++) {

                squirrelSeries ser = stud.seriesList[k];

                QString seriesDir;
                if (seriesDirFormat == "orig")
                    seriesDir = QString("%1").arg(ser.number);
                else
                    seriesDir = QString("%1").arg(k+1); /* start the numbering at 1 instead of 0 */

                seriesDir.replace(QRegularExpression("[^a-zA-Z0-9 _-]", QRegularExpression::CaseInsensitiveOption), "");
                QString vPath = QString("data/%1/%2/%3").arg(subjDir).arg(studyDir).arg(seriesDir);
                subjectList[i].studyList[j].seriesList[k].virtualPath = vPath;

                QString m;
                QString seriesPath = QString("%1/%2").arg(workingDir).arg(subjectList[i].studyList[j].seriesList[k].virtualPath);
                MakePath(seriesPath,m);

                Log(QString("Writing series [%1]. Data format [%2]").arg(seriesDir).arg(dataFormat), __FUNCTION__);

                /* orig vs other formats */
                if (dataFormat == "orig") {
                    /* copy all of the series files to the temp directory */
                    foreach (QString f, ser.stagedFiles) {
                        QString systemstring = QString("cp -uv %1 %2").arg(f).arg(seriesPath);
                        SystemCommand(systemstring);
                    }
                }
                else if ((dataFormat == "anon") || (dataFormat == "anonfull")) {
                    /* create temp directory */
                    QString td;
                    MakeTempDir(td);

                    /* copy all files to temp directory */
                    QString systemstring;
                    foreach (QString f, ser.stagedFiles) {
                        systemstring = QString("cp -uv %1 %2").arg(f).arg(td);
                        SystemCommand(systemstring);
                    }

                    /* anonymize the directory */
                    squirrelImageIO io;
                    QString m;
                    if (dataFormat == "anon")
                        io.AnonymizeDir(td,1,"Anonymized","Anonymized",m);
                    else
                        io.AnonymizeDir(td,2,"Anonymized","Anonymized",m);

                    /* move the anonymized files to the staging area */
                    systemstring = QString("mv %1/* %2/").arg(td).arg(seriesPath);
                    SystemCommand(systemstring);

                    /* delete temp directory */
                    QString m2;
                    RemoveDir(td, m2);
                }
                else if (dataFormat.contains("nifti")) {
                    int numConv(0), numRename(0);
                    bool gzip;
                    if (dataFormat.contains("gz"))
                        gzip = true;
                    else
                        gzip = false;

                    /* get path of first file to be converted */
                    if (ser.stagedFiles.size() > 0) {
                        Log(QString("Converting [%1] files to nifti").arg(ser.stagedFiles.size()), __FUNCTION__, true);

                        QFileInfo f(ser.stagedFiles[0]);
                        QString origSeriesPath = f.absoluteDir().absolutePath();
                        squirrelImageIO io;
                        QString m3;
                        io.ConvertDicom(dataFormat, origSeriesPath, seriesPath, QDir::currentPath(), gzip, subjDir, studyDir, seriesDir, "dicom", numConv, numRename, m3);
                        Log(QString("ConvertDicom() returned [%1]").arg(m3), __FUNCTION__, true);
                    }
                    else {
                        Log(QString("Variable squirrelSeries.stagedFiles is empty. No files to convert to Nifti"), __FUNCTION__, true);
                    }
                }
                else
                    Log(QString("dataFormat [%1] not recognized").arg(dataFormat), __FUNCTION__);

                /* get the number of files and size of the series */
                qint64 c(0), b(0);
                //Log(QString("Running GetDirSizeAndFileCount() on [%1]").arg(seriesPath), __FUNCTION__);
                GetDirSizeAndFileCount(seriesPath, c, b, false);
                //Log(QString("GetDirSizeAndFileCount() found  [%1] files   [%2] bytes").arg(c).arg(b), __FUNCTION__);
                subjectList[i].studyList[j].seriesList[k].numFiles = c;
                subjectList[i].studyList[j].seriesList[k].size = b;

                /* write the series .json file, containing the dicom header params */
                QJsonObject params;
                params = ser.ParamsToJSON();
                QByteArray j = QJsonDocument(params).toJson();
                QFile fout(QString("%1/params.json").arg(seriesPath));
                if (fout.open(QIODevice::WriteOnly)) {
                    fout.write(j);
                    Log(QString("Wrote %1/params.json").arg(seriesPath), __FUNCTION__, true);
                }
                else {
                    Log(QString("Error writing [%1]").arg(fout.fileName()), __FUNCTION__);
                    Log(QString("Error writing %1/params.json").arg(seriesPath), __FUNCTION__);
                }
            }
        }
    }

    /* ----- 2) write .json file ----- */
    Log("Creating JSON file...", __FUNCTION__);
    /* create JSON object */
    QJsonObject root;

    QJsonObject pkgInfo;
    pkgInfo["PackageName"] = name;
    pkgInfo["Description"] = description;
    pkgInfo["Datetime"] = CreateCurrentDateTime(2);
    pkgInfo["PackageFormat"] = format;
    pkgInfo["SquirrelVersion"] = version;

    root["package"] = pkgInfo;

    QJsonObject data;
    QJsonArray JSONsubjects;
    /* add subjects */
    for (int i=0; i < subjectList.size(); i++) {
        JSONsubjects.append(subjectList[i].ToJSON());
    }

    /* add group-analyses */
    if (groupAnalysisList.size() > 0) {
        Log(QString("Adding [%1] group-analyses...").arg(groupAnalysisList.size()), __FUNCTION__);
        QJsonArray JSONgroupanalyses;
        for (int i=0; i < groupAnalysisList.size(); i++) {
            JSONgroupanalyses.append(groupAnalysisList[i].ToJSON());
            Log(QString("Added group-analysis [%1]").arg(groupAnalysisList[i].groupAnalysisName), __FUNCTION__, true);
        }
        data["NumGroupAnalyses"] = JSONgroupanalyses.size();
        data["group-analysis"] = JSONgroupanalyses;
    }

    data["NumSubjects"] = JSONsubjects.size();
    data["subjects"] = JSONsubjects;
    root["data"] = data;

    /* add pipelines */
    if (pipelineList.size() > 0) {
        Log(QString("Adding [%1] pipelines...").arg(pipelineList.size()), __FUNCTION__);
        QJsonArray JSONpipelines;
        for (int i=0; i < pipelineList.size(); i++) {
            JSONpipelines.append(pipelineList[i].ToJSON(workingDir));
            Log(QString("Added pipeline [%1]").arg(pipelineList[i].pipelineName), __FUNCTION__, true);
        }
        root["NumPipelines"] = JSONpipelines.size();
        root["pipelines"] = JSONpipelines;
    }

    /* add experiments */
    if (experimentList.size() > 0) {
        Log(QString("Adding [%1] experiments...").arg(experimentList.size()), __FUNCTION__);
        QJsonArray JSONexperiments;
        for (int i=0; i < experimentList.size(); i++) {
            JSONexperiments.append(experimentList[i].ToJSON());
            Log(QString("Added experiment [%1]").arg(experimentList[i].experimentName), __FUNCTION__, true);
        }
        root["NumExperiments"] = JSONexperiments.size();
        root["experiments"] = JSONexperiments;
    }

    /* add data-dictionary */
    if (dataDictionary.dictItems.size() > 0) {
        Log(QString("Adding [%1] data-dictionary...").arg(groupAnalysisList.size()), __FUNCTION__);
        root["data-dictionary"] = dataDictionary.ToJSON();
    }

    /* write the final .json file */
    QByteArray j = QJsonDocument(root).toJson();
    QFile fout(QString("%1/squirrel.json").arg(workingDir));
    fout.open(QIODevice::WriteOnly);
    fout.write(j);
    fout.close();

    Log(QString("Wrote main JSON file [%1/squirrel.json]").arg(workingDir), __FUNCTION__);

    /* zip the temp directory into the output file */
    QString zipfile = outpath;
    if (!zipfile.endsWith(".zip"))
        zipfile += ".zip";

    QString systemstring;
    #ifdef Q_OS_WINDOWS
        systemstring = QString("\"C:/Program Files/7-Zip/7z.exe\" a \"" + zipfile + "\" \"" + workingDir + "/*\"");
    #else
        systemstring = "zip -1rv " + zipfile + ".";
    #endif

    Log("Beginning zipping package...", __FUNCTION__);
    SystemCommand(systemstring, false);
    Log("Finished zipping package...", __FUNCTION__);

    if (FileExists(zipfile)) {
        Log("Created .zip file [" + zipfile + "]", __FUNCTION__);
        filepath = zipfile;

        /* delete the tmp dir, if it exists */
        if (DirectoryExists(workingDir)) {
            Log("Temporary export dir [" + workingDir + "] exists and will be deleted", __FUNCTION__);
            QString m;
            if (!RemoveDir(workingDir, m))
                Log("Error [" + m + "] removing directory [" + workingDir + "]", __FUNCTION__);
        }
    }
    else {
        Log("Error creating zip file [" + zipfile + "]", __FUNCTION__);
        return false;
    }

    return true;
}


/* ------------------------------------------------------------ */
/* ----- validate --------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Validate if a squirrel package is readable
 * @return true if valid squirrel file, false otherwise
 */
bool squirrel::validate() {

    return true;
}


/* ------------------------------------------------------------ */
/* ----- print ------------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief Print the details of a package, including all objects
 */
void squirrel::print() {

    /* print package info */
    PrintPackage();

    /* iterate through subjects */
    for (int i=0; i < subjectList.size(); i++) {
        squirrelSubject sub = subjectList[i];
        sub.PrintSubject();

        /* iterate through studies */
        for (int j=0; j < sub.studyList.size(); j++) {
            squirrelStudy stud = sub.studyList[j];
            stud.PrintStudy();

            /* iterate through series */
            for (int k=0; k < stud.seriesList.size(); k++) {
                squirrelSeries ser = stud.seriesList[k];
                ser.PrintSeries();
            }

            /* iterate through analyses */
            for (int k=0; k < stud.analysisList.size(); k++) {
                squirrelAnalysis an = stud.analysisList[k];
                an.PrintAnalysis();
            }
        }

        /* iterate through measures */
        for (int j=0; j < sub.measureList.size(); j++) {
            squirrelMeasure meas = sub.measureList[j];
            meas.PrintMeasure();
        }

        /* iterate through drugs */
        for (int j=0; j < sub.drugList.size(); j++) {
            squirrelDrug drug = sub.drugList[j];
            drug.PrintDrug();
        }
    }

    /* iterate through pipelines */
    for (int i=0; i < pipelineList.size(); i++) {
        squirrelPipeline pipe = pipelineList[i];
        pipe.PrintPipeline();
    }

    /* iterate through experiments */
    for (int i=0; i < experimentList.size(); i++) {
        squirrelExperiment exp = experimentList[i];
        exp.PrintExperiment();
    }
}


/* ------------------------------------------------------------ */
/* ----- GetUnzipSize ----------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Get the unzipped size of the squirrel package in bytes
 * @return unzipped size of the squirrel package in bytes
 */
qint64 squirrel::GetUnzipSize() {

    qint64 unzipSize(0);

    /* iterate through subjects */
    for (int i=0; i < subjectList.size(); i++) {
        squirrelSubject sub = subjectList[i];
        /* iterate through studies */
        for (int j=0; j < sub.studyList.size(); j++) {
            squirrelStudy stud = sub.studyList[j];
            /* iterate through series */
            for (int k=0; k < stud.seriesList.size(); k++) {
                squirrelSeries ser = stud.seriesList[k];
                unzipSize += ser.size;
            }
            /* iterate through analyses */
            for (int k=0; k < stud.analysisList.size(); k++) {
                squirrelAnalysis an = stud.analysisList[k];
                unzipSize += an.size;
            }
        }
    }
    return unzipSize;
}


/* ------------------------------------------------------------ */
/* ----- GetNumFiles ------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief Get the number of files in the squirrel package
 * @return total number of files in the package
 */
qint64 squirrel::GetNumFiles() {

    qint64 numFiles(0);

    /* iterate through subjects */
    for (int i=0; i < subjectList.size(); i++) {
        squirrelSubject sub = subjectList[i];
        /* iterate through studies */
        for (int j=0; j < sub.studyList.size(); j++) {
            squirrelStudy stud = sub.studyList[j];
            /* iterate through series */
            for (int k=0; k < stud.seriesList.size(); k++) {
                squirrelSeries ser = stud.seriesList[k];
                numFiles += ser.numFiles;
            }
        }
    }
    return numFiles;
}


/* ------------------------------------------------------------ */
/* ----- GetNumSubjects --------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::GetNumSubjects
 * @return the number of subjects
 */
qint64 squirrel::GetNumSubjects() {
    return subjectList.size();
}


/* ------------------------------------------------------------ */
/* ----- GetNumStudies ---------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::GetNumStudies
 * @return the total number of studies in the package
 */
qint64 squirrel::GetNumStudies() {

    qint64 numStudies(0);

    /* iterate through subjects */
    for (int i=0; i < subjectList.size(); i++) {
        numStudies += subjectList[i].studyList.size();
    }
    return numStudies;
}


/* ------------------------------------------------------------ */
/* ----- GetNumSeries ----------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::GetNumSeries
 * @return the total number of series in the package
 */
qint64 squirrel::GetNumSeries() {
    qint64 numSeries(0);

    /* iterate through subjects */
    for (int i=0; i < subjectList.size(); i++) {
        squirrelSubject sub = subjectList[i];
        /* iterate through studies */
        for (int j=0; j < sub.studyList.size(); j++) {
            numSeries += sub.studyList[j].seriesList.size();
        }
    }
    return numSeries;
}


/* ------------------------------------------------------------ */
/* ----- GetNumMeasures --------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::GetNumMeasures
 * @return the total number of measure objects in the package
 */
qint64 squirrel::GetNumMeasures() {
    qint64 numMeasures(0);

    /* iterate through subjects */
    for (int i=0; i < subjectList.size(); i++) {
        numMeasures += subjectList[i].measureList.size();
    }

    return numMeasures;
}


/* ------------------------------------------------------------ */
/* ----- GetNumDrugs ------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::GetNumDrugs
 * @return the totel number of drug objects in the package
 */
qint64 squirrel::GetNumDrugs() {
    qint64 numDrugs(0);

    /* iterate through subjects */
    for (int i=0; i < subjectList.size(); i++) {
        numDrugs += subjectList[i].drugList.size();
    }

    return numDrugs;
}


/* ------------------------------------------------------------ */
/* ----- GetNumAnalyses --------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::GetNumAnalyses
 * @return the total number of analysis objects in the package
 */
qint64 squirrel::GetNumAnalyses() {
    qint64 numAnalyses(0);

    /* iterate through subjects */
    for (int i=0; i < subjectList.size(); i++) {
        squirrelSubject sub = subjectList[i];
        /* iterate through studies */
        for (int j=0; j < sub.studyList.size(); j++) {
            numAnalyses += sub.studyList[j].analysisList.size();
        }
    }
    return numAnalyses;
}


/* ------------------------------------------------------------ */
/* ----- GetNumExperiments ------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::GetNumExperiments
 * @return the number of experiments
 */
qint64 squirrel::GetNumExperiments() {
    return experimentList.size();
}


/* ------------------------------------------------------------ */
/* ----- GetNumPipelines -------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::GetNumPipelines
 * @return the number of pipelines
 */
qint64 squirrel::GetNumPipelines() {
    return pipelineList.size();
}


/* ------------------------------------------------------------ */
/* ----- GetNumGroupAnalyses ---------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::GetNumGroupAnalyses
 * @return the number of group analyses
 */
qint64 squirrel::GetNumGroupAnalyses() {
    return groupAnalysisList.size();
}


/* ------------------------------------------------------------ */
/* ----- GetNumDataDictionaryItems ---------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::GetNumDataDictionaryItems
 * @return the number of data dictionary items
 */
qint64 squirrel::GetNumDataDictionaryItems() {
    return dataDictionary.dictItems.size();
}


/* ------------------------------------------------------------ */
/* ----- addSubject ------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Add a subject to the package
 * @param subj squirrelSubject to be added
 * @return true if added, false otherwise
 */
bool squirrel::addSubject(squirrelSubject subj) {

    /* check size of the subject list before and after adding */
    qint64 size = subjectList.size();

    subjectList.append(subj);

    if (subjectList.size() > size)
        return true;
    else
        return false;
}


/* ------------------------------------------------------------ */
/* ----- addPipeline ------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief Add a pipeline to the package
 * @param pipe squirrelPipeline to be added
 * @return true if added, false otherwise
 */
bool squirrel::addPipeline(squirrelPipeline pipe) {

    /* check size of the pipeline list before and after adding */
    qint64 size = pipelineList.size();

    pipelineList.append(pipe);

    if (pipelineList.size() > size)
        return true;
    else
        return false;
}


/* ------------------------------------------------------------ */
/* ----- addExperiment ---------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Add an experiment to the package
 * @param exp a squirrelExperiment to be added
 * @return true if added, false if not added
 */
bool squirrel::addExperiment(squirrelExperiment exp) {

    /* check size of the pipeline list before and after adding */
    qint64 size = experimentList.size();

    experimentList.append(exp);

    if (experimentList.size() > size)
        return true;
    else
        return false;
}


/* ------------------------------------------------------------ */
/* ----- removeSubject ---------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Removed a subject, by ID, from the package
 * @param ID subject ID to be removed
 * @return true if subject found and removed, false otherwise
 */
bool squirrel::removeSubject(QString ID) {

    for(int i=0; i < subjectList.count(); ++i) {
        if (subjectList[i].ID == ID) {
            subjectList.remove(i);
            return true;
        }
    }
    return false;
}


/* ------------------------------------------------------------ */
/* ----- PrintPackage ----------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Print package details
 */
void squirrel::PrintPackage() {

    qint64 numSubjects = GetNumSubjects();
    qint64 numStudies = GetNumStudies();
    qint64 numSeries = GetNumSeries();
    qint64 numMeasures(0);
    qint64 numDrugs(0);
    qint64 numAnalyses(0);
    qint64 numExperiments(0);
    qint64 numPipelines(0);
    qint64 numGroupAnalyses(0);
    qint64 numDataDictionaries = GetNumDataDictionaryItems();

    Print("Squirrel Package: " + filePath);
    Print(QString("  Date: %1").arg(datetime.toString()));
    Print(QString("  Description: %1").arg(description));
    Print(QString("  Name: %1").arg(name));
    Print(QString("  Version: %1").arg(version));
    Print(QString("  Directory Format (subject, study, series): %1, %2, %3").arg(subjectDirFormat).arg(studyDirFormat).arg(seriesDirFormat));
    Print(QString("  Data Format: %1").arg(dataFormat));
    Print(QString("  Files:\n    %1 files\n    %2 bytes (unzipped)").arg(GetNumFiles()).arg(GetUnzipSize()));
    Print(QString("  Objects:\n    %1 subjects\n    %2 studies\n    %3 series\n    %4 measures\n    %5 drugs\n    %6 analyses\n    %7 experiments\n    %8 pipelines\n    %9 group analyses\n    %10 data dictionary").arg(numSubjects).arg(numStudies).arg(numSeries).arg(numMeasures).arg(numDrugs).arg(numAnalyses).arg(numExperiments).arg(numPipelines).arg(numGroupAnalyses).arg(numDataDictionaries));
}


/* ------------------------------------------------------------ */
/* ----- MakeTempDir ------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::MakeTempDir
 * @return true if created/exists, false otherwise
 */
bool squirrel::MakeTempDir(QString &dir) {

    QString d;
    #ifdef Q_OS_WINDOWS
        d = QString("C:/tmp/%1").arg(GenerateRandomString(20));
    #else
        d = QString("/tmp/%1").arg(GenerateRandomString(20));
    #endif

    QString m;
    if (MakePath(d, m)) {
        dir = d;
        return true;
    }
    else {
        dir = "";
        return false;
    }
}


/* ------------------------------------------------------------ */
/* ----- GetSubjectIndex -------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Search for index of a subject
 * @param ID subject ID
 * @return index of the subject, if found. -1 if not found
 */
int squirrel::GetSubjectIndex(QString ID) {

    /* find subject by ID */
    for (int i=0; i < subjectList.size(); i++) {
        if (subjectList[i].ID == ID) {
            return i;
        }
    }

    return -1;
}


/* ------------------------------------------------------------ */
/* ----- GetStudyIndex ---------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Search for index of a study
 * @param ID subject ID
 * @param studyNum study number
 * @return index of the study if found, -1 otherwise
 */
int squirrel::GetStudyIndex(QString ID, int studyNum) {

    /* first, find subject by ID */
    for (int i=0; i < subjectList.size(); i++) {
        if (subjectList[i].ID == ID) {
            /* next, find study by number */
            for (int j=0; j < subjectList[i].studyList.size(); j++) {
                if (subjectList[i].studyList[j].number == studyNum) {
                    return j;
                }
            }
        }
    }

    return -1;
}


/* ------------------------------------------------------------ */
/* ----- GetSeriesIndex --------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Search for index of a series
 * @param ID subject ID
 * @param studyNum study number
 * @param seriesNum series number
 * @return index of the series if found, -1 otherwise
 */
int squirrel::GetSeriesIndex(QString ID, int studyNum, int seriesNum) {
    squirrelSubject sqrlSubject;
    squirrelStudy sqrlStudy;
    bool subjectFound = false;
    bool studyFound = false;

    /* first, find subject by ID */
    for (int i=0; i < subjectList.size(); i++) {
        if (subjectList[i].ID == ID) {
            sqrlSubject = subjectList[i];
            subjectFound = true;
            break;
        }
    }

    /* next, find study by number */
    if (subjectFound) {
        for (int j=0; j < sqrlSubject.studyList.size(); j++) {
            if (sqrlSubject.studyList[j].number == studyNum) {
                sqrlStudy = sqrlSubject.studyList[j];
                studyFound = true;
                break;
            }
        }
    }

    /* then, find series by number */
    if (studyFound) {
        for (int k=0; k < sqrlStudy.seriesList.size(); k++) {
            if (sqrlStudy.seriesList[k].number == seriesNum) {
                return k;
            }
        }
    }

    return -1;
}


/* ------------------------------------------------------------ */
/* ----- GetPipelineIndex ------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Search for index of a pipeline
 * @param pipelineName pipeline name
 * @return index of pipeline if found, -1 otherwise
 */
int squirrel::GetPipelineIndex(QString pipelineName) {

    /* find pipeline by name */
    for (int i=0; i < pipelineList.size(); i++) {
        if (pipelineList[i].pipelineName == pipelineName) {
            return i;
        }
    }

    return -1;
}


/* ------------------------------------------------------------ */
/* ----- GetExperimentIndex ----------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Search for index of an experiment
 * @param experimentName experiment name
 * @return index of experiment if found, -1 otherwise
 */
int squirrel::GetExperimentIndex(QString experimentName) {

    /* find experiment by name */
    for (int i=0; i < experimentList.size(); i++) {
        if (experimentList[i].experimentName == experimentName) {
            return i;
        }
    }

    return -1;
}


/* ------------------------------------------------------------ */
/* ----- GetSubject ------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Search for and get a copy of a subject
 * @param ID subject ID
 * @param sqrlSubject copy of squirrelSubject object
 * @return true if found, false otherwise
 */
bool squirrel::GetSubject(QString ID, squirrelSubject &sqrlSubject) {

    /* find subject by ID */
    for (int i=0; i < subjectList.size(); i++) {
        if (subjectList[i].ID == ID) {
            sqrlSubject = subjectList[i];
            return true;
        }
    }

    return false;
}


/* ------------------------------------------------------------ */
/* ----- GetStudy --------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Search for and get a copy of a study
 * @param ID subjectID
 * @param studyNum studyNumber
 * @param sqrlStudy copy of squirrelStudy object
 * @return true if found, false otherwise
 */
bool squirrel::GetStudy(QString ID, int studyNum, squirrelStudy &sqrlStudy) {

    squirrelSubject sqrlSubject;
    bool subjectFound = false;

    /* first, find subject by ID */
    for (int i=0; i < subjectList.size(); i++) {
        if (subjectList[i].ID == ID) {
            sqrlSubject = subjectList[i];
            subjectFound = true;
            break;
        }
    }

    /* next, find study by number */
    if (subjectFound) {
        for (int i=0; i < sqrlSubject.studyList.size(); i++) {
            if (sqrlSubject.studyList[i].number == studyNum) {
                sqrlStudy = sqrlSubject.studyList[i];
                return true;
            }
        }
    }

    return false;
}


/* ------------------------------------------------------------ */
/* ----- GetSeries -------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Search for and get a copy of a series
 * @param ID subject ID
 * @param studyNum study number
 * @param seriesNum series number
 * @param sqrlSeries copy of squirrelSeries object
 * @return true if found, false otherwise
 */
bool squirrel::GetSeries(QString ID, int studyNum, int seriesNum, squirrelSeries &sqrlSeries) {
    squirrelSubject sqrlSubject;
    squirrelStudy sqrlStudy;
    bool subjectFound = false;
    bool studyFound = false;

    /* first, find subject by ID */
    for (int i=0; i < subjectList.size(); i++) {
        if (subjectList[i].ID == ID) {
            sqrlSubject = subjectList[i];
            subjectFound = true;
            break;
        }
    }

    /* next, find study by number */
    if (subjectFound) {
        for (int i=0; i < sqrlSubject.studyList.size(); i++) {
            if (sqrlSubject.studyList[i].number == studyNum) {
                sqrlStudy = sqrlSubject.studyList[i];
                studyFound = true;
                break;
            }
        }
    }

    /* then, find series by number */
    if (studyFound) {
        for (int i=0; i < sqrlStudy.seriesList.size(); i++) {
            if (sqrlStudy.seriesList[i].number == seriesNum) {
                sqrlSeries = sqrlStudy.seriesList[i];
                return true;
            }
        }
    }

    return false;
}


/* ------------------------------------------------------------ */
/* ----- GetSubjectList --------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Get a copy of the list of subjects
 * @param subjects QList of squirrelSubject objects
 * @return always true, even if empty
 */
bool squirrel::GetSubjectList(QList<squirrelSubject> &subjects) {

    subjects = subjectList;

    return true;
}


/* ------------------------------------------------------------ */
/* ----- GetStudyList ----------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Search for and get a copy of a list of studies
 * @param ID subject ID
 * @param studies QList of squirrelStudy objects
 * @return true if found, false otherwise
 */
bool squirrel::GetStudyList(QString ID, QList<squirrelStudy> &studies) {
    squirrelSubject sqrlSubj;

    if (GetSubject(ID, sqrlSubj)) {
        studies = sqrlSubj.studyList;
        return true;
    }

    return false;
}


/* ------------------------------------------------------------ */
/* ----- GetSeriesList ---------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Search for a get a copy of a list of series
 * @param ID subject ID
 * @param studyNum study number
 * @param series QList of squirrelSeries objects
 * @return true if found, false otherwise
 */
bool squirrel::GetSeriesList(QString ID, int studyNum, QList<squirrelSeries> &series) {
    squirrelStudy sqrlStudy;
    if (GetStudy(ID, studyNum, sqrlStudy)) {
        series = sqrlStudy.seriesList;
        return true;
    }

    return false;
}


/* ------------------------------------------------------------ */
/* ----- GetDrugList ------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief Search for an get a copy of a list of drugs
 * @param ID subject ID
 * @param drugs QList of squirrelDrug objects
 * @return true if found, false otherwise
 */
bool squirrel::GetDrugList(QString ID, QList<squirrelDrug> &drugs) {
    squirrelSubject sqrlSubj;

    if (GetSubject(ID, sqrlSubj)) {
        drugs = sqrlSubj.drugList;
        return true;
    }

    return false;
}


/* ------------------------------------------------------------ */
/* ----- GetMeasureList --------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Search for and get a copy of a list of measure objects
 * @param ID subject ID
 * @param measures QList of squirrelMeasure objects
 * @return true if found, false otherwise
 */
bool squirrel::GetMeasureList(QString ID, QList<squirrelMeasure> &measures) {
    squirrelSubject sqrlSubj;

    if (GetSubject(ID, sqrlSubj)) {
        measures = sqrlSubj.measureList;
        return true;
    }

    return false;
}


/* ------------------------------------------------------------ */
/* ----- GetAnalysis ------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief Search for and get a copy of an analysis
 * @param ID subject ID
 * @param studyNum study number
 * @param pipelineName pipeline name
 * @param sqrlAnalysis copy of a squirrelAnalysis object
 * @return true if found, false otherwise
 */
bool squirrel::GetAnalysis(QString ID, int studyNum, QString pipelineName, squirrelAnalysis &sqrlAnalysis) {
    squirrelStudy sqrlStudy;
    if (GetStudy(ID, studyNum, sqrlStudy)) {

        /* find analysis by pipelineName */
        for (int i=0; i < sqrlStudy.analysisList.size(); i++) {
            if (sqrlStudy.analysisList[i].pipelineName == pipelineName) {
                sqrlAnalysis = sqrlStudy.analysisList[i];
                return true;
            }
        }
    }

    return false;
}


/* ------------------------------------------------------------ */
/* ----- GetPipeline ------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief Search for and get a copy of a pipeline
 * @param pipelineName pipeline name
 * @param sqrlPipeline copy of a squirrelPipeline object
 * @return true if found, false otherwise
 */
bool squirrel::GetPipeline(QString pipelineName, squirrelPipeline &sqrlPipeline) {

    /* find pipeline by name */
    for (int i=0; i < pipelineList.size(); i++) {
        if (pipelineList[i].pipelineName == pipelineName) {
            sqrlPipeline = pipelineList[i];
            return true;
        }
    }

    return false;
}


/* ------------------------------------------------------------ */
/* ----- GetExperiment ---------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Search for a get a copy of an experiment
 * @param experimentName experiment name
 * @param sqrlExperiment copy of a squirrelExperiment object
 * @return true if found, false otherwise
 */
bool squirrel::GetExperiment(QString experimentName, squirrelExperiment &sqrlExperiment) {

    /* find experiment by name */
    for (int i=0; i < experimentList.size(); i++) {
        if (experimentList[i].experimentName == experimentName) {
            sqrlExperiment = experimentList[i];
            return true;
        }
    }

    return false;
}


/* ------------------------------------------------------------ */
/* ----- GetTempDir ------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Get the automatically created temp directory
 * @return the temp directory
 */
QString squirrel::GetTempDir() {
    return workingDir;
}


/* ------------------------------------------------------------ */
/* ----- Log -------------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Record a log - prints to screen and stores in log string
 * @param s log message
 * @param func function that called this function
 * @param dbg is this is a debug message, to be displayed only if debug is enabled at the command line
 */
void squirrel::Log(QString s, QString func, bool dbg) {
    //Print(QString("debug[%1]  dbg [%2]").arg(debug).arg(dbg));
    if (!quiet) {
        if ((!dbg) || (debug && dbg)) {
            if (s.trimmed() != "") {
                log.append(QString("%1() %2\n").arg(func).arg(s));
                Print(QString("%1() %2").arg(func).arg(s));
            }
        }
    }
}


/* ------------------------------------------------------------ */
/* ----- AddSeriesFiles --------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Add files to a series. This function also copies the files to the staging/temp directory
 * @param ID subject ID
 * @param studyNum study number
 * @param seriesNum series number
 * @param files list of files to be added
 * @param destDir sub-directory within the series staging directory
 * @return true if successful, false otherwise
 */
bool squirrel::AddSeriesFiles(QString ID, int studyNum, int seriesNum, QStringList files, QString destDir) {

    /* make sure the subject info is not blank */
    if (ID == "") return false;
    if (studyNum < 1) return false;
    if (seriesNum < 1) return false;

    /* create the experiment path on disk and set the experiment path in  */
    QString dir = QString("%1/data/%2/%3/%4").arg(workingDir).arg(ID).arg(studyNum).arg(seriesNum);
    QString m;
    MakePath(dir, m);
    foreach (QString f, files) {
        /* copy this to the packageRoot/destDir directory */
        Log(QString("Copying file [%1] to [%2]").arg(f).arg(dir), __FUNCTION__, true);
        CopyFile(f, dir);
    }

    return true;
}


/* ------------------------------------------------------------ */
/* ----- AddAnalysisFiles ------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Add files to an analysis. This function also copies the files to the staging/temp directory
 * @param ID subject ID
 * @param studyNum study number
 * @param pipelineName pipeline name
 * @param files list of files to be added
 * @param destDir sub-directory within the analysis staging directory
 * @return true if successful, false otherwise
 */
bool squirrel::AddAnalysisFiles(QString ID, int studyNum, QString pipelineName, QStringList files, QString destDir) {

    /* make sure the subject info is not blank */
    if (ID == "") return false;
    if (studyNum < 1) return false;
    if (pipelineName == "") return false;

    /* create the experiment path on disk and set the experiment path in  */
    QString dir = QString("%1/data/%2/%3/%4").arg(workingDir).arg(ID).arg(studyNum).arg(pipelineName);
    QString m;
    MakePath(dir, m);
    foreach (QString f, files) {
        /* copy this to the packageRoot/destDir directory */
        CopyFile(f, dir);
    }

    return true;
}


/* ------------------------------------------------------------ */
/* ----- AddPipelineFiles ------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Add files to a pipeline. This function also copies the files to the staging/temp directory
 * @param pipelineName pipeline name
 * @param files list of files to be added
 * @param destDir sub-directory within the analysis staging directory
 * @return true if successful, false otherwise
 */
bool squirrel::AddPipelineFiles(QString pipelineName, QStringList files, QString destDir) {

    /* make sure the experiment name is not blank */
    if (pipelineName == "") {
        return false;
    }
    /* create the experiment path on disk and set the experiment path in  */
    QString dir = QString("%1/experiments/%2/%3").arg(workingDir).arg(pipelineName).arg(destDir);
    QString m;
    MakePath(dir, m);
    foreach (QString f, files) {
        /* copy this to the packageRoot/destDir directory */
        CopyFile(f, dir);
    }

    return true;
}


/* ------------------------------------------------------------ */
/* ----- AddExperimentFiles ----------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Add files to an experiment. This function also copies the files to the staging/temp directory
 * @param experimentName experiment name
 * @param files list of files to be added
 * @param destDir sub-directory within the analysis staging directory
 * @return true if successful, false otherwise
 */
bool squirrel::AddExperimentFiles(QString experimentName, QStringList files, QString destDir) {

    /* make sure the experiment name is not blank */
    if (experimentName == "") {
        return false;
    }
    /* create the experiment path on disk and set the experiment path in  */
    QString dir = QString("%1/experiments/%2/%3").arg(workingDir).arg(experimentName).arg(destDir);
    QString m;
    MakePath(dir, m);
    foreach (QString f, files) {
        /* copy this to the packageRoot/destDir directory */
        CopyFile(f, dir);
    }

    return true;
}

/* ------------------------------------------------------------ */
/* ----- ReadParamsFile --------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Reads a JSON key/value pair file into a hash
 * @param f JSON file
 * @return list of key/value pairs
 */
QHash<QString, QString> squirrel::ReadParamsFile(QString f) {

    QString jsonStr;
    QFile file;
    file.setFileName(f);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    jsonStr = file.readAll();
    file.close();

    /* get the JSON document and root object */
    QJsonDocument d = QJsonDocument::fromJson(jsonStr.toUtf8());

    QHash<QString, QString> tags;

    QJsonObject json = d.object();
    foreach(const QString& key, json.keys()) {
        tags[key] = json.value(key).toString();
    }
    return tags;

}


/* ------------------------------------------------------------ */
/* ----- PrintSubjects ---------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::PrintSubjects print list of subjects to stdout
 * @param details true to print details, false to print list of subject IDs
 */
void squirrel::PrintSubjects(bool details) {

    if (details) {
        foreach (squirrelSubject s, subjectList) {
            s.PrintSubject();
        }
    }
    else {
        QStringList ids;
        foreach (squirrelSubject s, subjectList) {
            ids.append(s.ID);
        }
        Print(ids.join(" "));
    }
}


/* ------------------------------------------------------------ */
/* ----- PrintStudies ----------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::PrintStudies print list of studies to the stdout
 * @param subjectID the subject ID to print studies for
 * @param details true to print details, false to print list of study numbers
 */
void squirrel::PrintStudies(QString subjectID, bool details) {
    if (details) {
        squirrelSubject sqrlSubject;
        if (GetSubject(subjectID, sqrlSubject)) {
            foreach (squirrelStudy s, sqrlSubject.studyList) {
                s.PrintStudy();
            }
        }
    }
    else {
        QStringList studynums;
        squirrelSubject sqrlSubject;
        if (GetSubject(subjectID, sqrlSubject)) {
            foreach (squirrelStudy s, sqrlSubject.studyList) {
                studynums.append(QString("%1").arg(s.number));
            }
        }
        Print(studynums.join(" "));
    }
}


/* ------------------------------------------------------------ */
/* ----- PrintSeries ------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::PrintSeries print list of series to stdout
 * @param subjectID the subject ID
 * @param studyNum the study number
 * @param details true to print details, false to print list of series numbers
 */
void squirrel::PrintSeries(QString subjectID, int studyNum, bool details) {
    if (details) {
        squirrelStudy sqrlStudy;
        if (GetStudy(subjectID, studyNum, sqrlStudy)) {
            foreach (squirrelSeries s, sqrlStudy.seriesList) {
                s.PrintSeries();
            }
        }
    }
    else {
        QStringList seriesnums;
        squirrelStudy sqrlStudy;
        if (GetStudy(subjectID, studyNum, sqrlStudy)) {
            foreach (squirrelSeries s, sqrlStudy.seriesList) {
                seriesnums.append(QString("%1").arg(s.number));
            }
        }
        Print(seriesnums.join(" "));
    }
}


/* ------------------------------------------------------------ */
/* ----- PrintExperiments ------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::PrintExperiments
 * @param details true to print details, false to print list of pipeline names
 */
void squirrel::PrintExperiments(bool details) {
    if (details) {
        foreach (squirrelExperiment e, experimentList) {
            e.PrintExperiment();
        }
    }
    else {
        QStringList exps;
        foreach (squirrelExperiment e, experimentList) {
            exps.append(e.experimentName);
        }
        Print(exps.join(" "));
    }
}


/* ------------------------------------------------------------ */
/* ----- PrintPipelines --------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrel::PrintPipelines
 * @param details true to print details, false to print list of pipeline names
 */
void squirrel::PrintPipelines(bool details) {
    if (details) {
        foreach (squirrelPipeline p, pipelineList) {
            p.PrintPipeline();
        }
    }
    else {
        QStringList pipes;
        foreach (squirrelPipeline p, pipelineList) {
            pipes.append(p.pipelineName);
        }
        Print(pipes.join(" "));
    }
}


/* ------------------------------------------------------------ */
/* ----- PrintGroupAnalyses ----------------------------------- */
/* ------------------------------------------------------------ */
void squirrel::PrintGroupAnalyses(bool details) {
}


/* ------------------------------------------------------------ */
/* ----- PrintDataDictionary ---------------------------------- */
/* ------------------------------------------------------------ */
void squirrel::PrintDataDictionary(bool details) {
}
