/* ------------------------------------------------------------------------------
  Squirrel experiment.cpp
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

#include "squirrelExperiment.h"
#include "utils.h"

squirrelExperiment::squirrelExperiment()
{
}

/* ------------------------------------------------------------ */
/* ----- Get -------------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrelExperiment::Get
 * @return true if successful
 *
 * This function will attempt to load the experiment data from
 * the database. The experimentRowID must be set before calling
 * this function. If the object exists in the DB, it will return true.
 * Otherwise it will return false.
 */
bool squirrelExperiment::Get() {
    if (objectID < 0) {
        valid = false;
        err = "objectID is not set";
        return false;
    }
    q.prepare("select * from Experiment where ExperimentRowID = :id");
    q.bindValue(":id", objectID);
    SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
    if (q.size() > 0) {
        q.first();

        /* get the data */
        objectID = q.value("ExperimentRowID").toLongLong();
        experimentName = q.value("ExperimentName").toString();
        numFiles = q.value("NumFiles").toLongLong();
        size = q.value("NumFiles").toLongLong();
        virtualPath = q.value("VirtualPath").toString();
        packageRowID = q.value("PackageRowID").toLongLong();

        /* get any staged files */
        stagedFiles.clear();
        q.prepare("select * from StagedFiles where ObjectRowID = :id and ObjectType = :type");
        q.bindValue(":id", objectID);
        q.bindValue(":type", "experiment");
        SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
        if (q.size() > 0) {
            while (q.next()) {
                stagedFiles.append(q.value("StagedPath").toString());
            }
        }

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
 * @brief squirrelExperiment::Store
 * @return true if successful
 *
 * This function will attempt to load the experiment data from
 * the database. The experimentRowID must be set before calling
 * this function. If the object exists in the DB, it will return true.
 * Otherwise it will return false.
 */
bool squirrelExperiment::Store() {

    /* insert if the object doesn't exist ... */
    if (objectID < 0) {
        q.prepare("insert into Experiment (PackageRowID, ExperimentName, Size, NumFiles, VirtualPath) values (:packageid, :name, :size, :numfiles, :virtualpath)");
        q.bindValue(":packageid", packageRowID);
        q.bindValue(":name", experimentName);
        q.bindValue(":size", size);
        q.bindValue(":numfiles", numFiles);
        q.bindValue(":virtualPath", virtualPath);
        SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
        objectID = q.lastInsertId().toInt();
    }
    /* ... otherwise update */
    else {
        q.prepare("update Experiment set ");
        q.bindValue(":id", objectID);
        q.bindValue(":packageid", packageRowID);
        q.bindValue(":name", experimentName);
        q.bindValue(":size", size);
        q.bindValue(":numfiles", numFiles);
        q.bindValue(":virtualPath", virtualPath);
        SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
    }

    /* store any staged files */
    if (objectID >= 0) {
        /* delete previously staged files from the database */
        q.prepare("delete from StagedFiles where ObjectRowID = :id and ObjectType = :type");
        q.bindValue(":id", objectID);
        q.bindValue(":type", "experiment");
        SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);

        QString path;
        foreach (path, stagedFiles) {
            q.prepare("insert into StagedFiles (PackageRowID, ObjectRowID, ObjectType) values (:packageid, :id, :type)");
            q.bindValue(":packageid", packageRowID);
            q.bindValue(":id", objectID);
            q.bindValue(":type", "experiment");
            SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
        }
    }
    return true;
}


/* ------------------------------------------------------------ */
/* ----- ToJSON ----------------------------------------------- */
/* ------------------------------------------------------------ */
QJsonObject squirrelExperiment::ToJSON() {
    QJsonObject json;

    json["ExperimentName"] = experimentName;
    json["NumFiles"] = numFiles;
    json["Size"] = size;
    json["VirtualPath"] = virtualPath;

    return json;
}


/* ------------------------------------------------------------ */
/* ----- PrintExperiment -------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrelExperiment::PrintExperiment
 */
void squirrelExperiment::PrintExperiment() {

    Print("\t----- EXPERIMENT -----");
    Print(QString("\tExperimentName: %1").arg(experimentName));
    Print(QString("\tNumfiles: %1").arg(numFiles));
    Print(QString("\tSize: %1").arg(size));
    Print(QString("\tVirtualPath: %1").arg(virtualPath));
}
