/* ------------------------------------------------------------------------------
  Squirrel study.cpp
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

#include "squirrelStudy.h"
#include "utils.h"
#include <iostream>
#include <exception>

/* ------------------------------------------------------------ */
/* ----- study ------------------------------------------------ */
/* ------------------------------------------------------------ */
squirrelStudy::squirrelStudy()
{
}


/* ------------------------------------------------------------ */
/* ----- Get -------------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrelStudy::Get
 * @return true if successful
 *
 * This function will attempt to load the study data from
 * the database. The studyRowID must be set before calling
 * this function. If the object exists in the DB, it will return true.
 * Otherwise it will return false.
 */
bool squirrelStudy::Get() {
    if (objectID < 0) {
        valid = false;
        err = "objectID is not set";
        return false;
    }

    QSqlQuery q;
    q.prepare("select * from Study where StudyRowID = :id");
    q.bindValue(":id", objectID);
    utils::SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
    if (q.size() > 0) {
        q.first();

        /* get the data */
        objectID = q.value("StudyRowID").toLongLong();
        subjectRowID = q.value("SubjectRowID").toLongLong();
        number = q.value("StudyNumber").toLongLong();
        dateTime = q.value("Datetime").toDateTime();
        ageAtStudy = q.value("Age").toDouble();
        height = q.value("Height").toDouble();
        weight = q.value("Weight").toDouble();
        modality = q.value("Modality").toString();
        description = q.value("Description").toString();
        studyUID = q.value("StudyUID").toString();
        visitType = q.value("VisitType").toString();
        dayNumber = q.value("DayNumber").toInt();
        timePoint = q.value("TimePoint").toInt();
        equipment = q.value("StudyRowID").toString();
        sequence = q.value("Sequence").toInt();
        virtualPath = q.value("VirtualPath").toString();

        valid = true;
        return true;
    }
    else {
        valid = false;
        err = "objectID not found in database";
        return false;
    }
}


/* ------------------------------------------------------------ */
/* ----- Store ------------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief squirrelStudy::Store
 * @return true if successful
 *
 * This function will attempt to load the study data from
 * the database. The studyRowID must be set before calling
 * this function. If the object exists in the DB, it will return true.
 * Otherwise it will return false.
 */
bool squirrelStudy::Store() {

    QSqlQuery q;

    /* insert if the object doesn't exist ... */
    if (objectID < 0) {
        /* get the next study number */
        q.prepare("select max(StudyNumber) 'Max' from Study where StudyRowID = :id");
        q.bindValue(":id", objectID);
        utils::SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
        number = q.value("Max").toInt() + 1;

        q.prepare("insert into Study (SubjectRowID, StudyNumber, Datetime, Age, Height, Weight, Modality, Description, StudyUID, VisitType, DayNumber, Timepoint, Equipment, Sequence, VirtualPath) values (:SubjectRowID, :StudyNumber, :Datetime, :Age, :Height, :Weight, :Modality, :Description, :StudyUID, :VisitType, :DayNumber, :Timepoint, :Equipment, :Sequence, :VirtualPath)");
        q.bindValue(":SubjectRowID", subjectRowID);
        q.bindValue(":StudyNumber", number);
        q.bindValue(":Datetime", dateTime);
        q.bindValue(":Age", ageAtStudy);
        q.bindValue(":Height", height);
        q.bindValue(":Weight", weight);
        q.bindValue(":Modality", modality);
        q.bindValue(":Description", description);
        q.bindValue(":StudyUID", studyUID);
        q.bindValue(":VisitType", visitType);
        q.bindValue(":DayNumber", dayNumber);
        q.bindValue(":Timepoint", timePoint);
        q.bindValue(":Equipment", equipment);
        q.bindValue(":Sequence", sequence);
        q.bindValue(":VirtualPath", virtualPath);
        utils::SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
        objectID = q.lastInsertId().toInt();
    }
    /* ... otherwise update */
    else {
        q.prepare("update Study set SubjectRowID = :SubjectRowID, StudyNumber = :StudyNumber, Datetime = :Datetime, Age = :Age, Height = :Height, Weight = :Weight, Modality = :Modality, Description = :Description, StudyUID = :StudyUID, VisitType = :VisitType, DayNumber = :DayNumber, Timepoint = :Timepoint, Equipment = :Equipment, Sequence = :Sequence, VirtualPath = :VirtualPath where StudyRowID = :id");
        q.bindValue(":id", objectID);
        q.bindValue(":SubjectRowID", subjectRowID);
        q.bindValue(":StudyNumber", number);
        q.bindValue(":Datetime", dateTime);
        q.bindValue(":Age", ageAtStudy);
        q.bindValue(":Height", height);
        q.bindValue(":Weight", weight);
        q.bindValue(":Modality", modality);
        q.bindValue(":Description", description);
        q.bindValue(":StudyUID", studyUID);
        q.bindValue(":VisitType", visitType);
        q.bindValue(":DayNumber", dayNumber);
        q.bindValue(":Timepoint", timePoint);
        q.bindValue(":Equipment", equipment);
        q.bindValue(":Sequence", sequence);
        q.bindValue(":VirtualPath", virtualPath);
        utils::SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
    }

    return true;
}


