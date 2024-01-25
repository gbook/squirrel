/* ------------------------------------------------------------------------------
  Squirrel series.cpp
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

#include "squirrelSeries.h"
#include "utils.h"

squirrelSeries::squirrelSeries()
{

}

/* ------------------------------------------------------------ */
/* ----- Get -------------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrelSeries::Get
 * @return true if successful
 *
 * This function will attempt to load the series data from
 * the database. The seriesRowID must be set before calling
 * this function. If the object exists in the DB, it will return true.
 * Otherwise it will return false.
 */
bool squirrelSeries::Get() {
    if (objectID < 0) {
        valid = false;
        err = "objectID is not set";
        return false;
    }
    q.prepare("select * from Series where SeriesRowID = :id");
    q.bindValue(":id", objectID);
    utils::SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
    if (q.size() > 0) {
        q.first();

        /* get the data */
        objectID = q.value("SeriesRowID").toLongLong();
        studyRowID = q.value("StudyRowID").toLongLong();
        number = q.value("SeriesNum").toLongLong();
        dateTime = q.value("Datetime").toDateTime();
        seriesUID = q.value("SeriesUID").toString();
        description = q.value("Description").toString();
        protocol = q.value("Protocol").toString();
        //QStringList experimentNames = q.value("SeriesRowID").toString();
        numFiles = q.value("NumFiles").toLongLong();
        size = q.value("Size").toLongLong();
        numBehFiles = q.value("BehNumFiles").toLongLong();
        behSize = q.value("BehSize").toLongLong();
        //QHash<QString, QString> params = q.value("SeriesRowID").toLongLong();
        virtualPath = q.value("VirtualPath").toString();

        /* get any staged files */
        stagedFiles = utils::GetStagedFileList(objectID, "series");

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
 * @brief squirrelSeries::Store
 * @return true if successful
 *
 * This function will attempt to load the series data from
 * the database. The seriesRowID must be set before calling
 * this function. If the object exists in the DB, it will return true.
 * Otherwise it will return false.
 */
bool squirrelSeries::Store() {

    /* insert if the object doesn't exist ... */
    if (objectID < 0) {
        q.prepare("insert into Series (StudyRowID, SeriesNum, Datetime, SeriesUID, Description, Protocol, ExperimentRowID, Size, NumFiles, BehSize, BehNumFiles, VirtualPath) values (:StudyRowID, :SeriesNum, :Datetime, :SeriesUID, :Description, :Protocol, :ExperimentRowID, :Size, :NumFiles, :BehSize, :BehNumFiles, :VirtualPath)");
        q.bindValue(":StudyRowID", objectID);
        q.bindValue(":SeriesNum", objectID);
        q.bindValue(":Datetime", objectID);
        q.bindValue(":SeriesUID", objectID);
        q.bindValue(":Description", objectID);
        q.bindValue(":Protocol", objectID);
        q.bindValue(":ExperimentRowID", objectID);
        q.bindValue(":Size", objectID);
        q.bindValue(":NumFiles", objectID);
        q.bindValue(":BehSize", objectID);
        q.bindValue(":BehNumFiles", objectID);
        q.bindValue(":VirtualPath", virtualPath);
        utils::SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
        objectID = q.lastInsertId().toInt();
    }
    /* ... otherwise update */
    else {
        q.prepare("update Series set StudyRowID = :StudyRowID, SeriesNum = :SeriesNum, Datetime = :Datetime, SeriesUID = :SeriesUID, Description = :Description, Protocol = :Protocol, ExperimentRowID = :ExperimentRowID, Size = :Size, NumFiles = :NumFiles, BehSize = :BehSize, BehNumFiles = :BehNumFiles, VirtualPath = :VirtualPath where SeriesRowID = :id");
        q.bindValue(":StudyRowID", objectID);
        q.bindValue(":SeriesNum", objectID);
        q.bindValue(":Datetime", objectID);
        q.bindValue(":SeriesUID", objectID);
        q.bindValue(":Description", objectID);
        q.bindValue(":Protocol", objectID);
        q.bindValue(":ExperimentRowID", objectID);
        q.bindValue(":Size", objectID);
        q.bindValue(":NumFiles", objectID);
        q.bindValue(":BehSize", objectID);
        q.bindValue(":BehNumFiles", objectID);
        q.bindValue(":VirtualPath", virtualPath);
        q.bindValue(":id", objectID);
        utils::SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
    }

    /* store any staged filepaths */
    utils::StoreStagedFileList(objectID, "series", stagedFiles);

    return true;
}


/* ------------------------------------------------------------ */
/* ----- PrintSeries ------------------------------------------ */
/* ------------------------------------------------------------ */
/**
 * @brief Print the series details
 */
void squirrelSeries::PrintSeries() {
    utils::Print("\t\t\t\t----- SERIES -----");
    utils::Print(QString("\t\t\t\tSeriesNumber: %1").arg(number));
    utils::Print(QString("\t\t\t\tSeriesDatetime: %1").arg(dateTime.toString("yyyy-MM-dd HH:mm:ss")));
    utils::Print(QString("\t\t\t\tSeriesUID: %1").arg(seriesUID));
    utils::Print(QString("\t\t\t\tDescription: %1").arg(description));
    utils::Print(QString("\t\t\t\tProtocol: %1").arg(protocol));
    utils::Print(QString("\t\t\t\tExperimentNames: %1").arg(experimentNames.join(",")));
    utils::Print(QString("\t\t\t\tSize: %1").arg(size));
    utils::Print(QString("\t\t\t\tNumFiles: %1").arg(numFiles));
    utils::Print(QString("\t\t\t\tNumBehFiles: %1").arg(numBehFiles));
    utils::Print(QString("\t\t\t\tBehSize: %1").arg(behSize));
}


/* ------------------------------------------------------------ */
/* ----- ToJSON ----------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Get a JSON object for the entire series
 * @return JSON object
 */
QJsonObject squirrelSeries::ToJSON() {
    QJsonObject json;

    json["SeriesNumber"] = number;
    json["SeriesDatetime"] = dateTime.toString("yyyy-MM-dd HH:mm:ss");
    json["SeriesUID"] = seriesUID;
    json["Description"] = description;
    json["Protocol"] = protocol;
    json["NumFiles"] = numFiles;
    json["Size"] = size;
    json["NumBehFiles"] = numBehFiles;
    json["BehSize"] = behSize;
    json["VirtualPath"] = virtualPath;

    /* experiments */
    //QJsonArray JSONseriess;
    //for (int i=0; i<seriesNames.size(); i++) {
    //    JSONseriess.append(seriesNames[i]);
    //}
    //if (JSONseriess.size() > 0)
    //    json["SeriesNames"] = JSONseriess;

    return json;
}


/* ------------------------------------------------------------ */
/* ----- ParamsToJSON ----------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief Get series params in JSON format, likely MRI sequence params
 * @return JSON object containing series params
 */
QJsonObject squirrelSeries::ParamsToJSON() {
	QJsonObject json;

	for(QHash<QString, QString>::iterator a = params.begin(); a != params.end(); ++a) {
		json[a.key()] = a.value();
	}

	return json;
}
