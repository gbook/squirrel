/* ------------------------------------------------------------------------------
  Squirrel bids.cpp
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



#include "bids.h"


/* ---------------------------------------------------------------------------- */
/* ----- bids ----------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
bids::bids()
{

}


/* ---------------------------------------------------------------------------- */
/* ----- LoadToSquirrel ------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/**
 * @brief Load a BIDS directory into a squirrel object
 * @param dir path to the BIDS directory
 * @param sqrl squirrel object
 * @return true if loaded successfully, false otherwise
 */
bool bids::LoadToSquirrel(QString dir, squirrel *sqrl) {

    sqrl->Log(QString("Entering function. dir [%1]").arg(dir), __FUNCTION__);

    /* check if directory exists */
    QDir d(dir);
    if (!d.exists()) {
        sqrl->Log(QString("Directory [%1] does not exist").arg(dir), __FUNCTION__);
        return false;
    }

    /* check for all .json files in the root directory */
    QStringList rootfiles = FindAllFiles(dir, "*", false);
    sqrl->Log(QString("Found [%1] root files matching '%2/*'").arg(rootfiles.size()).arg(dir), __FUNCTION__);
    LoadRootFiles(rootfiles, sqrl);

    /* get list of directories in the root named 'sub-*' */
    QStringList subjdirs = FindAllDirs(dir, "sub-*", false);

    sqrl->Log(QString("Found [%1] subject directories matching '%2/sub-*'").arg(subjdirs.size()).arg(dir), __FUNCTION__);
    foreach (QString subjdir, subjdirs) {

        /* get all the FILES inside of the subject directory */
        QStringList subjfiles = FindAllFiles(subjdir, "*", false);
        sqrl->Log(QString("Found [%1] subject root files matching '%2/*'").arg(subjfiles.size()).arg(subjdir), __FUNCTION__);

        QString ID = subjdir;
        LoadSubjectFiles(subjfiles, subjdir, sqrl);

        /* get a list of ses-* DIRS, if there are any */
        QStringList sesdirs = FindAllDirs(subjdir, "ses-*", false);
        sqrl->Log(QString("Found [%1] session directories matching '%2/ses-*'").arg(sesdirs.size()).arg(subjdir), __FUNCTION__);
        if (sesdirs.size() > 0) {
            foreach (QString sesdir, sesdirs) {
                QString sespath = QString("%1/%2/%3").arg(dir).arg(subjdir).arg(sesdir);
                LoadSessionDir(sespath, sqrl);
            }
        }
        else {
            /* if there are no ses-* directories, then the session must be in the root subject directory */
            QString sespath = QString("%1/%2").arg(dir).arg(subjdir);
            LoadSessionDir(sespath, sqrl);
        }

    }

    /* check for a 'derivatives' directory, which are analyses */

    /* check for a 'logs' directory, which are logs */

    /* check for a 'code' directory, which are pipeline/code */

    /* check for a 'stimuli' directory, which are experiments */

    /* check for a 'phenotype' directory, which are ... subject demographics? */

    return true;
}


/* ---------------------------------------------------------------------------- */
/* ----- LoadRootFiles -------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/**
 * @brief Load the files contained in the root of the BIDS directory
 * @param rootfiles list of files
 * @param sqrl squirrel object
 * @return true if successful, false if any errors
 */
bool bids::LoadRootFiles(QStringList rootfiles, squirrel *sqrl) {
    sqrl->Log(QString("Entering function to process [%1] root files").arg(rootfiles.size()), __FUNCTION__);

    foreach (QString f, rootfiles) {
        QFileInfo fi(f);
        QString filename = fi.fileName();

        sqrl->Log(QString("Found file [%1]").arg(filename), __FUNCTION__);
        /* possible files in the root dir
            dataset_description.json
            participants.json
            participants.tsv
            README
            README.md
            CHANGES
            task-*_bold.json
            task-*_events.tsv
            task-*_physio.json
            task-*_stim.json
            desc-aparcaseg_dseg.tsv
            acq-*_<modality>.json (modality is something like T1w, dwi, and will match a directory inside each sub-* directory)
        */

        if (filename == "dataset_description.json") {
            sqrl->description = ReadTextFileToString(f);
        }
        else if ((filename == "README") || (filename == "README.md")) {
            sqrl->readme = ReadTextFileToString(f);
        }
        else if (filename == "CHANGES") {
            sqrl->changes = ReadTextFileToString(f);
        }
        else if (filename.startsWith("task-")) {
            /* this goes into the squirrel experiments object */
            LoadTaskFile(f, sqrl);
        }
        else if (filename == "participants.tsv") {
            /* this goes into the subjects object */
            if (!LoadParticipantsFile(f, sqrl)) {
                sqrl->Log("Error loading participants.tsv", __FUNCTION__);
            }
        }
    }

    return true;
}


