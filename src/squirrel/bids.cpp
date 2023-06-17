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
bool bids::LoadToSquirrel(QString dir, squirrel *sqrl, QString &msg) {
    QStringList msgs;
    QString m;

    msgs << QString("%1() Entering function. dir [%2]").arg(__FUNCTION__).arg(dir);

    /* check if directory exists */
    QDir d(dir);
    if (!d.exists()) {
        msgs << QString("%1() Directory [%2] does not exist").arg(__FUNCTION__).arg(dir);
        msg = msgs.join("\n");
        return false;
    }

    /* check for all .json files in the root directory */
    QStringList rootfiles = FindAllFiles(dir, "*", false);
    msgs << QString("%1() Found [%2] root files matching '%3/*'").arg(__FUNCTION__).arg(rootfiles.size()).arg(dir);
    m = "";
    LoadRootFiles(rootfiles, sqrl, m);
    msgs << m;

    /* get list of directories in the root named 'sub-*' */
    QStringList subjdirs = FindAllDirs(dir, "sub-*", false);

    msgs << QString("%1() Found [%2] subject directories matching '%3/sub-*'").arg(__FUNCTION__).arg(subjdirs.size()).arg(dir);
    foreach (QString subjdir, subjdirs) {

        /* get all the FILES inside of the subject directory */
        QStringList subjfiles = FindAllFiles(subjdir, "*", false);
        msgs << QString("%1() Found [%2] subject root files matching '%3/*'").arg(__FUNCTION__).arg(subjfiles.size()).arg(subjdir);

        m = "";
        LoadSubjectFiles(subjfiles, sqrl, m);
        msgs << m;

        /* get a list of ses-* DIRS, if there are any */
        QStringList sesdirs = FindAllDirs(subjdir, "ses-*", false);
        msgs << QString("%1() Found [%2] session directories matching '%3/ses-*'").arg(__FUNCTION__).arg(sesdirs.size()).arg(subjdir);
        if (sesdirs.size() > 0) {
            foreach (QString sesdir, sesdirs) {
                LoadSessionDir(sesdir, sqrl, m);
            }
        }
        else {
            /* if there are no ses-* directories, then the session must be in the root subject directory */
            LoadSessionDir(subjdir, sqrl, m);
        }

    }

    /* check for a 'derivatives' directory, which are analyses */

    /* check for a 'logs' directory, which are logs */

    /* check for a 'code' directory, which are pipeline/code */

    /* check for a 'stimuli' directory, which are experiments */

    /* check for a 'phenotype' directory, which are ... subject demographics? */

    msg = msgs.join("\n");
    return true;
}


/* ---------------------------------------------------------------------------- */
/* ----- LoadRootFiles -------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
bool bids::LoadRootFiles(QStringList rootfiles, squirrel *sqrl, QString &m) {
    /* read into consolidated JSON object and call it bids.json, with each file in it's own object */
    /* read the participants.tsv, should be in the root */

    QStringList msgs;
    msgs << QString("%1() Entering function").arg(__FUNCTION__);

    foreach (QString f, rootfiles) {
        QFileInfo fi(f);
        QString filename = fi.fileName();

        msgs << QString("%1() Found file [%2]").arg(__FUNCTION__).arg(filename);
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
        }
        else if (filename == "participants.tsv") {
            /* this goes into the subjects object */
            if (!LoadParticipantsFile(f, sqrl, m)) {
                msgs << "Error loading participants.tsv";
            }
        }
    }

    m = msgs.join("\n");
    return true;
}


/* ---------------------------------------------------------------------------- */
/* ----- LoadSubjectFiles ----------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
bool bids::LoadSubjectFiles(QStringList subjfiles, squirrel *sqrl, QString &m) {
    QStringList msgs;
    msgs << QString("%1() Entering function").arg(__FUNCTION__);

    foreach (QString f, subjfiles) {
        QFileInfo fi(f);
        QString filename = fi.fileName();

        msgs << QString("%1() Found file [%2]").arg(__FUNCTION__).arg(filename);

        /* possible files in the subject root dir
            sub-*-sessions.tsv
        */
        if (filename == "") {
        }
    }

    m = msgs.join("\n");
    return true;
}


/* ---------------------------------------------------------------------------- */
/* ----- LoadSessionDir ------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
bool bids::LoadSessionDir(QString sesdir, squirrel *sqrl, QString &m) {

    QStringList msgs;
    msgs << QString("%1() Entering function").arg(__FUNCTION__);

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

    m = msgs.join("\n");
    return true;
}


/* ---------------------------------------------------------------------------- */
/* ----- LoadParticipantFile -------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
bool bids::LoadParticipantsFile(QString f, squirrel *sqrl, QString &m) {
    QStringList lines = ReadTextFileIntoArray(f);

    int i=1;
    QStringList cols;
    foreach (QString line, lines) {
        if (i == 1) {
            cols = line.split("\t");
        }
        else {
            QStringList parts = line.split("\t");
        }
        i++;
    }
    return true;
}
