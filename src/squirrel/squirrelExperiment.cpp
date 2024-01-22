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

void squirrelExperiment::experimentName(QString val) {
    q.prepare("select ExperimentName from experiment where ExperimentRowID = :id");
    q.bindValue(":id", val);
    SQLQuery(q, __FUNCTION__, __FILE__, __LINE__);
}

void squirrelExperiment::numFiles(qint64 val) {
}

void squirrelExperiment::size(qint64 val) {
}

void squirrelExperiment::virtualPath(QString val) {
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