/* ---------------------------------------------------------------------------- */
/* ----- LoadSubjectFiles ----------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/**
 * @brief Load the files contained in the subject's directory in the BIDS object
 * @param subjfiles list of subject files
 * @param ID BIDS ID of this subject
 * @param sqrl squirrel object
 * @return true
 */
bool bids::LoadSubjectFiles(QStringList subjfiles, QString ID, squirrel *sqrl) {
    sqrl->Log("Entering function", __FUNCTION__);

    foreach (QString f, subjfiles) {
        QFileInfo fi(f);
        QString filename = fi.fileName();

        sqrl->Log(QString("Found file [%1]").arg(filename), __FUNCTION__);

        /* possible files in the subject root dir
            sub-*-sessions.tsv

            There's not a lot whole lot of information in this file, but it seems to contain analysis
            or behavioral information... So let's put it in the analysis object
            But, we need to create an empty study and pipeline for these files, because the BIDS analysis files are
            not associated with a session/study or pipeline
        */
        if (filename.endsWith("_scans.tsv")) {

            /* get the subject */
            squirrelSubject sqrlSubject;
            sqrl->GetSubject(ID, sqrlSubject);

            /* create a session/study and add it to the subject */
            squirrelStudy sqrlStudy;
            sqrlStudy.number = sqrlSubject.GetNextStudyNumber();
            sqrlSubject.addStudy(sqrlStudy);

            /* create an analysis */
            squirrelAnalysis sqrlAnalysis;
            sqrlAnalysis.pipelineName = "analysis";
            sqrlAnalysis.lastMessage = "BIDS imported analysis file";
            sqrlStudy.addAnalysis(sqrlAnalysis);

            QStringList files;
            files.append(f);
            sqrl->AddAnalysisFiles(ID, sqrlStudy.number, "analysis", files);
            sqrl->Log(QString("Added [%1] files to analysis [%2]").arg(files.size()).arg("analysis"), __FUNCTION__);

            /* not sure we need to read this file in... just copying it to analysis should be
             * sufficient. But what is the analysis called? */
            //QString str = ReadTextFileToString(f);
            //indexedHash tsv;
            //QStringList cols;
            //QString m;

            //if (ParseTSV(str, tsv, cols, m)) {
            //    sqrl->Log(QString("Successful read [%1] into [%2] rows").arg(f).arg(tsv.size()), __FUNCTION__);
            //    for (int i=0; i<tsv.size(); i++) {
            //        QString id = tsv[i]["participant_id"];
            //    }
            //}
        }
    }

    return true;
}


/* ---------------------------------------------------------------------------- */
/* ----- LoadSessionDir ------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/**
 * @brief Load the files in a session directory
 * @param sesdir the session directory
 * @param sqrl squirrel object
 * @return true
 */
bool bids::LoadSessionDir(QString sesdir, squirrel *sqrl) {

    //sqrl->Log(QString("Entering function to load session dir [%1]").arg(sesdir), __FUNCTION__);

    /* possible directories
        anat
        func
        figures
        fmap
        ieeg
        perf
        eeg
        micr
        motion
    */

    /* get list of all dirs in this sesdir */
    QStringList sesdirs = FindAllDirs(sesdir, "*", false);
    sqrl->Log(QString("Found [%1] directories in [%2/*]").arg(sesdirs.size()).arg(sesdir), __FUNCTION__);
    if (sesdirs.size() > 0) {
        foreach (QString dir, sesdirs) {
            QString datadir = QString("%1/%2").arg(sesdir).arg(dir);
            QStringList files = FindAllFiles(datadir, "*", false);
            sqrl->Log(QString("Found [%1] files in '%2'").arg(files.size()).arg(datadir), __FUNCTION__);

            /* now do something with the files, depending on what they are */
            if ((dir == "anat") || (dir == "func") || (dir == "fmap") || (dir == "perf")) {
                foreach (QString f, files) {

                    sqrl->Log(QString("Found file [%1] of type [%2]").arg(f).arg(dir), __FUNCTION__);

                    QString filename = QFileInfo(f).fileName();
                    filename.replace(".nii.gz", "");
                    filename.replace(".nii", "");
                    QString ID = filename.section("_", 0,0);
                    QString protocol = filename.section("_", -1);
                    int studyNum = 1;
                    qint64 seriesNum = 1;

                    /* get the subject index */
                    int subjectIndex = sqrl->GetSubjectIndex(ID);

                    /* check if this study exists */
                    int studyIndex = sqrl->GetStudyIndex(ID, studyNum);

                    if (studyIndex > -1) {
                        /* study exists, so let's add a series to it */
                        seriesNum = sqrl->subjectList[subjectIndex].studyList[studyIndex].GetNextSeriesNumber();
                        sqrl->Log(QString("Next series number [%1]").arg(seriesNum), __FUNCTION__);
                        squirrelSeries series;
                        series.number = seriesNum;
                        series.protocol = protocol;
                        if (!sqrl->subjectList[subjectIndex].studyList[studyIndex].addSeries(series))
                            sqrl->Log(QString("Unable to add seriesNum [%1] protocol [%2]").arg(seriesNum).arg(protocol), __FUNCTION__);
                    }
                    else {
                        /* study doesn't exist, so create it */
                        squirrelStudy study2;
                        study2.number = studyNum;
                        study2.modality = "MR";

                        /* create the series and add it to the study */
                        squirrelSeries series;
                        series.number = seriesNum;
                        series.protocol = protocol;
                        study2.addSeries(series);

                        /* add the study to the subject */
                        if (!sqrl->subjectList[subjectIndex].addStudy(study2))
                            sqrl->Log("Unable to add study!", __FUNCTION__);
                    }

                    /* now that the subject/study/series exist, add the file(s) */
                    QStringList files;
                    files << QString(f);
                    sqrl->AddSeriesFiles(ID, studyNum, seriesNum, files);
                }
            }
            else if (dir == "func") {
            }
            else if (dir == "figures") {
            }
            else {
            }
        }
    }

    /* for each scan... */
    /* map the BIDS thing to an actual modality: MapBIDStoModality() */
    /* parse the file names --> protocol and run/series number */
    /* read the .json file for all the parameters --> params.json, modality, maybe other info */
    /* the real modalitity might be in one of the .json files */

    return true;
}


