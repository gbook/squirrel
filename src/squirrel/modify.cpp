/* ------------------------------------------------------------------------------
  Squirrel modify.cpp
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

#include "modify.h"
#include "utils.h"

modify::modify() {
}


/* ---------------------------------------------------------------------------- */
/* ----- DoModify ------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
bool modify::DoModify(QString packagePath, QString addObject, QString removeObject, QString dataPath, bool recursive, QString objectData, QString objectID, QString subjectID, int studyNum, QString &m) {

    /* check if any operation was specified */
    if ((addObject == "") && (removeObject == "")) {
        m = "No object specified to add or remove";
        return false;
    }
    /* check if at most one operation was specified */
    if ((addObject != "") && (removeObject != "")) {
        m = "Both add and remove operations were specified. Only one add/remove operation allowed at a time";
        return false;
    }

    /* perform the ADD object */
    if (addObject != "") {
        QStringList objectsWithPaths = {"series", "analysis", "experiment", "pipeline", "groupanalysis"};

        /* check if the user should have specified a path */
        if (objectsWithPaths.contains(addObject)) {

            /* check if that path is specified */
            if (dataPath == "") {
                m = QString("No datapath specified for this object. A datapath must be specified for %1 objects.").arg(addObject);
                return false;
            }

            /* check if the specified path exists */
            if (!utils::DirectoryExists(dataPath)) {
                m = QString("Specified datapath [%1] does not exist").arg(dataPath);
                return false;
            }
        }

        /* get the object data */
        QHash<QString, QString> vars;
        QStringList metadata = objectData.split("&");
        foreach (QString keyvalue, metadata) {
            QStringList keyVal = keyvalue.split("=");
            if (keyVal.count() == 2)
                vars[keyVal[0]] = keyVal[1];
            else {
                m = QString("Malformed subject metadata string [%1]. Inconsistent key/value pair count").arg(objectData);
                return false;
            }
        }

        /* load the package */
        squirrel *sqrl = new squirrel();
        sqrl->SetFilename(packagePath);
        if (!sqrl->Read(true)) {
            m = QString("Package unreadable [%1] already exists in package").arg(vars["SubjectID"]);
            delete sqrl;
            return false;
        }

        /* --- add a subject --- */
        if (addObject == "subject") {
            int subjectRowID;
            subjectRowID = sqrl->FindSubject(vars["PatientID"]);
            if (subjectRowID < 0) {
                squirrelSubject subject;
                sqrl->Log(QString("Creating squirrel Subject [%1]").arg(vars["PatientID"]), __FUNCTION__);
                subject.ID = vars["SubjectID"];
                subject.AlternateIDs = vars["AlternateIDs"].split(",");
                subject.GUID = vars["GUID"];
                subject.DateOfBirth = QDate::fromString(vars["DateOfBirth"], "yyyy-MM-dd");
                subject.Sex = vars["Gender"];
                subject.Gender = vars["Gender"];
                subject.Ethnicity1 = vars["Ethnicity1"];
                subject.Ethnicity2 = vars["Ethnicity2"];
                subject.Store();
                subjectRowID = subject.GetObjectID();
                /* resequence the newly added subject */
                sqrl->ResequenceSubjects();
            }
            else {
                m = QString("Subject with ID [%1] already exists in package").arg(vars["SubjectID"]);
                delete sqrl;
                return false;
            }
        }
        else if (addObject == "study") {
            int subjectRowID = sqrl->FindSubject(subjectID);
            int studyRowID = sqrl->FindStudy(subjectID, vars["StudyNumber"].toInt());
            if (studyRowID < 0) {
                squirrelStudy study;
                sqrl->Log(QString("Creating squirrel Subject [%1]").arg(vars["PatientID"]), __FUNCTION__);
                study.StudyNumber = vars["StudyNumber"].toInt();
                study.DateTime = QDateTime::fromString(vars["Datetime"], "yyyy-MM-dd HH:mm:ss");
                study.AgeAtStudy = vars["AgeAtStudy"].toDouble();
                study.Modality = vars["Modality"];
                study.Height = vars["Height"].toDouble();
                study.Weight = vars["Weight"].toDouble();
                study.Description = vars["Description"];
                study.StudyUID = vars["StudyUID"];
                study.VisitType = vars["VisitType"];
                study.DayNumber = vars["DayNumber"].toInt();
                study.TimePoint = vars["TimePoint"].toInt();
                study.Equipment = vars["Equipment"];
                study.subjectRowID = subjectRowID;
                study.Store();
                studyRowID = study.GetObjectID();
                /* resequence the newly added subject */
                sqrl->ResequenceStudies(subjectRowID);
            }
            else {
                m = QString("Study with StudyNumber [%1] already exists for subject [%2] in package").arg(vars["StudyNumber"]).arg(subjectID);
                delete sqrl;
                return false;
            }
        }
        else if (addObject == "series") {
            int studyRowID = sqrl->FindStudy(subjectID, studyNum);
            int seriesRowID = sqrl->FindSeries(subjectID, studyNum, vars["SeriesNumber"].toInt());
            if (seriesRowID < 0) {
                squirrelSeries series;
                sqrl->Log(QString("Creating squirrel Series [%1]").arg(vars["SeriesNumber"]), __FUNCTION__);
                series.SeriesNumber = vars["StudyNumber"].toInt();
                series.DateTime = QDateTime::fromString(vars["Datetime"], "yyyy-MM-dd HH:mm:ss");
                series.Description = vars["Description"];
                series.Protocol = vars["Protocol"];
                series.SeriesUID = vars["SeriesUID"];
                series.stagedBehFiles = vars["StagedBehFiles"].split(",");
                series.stagedFiles = vars["StagedFiles"].split(",");
                series.studyRowID = studyRowID;
                series.Store();
                seriesRowID = series.GetObjectID();
                /* resequence the newly added subject */
                sqrl->ResequenceSeries(studyRowID);
            }
            else {
                m = QString("Series with SeriesNumber [%1] already exists for study [%2] and subject [%3] in package").arg(vars["SeriesNumber"]).arg(studyNum).arg(subjectID);
                delete sqrl;
                return false;
            }
        }
        else if (addObject == "measure") {
            int subjectRowID = sqrl->FindSubject(subjectID);
            if (subjectRowID < 0) {
                m = QString("Subject [%3] not found in package").arg(subjectID);
                delete sqrl;
                return false;
            }
            else {
                squirrelMeasure measure;
                sqrl->Log(QString("Creating squirrel Measure [%1]").arg(vars["MeasureName"]), __FUNCTION__);
                measure.DateEnd = QDateTime::fromString(vars["DateEnd"], "yyyy-MM-dd HH:mm:ss");
                measure.DateRecordCreate = QDateTime::fromString(vars["DateRecordCreate"], "yyyy-MM-dd HH:mm:ss");
                measure.DateRecordEntry = QDateTime::fromString(vars["DateRecordEntry"], "yyyy-MM-dd HH:mm:ss");
                measure.DateRecordModify = QDateTime::fromString(vars["DateRecordModify"], "yyyy-MM-dd HH:mm:ss");
                measure.DateStart = QDateTime::fromString(vars["DateStart"], "yyyy-MM-dd HH:mm:ss");
                measure.Description = vars["Description"];
                measure.Duration = vars["Duration"].toDouble();
                measure.InstrumentName = vars["InstrumentName"];
                measure.MeasureName = vars["MeasureName"];
                measure.Notes = vars["Notes"];
                measure.Rater = vars["Rater"];
                measure.Value = vars["Value"];
                measure.subjectRowID = subjectRowID;
                measure.Store();
            }
        }
        else if (addObject == "drug") {
            int subjectRowID = sqrl->FindSubject(subjectID);
            if (subjectRowID < 0) {
                m = QString("Subject [%3] not found in package").arg(subjectID);
                delete sqrl;
                return false;
            }
            else {
                squirrelDrug drug;
                sqrl->Log(QString("Creating squirrel Drug [%1]").arg(vars["DrugName"]), __FUNCTION__);
                drug.DateEnd = QDateTime::fromString(vars["DateEnd"], "yyyy-MM-dd HH:mm:ss");
                drug.DateRecordCreate = QDateTime::fromString(vars["DateRecordCreate"], "yyyy-MM-dd HH:mm:ss");
                drug.DateRecordEntry = QDateTime::fromString(vars["DateRecordEntry"], "yyyy-MM-dd HH:mm:ss");
                drug.DateRecordModify = QDateTime::fromString(vars["DateRecordModify"], "yyyy-MM-dd HH:mm:ss");
                drug.DateStart = QDateTime::fromString(vars["DateStart"], "yyyy-MM-dd HH:mm:ss");
                drug.Description = vars["Description"];
                drug.DoseAmount = vars["DoseAmount"].toDouble();
                drug.DoseFrequency = vars["DoseFrequency"];
                drug.DoseKey = vars["DoseKey"];
                drug.DoseString = vars["DoseString"];
                drug.DoseUnit = vars["DoseUnit"];
                drug.DrugClass = vars["DrugClass"];
                drug.DrugName = vars["DrugName"];
                drug.Notes = vars["Notes"];
                drug.Rater = vars["Rater"];
                drug.subjectRowID = subjectRowID;
                drug.Store();
            }
        }
        else if (addObject == "analysis") {
        }
        else if (addObject == "experiment") {
        }
        else if (addObject == "pipeline") {
        }
        else if (addObject == "groupanalysis") {
        }
        else if (addObject == "datadictionary") {
        }
        else {
            m = QString("Unrecognized object type [%1]").arg(addObject);
            delete sqrl;
            return false;
        }

        /* write the squirrel object */
        sqrl->Write(true);

        delete sqrl;
    }

    return true;
}
