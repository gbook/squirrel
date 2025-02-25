/* ------------------------------------------------------------------------------
  Squirrel extract.cpp
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

#include "extract.h"

extract::extract() {}

/* ---------------------------------------------------------------------------- */
/* ----- DoExtract ------------------------------------------------------------ */
/* ---------------------------------------------------------------------------- */
bool extract::DoExtract(QString packagePath, QString operation, QString objectType, QString dataPath, QString objectData, QString objectID, QString subjectID, int studyNum, QString &m) {

    if (operation == "add") {
        if (AddObject(packagePath, objectType, dataPath, objectData, objectID, subjectID, studyNum, m))
            return true;
        else
            return false;
    }
    else if (operation == "remove") {
        if (RemoveObject(packagePath, objectType, dataPath, objectData, objectID, subjectID, studyNum, m))
            return true;
        else
            return false;
    }
    else if (operation == "update") {
        if (UpdateObject(packagePath, objectType, dataPath, objectData, objectID, subjectID, studyNum, m))
            return true;
        else
            return false;
    }
    else if (operation == "splitbymodality") {
        if (SplitByModality(packagePath, objectType, dataPath, objectData, objectID, m))
            return true;
        else
            return false;
    }
    else {
        m = "Invalid operation [" + operation + "] specified";
        return false;
    }
}