/* ------------------------------------------------------------ */
/* ----- addSeries -------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Add series to this study
 * @param s squirrelSeries to be added
 * @return true if series was added, false otherwise
 */
bool squirrelStudy::addSeries(squirrelSeries s) {
	/* check size of the series list before and after adding */
	qint64 size(0);
	size = seriesList.size();

	/* create a copy of the object before appending */
    seriesList.append(s);

    if (seriesList.size() > size)
        return true;
    else
        return false;
}


/* ------------------------------------------------------------ */
/* ----- addAnalysis ------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief Add an analysis to this study
 * @param a squirrelAnalysis to be added
 * @return true if analysis was added, false otherwise
 */
bool squirrelStudy::addAnalysis(squirrelAnalysis a) {

	/* check size of the series list before and after adding */
	qint64 size = analysisList.size();

	analysisList.append(a);

	if (analysisList.size() > size)
		return true;
	else
		return false;
}


/* ------------------------------------------------------------ */
/* ----- GetNextSeriesNumber ---------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Get the next series number for this study
 * @return the next series number
 */
qint64 squirrelStudy::GetNextSeriesNumber() {

    /* find the current highest series number */
    qint64 maxnum = 0;
    for (int i=0; i<seriesList.size(); i++)
        if (seriesList[i].number > maxnum)
            maxnum = seriesList[i].number;

    return maxnum+1;
}


/* ------------------------------------------------------------ */
/* ----- PrintStudy ------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Print study details
 */
void squirrelStudy::PrintStudy() {

    utils::Print("\t\t\t----- STUDY -----");
    utils::Print(QString("\t\t\tNumber: %1").arg(number));
    utils::Print(QString("\t\t\tDatetime: %1").arg(dateTime.toString("yyyy-MM-dd HH:mm:ss")));
    utils::Print(QString("\t\t\tAgeAtStudy: %1").arg(ageAtStudy));
    utils::Print(QString("\t\t\tHeight: %1 m").arg(height));
    utils::Print(QString("\t\t\tWeight: %1 kg").arg(weight));
    utils::Print(QString("\t\t\tModality: %1").arg(modality));
    utils::Print(QString("\t\t\tDescription: %1").arg(description));
    utils::Print(QString("\t\t\tstudy-uid: %1").arg(studyUID));
    utils::Print(QString("\t\t\tVisitType: %1").arg(visitType));
    utils::Print(QString("\t\t\tDayNumber: %1").arg(dayNumber));
    utils::Print(QString("\t\t\tTimePoint: %1").arg(timePoint));
    utils::Print(QString("\t\t\tEquipment: %1").arg(equipment));
    utils::Print(QString("\t\t\tVirtualPath: %1").arg(virtualPath));
}


/* ------------------------------------------------------------ */
/* ----- ToJSON ----------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Get a JSON object for this study
 * @return JSON object containing the study
 */
QJsonObject squirrelStudy::ToJSON() {
	QJsonObject json;

    json["Number"] = number;
    json["Datetime"] = dateTime.toString("yyyy-MM-dd HH:mm:ss");
    json["AgeAtStudy"] = ageAtStudy;
    json["Height"] = height;
    json["Weight"] = weight;
    json["Modality"] = modality;
    json["Description"] = description;
    json["StudyUID"] = studyUID;
    json["VisitType"] = visitType;
    json["DayNumber"] = dayNumber;
    json["TimePoint"] = timePoint;
    json["Equipment"] = equipment;
    json["VirtualPath"] = virtualPath;

    /* add all the series */
	QJsonArray JSONseries;
	for (int i=0; i<seriesList.size(); i++) {
		JSONseries.append(seriesList[i].ToJSON());
	}
    json["NumSeries"] = JSONseries.size();
	json["series"] = JSONseries;

    /* add all the analyses */
    QJsonArray JSONanalyses;
    for (int i=0; i<analysisList.size(); i++) {
        JSONanalyses.append(analysisList[i].ToJSON());
    }
    json["NumAnalyses"] = JSONanalyses.size();
    json["analyses"] = JSONanalyses;

	return json;
}
