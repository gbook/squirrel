/* ------------------------------------------------------------------------------
  Squirrel subject.cpp
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

#include "squirrelSubject.h"
#include "utils.h"


/* ------------------------------------------------------------ */
/* ----- subject ---------------------------------------------- */
/* ------------------------------------------------------------ */
squirrelSubject::squirrelSubject() {

}


/* ------------------------------------------------------------ */
/* ----- Get -------------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrelSubject::Get
 * @return true if successful
 *
 * This function will attempt to load the subject data from
 * the database. The subjectRowID must be set before calling
 * this function. If the object exists in the DB, it will return true.
 * Otherwise it will return false.
 */
bool squirrelSubject::Get() {
    if (objectID < 0) {
        valid = false;
        err = "objectID is not set";
        return false;
    }

    QSqlQuery q;
    q.prepare("select * from Subject where SubjectRowID = :id");
    q.bindValue(":id", objectID);
    utils::SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
    if (q.size() > 0) {
        q.first();

        /* get the data */
        objectID = q.value("SubjectRowID").toLongLong();
        ID = q.value("ID").toString();
        alternateIDs = q.value("AltIDs").toString().split(",");
        GUID = q.value("GUID").toString();
        dateOfBirth = q.value("DateOfBirth").toDate();
        sex = q.value("Sex").toString();
        gender = q.value("Gender").toString();
        ethnicity1 = q.value("Ethnicity1").toString();
        ethnicity2 = q.value("Ethnicity2").toString();
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
 * @brief squirrelSubject::Store
 * @return true if successful
 *
 * This function will attempt to load the subject data from
 * the database. The subjectRowID must be set before calling
 * this function. If the object exists in the DB, it will return true.
 * Otherwise it will return false.
 */
bool squirrelSubject::Store() {

    QSqlQuery q;
    /* insert if the object doesn't exist ... */
    if (objectID < 0) {
        q.prepare("insert into Subject (ID, AltIDs, GUID, DateOfBirth, Sex, Gender, Ethnicity1, Ethnicity2, Sequence, VirtualPath) values (:ID, :AltIDs, :GUID, :DateOfBirth, :Sex, :Gender, :Ethnicity1, :Ethnicity2, :Sequence, :VirtualPath)");
        q.bindValue(":ID", ID);
        q.bindValue(":AltIDs", alternateIDs.join(","));
        q.bindValue(":GUID", GUID);
        q.bindValue(":DateOfBirth", dateOfBirth);
        q.bindValue(":Sex", sex);
        q.bindValue(":Gender", gender);
        q.bindValue(":Ethnicity1", ethnicity1);
        q.bindValue(":Ethnicity2", ethnicity2);
        q.bindValue(":Sequence", sequence);
        q.bindValue(":VirtualPath", virtualPath);
        utils::SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
        objectID = q.lastInsertId().toInt();
    }
    /* ... otherwise update */
    else {
        q.prepare("update Subject set ID = :ID, AltIDs = :AltIDs, GUID = :GUID, DateOfBirth = :DateOfBirth, Sex = :Sex, Gender = :Gender, Ethnicity1 = :Ethnicity1, Ethnicity2 = :Ethnicity2, Sequence = :Sequence, VirtualPath = :VirtualPath where SubjectRowID = :id");
        q.bindValue(":id", objectID);
        q.bindValue(":ID", ID);
        q.bindValue(":AltIDs", alternateIDs.join(","));
        q.bindValue(":GUID", GUID);
        q.bindValue(":DateOfBirth", dateOfBirth);
        q.bindValue(":Sex", sex);
        q.bindValue(":Gender", gender);
        q.bindValue(":Ethnicity1", ethnicity1);
        q.bindValue(":Ethnicity2", ethnicity2);
        q.bindValue(":Sequence", sequence);
        q.bindValue(":VirtualPath", virtualPath);
        utils::SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
    }

    return true;
}


// /* ------------------------------------------------------------ */
// /* ----- addStudy --------------------------------------------- */
// /* ------------------------------------------------------------ */
// /**
//  * @brief Add a study to this subject
//  * @param s squirrelStudy to be added
//  * @return true if added, false otherwise
//  */
// bool squirrelSubject::addStudy(squirrelStudy s) {

//     /* check size of the study list before and after adding */
//     qint64 size = studyList.size();

//     /* check if this study already exists, by UID */
//     bool exists = false;
//     for (int i=0; i<studyList.size(); i++)
//         if ((studyList[i].studyUID == s.studyUID) && (s.studyUID != ""))
//             exists = true;

//     /* if it doesn't exist, append it */
//     if (!exists)
//         studyList.append(s);

//     if (studyList.size() > size)
//         return true;
//     else
//         return false;
// }


// /* ------------------------------------------------------------ */
// /* ----- addMeasure ------------------------------------------- */
// /* ------------------------------------------------------------ */
// /**
//  * @brief Add a measure to this subject
//  * @param m squirrelMeasure to be added
//  * @return true if added, false otherwise
//  */
// bool squirrelSubject::addMeasure(squirrelMeasure m) {

// 	/* check size of the measure list before and after adding */
// 	qint64 size = measureList.size();

// 	/* check if this measure already exists, by UID */
// 	bool exists = false;
// 	//for (int i=0; i<studyList.size(); i++)
// 	//	if (studyList.at(i).studyUID == s.studyUID)
// 	//        exists = true;

// 	/* if it doesn't exist, append it */
// 	if (!exists)
// 		measureList.append(m);

// 	if (measureList.size() > size)
// 		return true;
// 	else
// 		return false;
// }


// /* ------------------------------------------------------------ */
// /* ----- addDrug ---------------------------------------------- */
// /* ------------------------------------------------------------ */
// /**
//  * @brief Add a drug to this subject
//  * @param d squirrelDrug to be added
//  * @return true if added, false otherwise
//  */
// bool squirrelSubject::addDrug(squirrelDrug d) {

// 	/* check size of the drug list before and after adding */
// 	qint64 size = drugList.size();

// 	/* check if this drug already exists, by UID */
// 	bool exists = false;
// 	//for (int i=0; i<studyList.size(); i++)
// 	//	if (studyList.at(i).studyUID == s.studyUID)
// 	//        exists = true;

// 	/* if it doesn't exist, append it */
// 	if (!exists)
//         drugList.append(d);

// 	if (drugList.size() > size)
// 		return true;
// 	else
// 		return false;
// }


/* ------------------------------------------------------------ */
/* ----- GetNextStudyNumber ----------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Gets the next study number for this subject
 * @return the next study number
 */
//qint64 squirrelSubject::GetNextStudyNumber() {

    /* find the current highest study number */
//    qint64 maxnum = 0;
//    for (int i=0; i<studyList.size(); i++)
//        if (studyList[i].number > maxnum)
//            maxnum = studyList[i].number;

//    return maxnum+1;
//}


/* ------------------------------------------------------------ */
/* ----- PrintSubject ----------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Print subject details
 */
void squirrelSubject::PrintSubject() {

    utils::Print("\t\t----- SUBJECT -----");
    utils::Print(QString("\t\tSubjectID: %1").arg(ID));
    utils::Print(QString("\t\tAlternateIDs: %1").arg(alternateIDs.join(",")));
    utils::Print(QString("\t\tGUID: %1").arg(GUID));
    utils::Print(QString("\t\tSex: %1").arg(sex));
    utils::Print(QString("\t\tGender: %1").arg(gender));
    utils::Print(QString("\t\tDateOfBirth: %1").arg(dateOfBirth.toString()));
    utils::Print(QString("\t\tEthnicity1: %1").arg(ethnicity1));
    utils::Print(QString("\t\tEthnicity2: %1").arg(ethnicity2));
    utils::Print(QString("\t\tVirtualPath: %1").arg(ethnicity2));
}


/* ------------------------------------------------------------ */
/* ----- ToJSON ----------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Get JSON object for this subject
 * @return a JSON object containing the entire subject
 */
QJsonObject squirrelSubject::ToJSON() {
    QJsonObject json;

    json["SubjectID"] = ID;
    json["AlternateIDs"] = QJsonArray::fromStringList(alternateIDs);
    json["GUID"] = GUID;
    json["DateOfBirth"] = dateOfBirth.toString("yyyy-MM-dd");
    json["Sex"] = sex;
    json["Gender"] = gender;
    json["Ethnicity1"] = ethnicity1;
    json["Ethnicity2"] = ethnicity2;
    json["VirtualPath"] = virtualPath;

  //   QJsonArray JSONstudies;
  //   for (int i=0; i<studyList.size(); i++) {
  //       JSONstudies.append(studyList[i].ToJSON());
  //   }
  //   json["NumStudies"] = JSONstudies.size();
  //   json["studies"] = JSONstudies;

  //   /* add measures */
  //   if (measureList.size() > 0) {
  //       QJsonArray JSONmeasures;
  //       for (int i=0; i < measureList.size(); i++) {
  //           JSONmeasures.append(measureList[i].ToJSON());
  //       }
  //       json["NumMeasures"] = JSONmeasures.size();
        // json["measures"] = JSONmeasures;
  //   }

  //   /* add drugs */
  //   if (drugList.size() > 0) {
  //       QJsonArray JSONdrugs;
  //       for (int i=0; i < drugList.size(); i++) {
  //           JSONdrugs.append(drugList[i].ToJSON());
  //       }
  //       json["NumDrugs"] = JSONdrugs.size();
        // json["drugs"] = JSONdrugs;
  //   }

    return json;
}