/* ---------------------------------------------------------------------------- */
/* ----- LoadParticipantFile -------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/**
 * @brief Loads the participants.tsv file
 * @param f the path to the file
 * @param sqrl squirrel object
 * @return true if successful, false otherwise
 */
bool bids::LoadParticipantsFile(QString f, squirrel *sqrl) {
    /* do we need to read the .json file? There's not much in there that isn't already specified here */

    QString file = ReadTextFileToString(f);

    indexedHash tsv;
    QStringList cols;
    QString m;

    if (ParseTSV(file, tsv, cols, m)) {
        sqrl->Log(QString("Successful read [%1] into [%2] rows").arg(f).arg(tsv.size()), __FUNCTION__);
        for (int i=0; i<tsv.size(); i++) {
            QString id = tsv[i]["participant_id"];
            QString age = tsv[i]["age"];
            QString sex = tsv[i]["sex"];
            QString hand = tsv[i]["handedness"];
            QString species = tsv[i]["species"];
            QString strain = tsv[i]["strain"];

            /* add a subject */
            squirrelSubject sqrlSubj;
            sqrlSubj.ID = id;
            sqrlSubj.sex = sex;
            sqrlSubj.gender = sex;

            /* add handedness as a measure */
            squirrelMeasure sqrlMeas;
            sqrlMeas.description = "Handedness";
            sqrlMeas.measureName = "Handedness";
            sqrlMeas.value = hand;
            sqrlSubj.addMeasure(sqrlMeas);

            sqrlMeas.description = "Species";
            sqrlMeas.measureName = "Species";
            sqrlMeas.value = species;
            sqrlSubj.addMeasure(sqrlMeas);

            sqrlMeas.description = "Strain";
            sqrlMeas.measureName = "Strain";
            sqrlMeas.value = strain;
            sqrlSubj.addMeasure(sqrlMeas);

            sqrlMeas.description = "age";
            sqrlMeas.measureName = "age";
            sqrlMeas.value = age;
            sqrlSubj.addMeasure(sqrlMeas);

            sqrl->addSubject(sqrlSubj);
        }
    }
    else {
        sqrl->Log(QString("Error reading tsv file [%1] message [%2]").arg(f).arg(m), __FUNCTION__);
    }

    return true;
}


/* ---------------------------------------------------------------------------- */
/* ----- LoadTaskFile --------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/**
 * @brief Load a file containing task information
 * @param f the file path
 * @param sqrl squirrel object
 * @return true if successful, false otherwise
 */
bool bids::LoadTaskFile(QString f, squirrel *sqrl) {

    QFileInfo fi(f);
    QString filename = fi.fileName();
    filename.replace("task-", "");
    filename.replace(".json", "");
    filename.replace("_events", "");
    filename.replace("_bold", "");
    QString str = ReadTextFileToString(f);
    QJsonDocument d = QJsonDocument::fromJson(str.toUtf8());
    QJsonObject root = d.object();

    QString experimentName = root.value("TaskName").toString().toLower();
    if (experimentName == "")
        experimentName = filename;

    //double tr = root.value("RepetitionTime").toDouble();

    QStringList files;
    files.append(f);
    squirrelExperiment sqrlExp;
    sqrlExp.experimentName = experimentName;
    sqrlExp.path = QString("%1/%2").arg(sqrl->GetTempDir()).arg(experimentName);
    sqrl->AddExperimentFiles(experimentName, files);
    sqrl->Log(QString("Added [%1] files to experiment [%2] with path [%3]").arg(files.size()).arg(experimentName).arg(sqrlExp.path), __FUNCTION__);

    return true;
}
