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

        LoadSubjectFiles(subjfiles, sqrl);

        /* get a list of ses-* DIRS, if there are any */
        QStringList sesdirs = FindAllDirs(subjdir, "ses-*", false);
        sqrl->Log(QString("Found [%1] session directories matching '%2/ses-*'").arg(sesdirs.size()).arg(subjdir), __FUNCTION__);
        if (sesdirs.size() > 0) {
            foreach (QString sesdir, sesdirs) {
                LoadSessionDir(sesdir, sqrl);
            }
        }
        else {
            /* if there are no ses-* directories, then the session must be in the root subject directory */
            LoadSessionDir(subjdir, sqrl);
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
bool bids::LoadRootFiles(QStringList rootfiles, squirrel *sqrl) {
    /* read into consolidated JSON object and call it bids.json, with each file in it's own object */
    /* read the participants.tsv, should be in the root */

    sqrl->Log("Entering function", __FUNCTION__);

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
bool bids::LoadSubjectFiles(QStringList subjfiles, squirrel *sqrl) {
    sqrl->Log("Entering function", __FUNCTION__);

    foreach (QString f, subjfiles) {
        QFileInfo fi(f);
        QString filename = fi.fileName();

        sqrl->Log(QString("Found file [%1]").arg(filename), __FUNCTION__);

        /* possible files in the subject root dir
            sub-*-sessions.tsv
        */
        if (filename == "") {
        }
    }

    return true;
}


/* ---------------------------------------------------------------------------- */
/* ----- LoadSessionDir ------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
bool bids::LoadSessionDir(QString sesdir, squirrel *sqrl) {

    sqrl->Log("Entering function", __FUNCTION__);

    /* possible directories
        anat
        func
        figures
        anat
        fmap
        ieeg
        perf
        eeg
        micr
        motion
    */

    /* get list of all dirs in this sesdir */

    /*if (dirname == "anat") {
    }
    else if (dirname == "func") {
    }
    else if (dirname == "figures") {
    }
    else if (dirname == "anat") {
    }
    else {
    }*/

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
bool bids::LoadTaskFile(QString f, squirrel *sqrl) {
    QString str = ReadTextFileToString(f);
    QJsonDocument d = QJsonDocument::fromJson(str.toUtf8());
    QJsonObject root = d.object();

    QString experimentName = root.value("TaskName").toString();
    double tr = root.value("RepetitionTime").toDouble();

    squirrelExperiment sqrlExp;
    sqrlExp.experimentName = experimentName;
    //sqrlExp.
}
