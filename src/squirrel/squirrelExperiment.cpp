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
/* ----- ToJSON ----------------------------------------------- */
/* ------------------------------------------------------------ */
QJsonObject squirrelExperiment::ToJSON() {
    QJsonObject json;

	json["experimentName"] = experimentName;
	json["numFiles"] = numFiles;
	json["size"] = size;

    return json;
}


/* ------------------------------------------------------------ */
/* ----- AddFiles --------------------------------------------- */
/* ------------------------------------------------------------ */
bool squirrelExperiment::AddFiles(QStringList files, QString destDir) {
    /* make sure the experiment name is not blank */
    if (experimentName == "") {
        return false;
    }
    /* copy this to the packageRoot/destDir directory */
    QString dir = QString("%1/%2").arg(packageroot).arg(destDir);
    foreach (QString f, files) {
        CopyFile(f, dir);
    }
    /* create the experiment path on disk and set the experiment path in  */
    //foreach

    return true;
}


/* ------------------------------------------------------------ */
/* ----- AddDir ----------------------------------------------- */
/* ------------------------------------------------------------ */
bool squirrelExperiment::AddDir(QString dir, QString destDir, bool recurse) {
    /* make sure the experiment name is not blank */
    if (experimentName == "") {
        return false;
    }

    return true;
}
