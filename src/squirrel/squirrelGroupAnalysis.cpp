/* ------------------------------------------------------------------------------
  Squirrel squirrelGroupAnalysis.cpp
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

#include "squirrelGroupAnalysis.h"
#include "utils.h"

squirrelGroupAnalysis::squirrelGroupAnalysis()
{

}


/* ------------------------------------------------------------ */
/* ----- ToJSON ----------------------------------------------- */
/* ------------------------------------------------------------ */
QJsonObject squirrelGroupAnalysis::ToJSON() {
    QJsonObject json;

    json["GroupAnalysisName"] = groupAnalysisName;
    json["Datetime"] = dateTime.toString("yyyy-MM-dd HH:mm:ss");
    json["Description"] = description;
    json["Notes"] = notes;
    json["NumFiles"] = numfiles;
    json["Size"] = size;
    json["VirtualPath"] = virtualPath;

    return json;
}


/* ------------------------------------------------------------ */
/* ----- PrintGroupAnalysis ----------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrelGroupAnalysis::PrintGroupAnalysis
 */
void squirrelGroupAnalysis::PrintGroupAnalysis() {

    utils::Print("\t----- GROUPANALYSIS ------");
    utils::Print(QString("\tGroupAnalysisName: %1").arg(groupAnalysisName));
    utils::Print(QString("\tDatetime: %1").arg(dateTime.toString("yyyy-MM-dd HH:mm:ss")));
    utils::Print(QString("\tDescription: %1").arg(description));
    utils::Print(QString("\tNotes: %1").arg(notes));
    utils::Print(QString("\tNumfiles: %1").arg(numfiles));
    utils::Print(QString("\tSize: %1").arg(size));
    utils::Print(QString("\tVirtualPath: %1").arg(virtualPath));

}
