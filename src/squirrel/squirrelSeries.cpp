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
	number = 0;
	dateTime = QDateTime::currentDateTime();
    seriesUID = "";
    description = "";
    protocol = "";
	numFiles = 0;
	size = 0;
	numBehFiles = 0;
	behSize = 0;
	//QHash<QString, QString> params; /*!< Hash containing experimental parameters. eg MR params */
	//QStringList stagedFiles; /*!< staged file list: list of raw files in their own directories before the package is zipped up */
	//QStringList stagedBehFiles; /*!< staged beh file list: list of raw files in their own directories before the package is zipped up */
	//QStringList experimentList; /*!< List of experiment names attached to this series */
	virtualPath = "";

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

    QJsonArray JSONexperiments;
    for (int i=0; i<experimentNames.size(); i++) {
        JSONexperiments.append(experimentNames[i]);
    }
	if (JSONexperiments.size() > 0)
        json["ExperimentNames"] = JSONexperiments;

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
