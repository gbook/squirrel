/* ------------------------------------------------------------------------------
  Squirrel drug.cpp
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

#include "squirrelDrug.h"
#include "utils.h"

squirrelDrug::squirrelDrug()
{

}


/* ------------------------------------------------------------ */
/* ----- ToJSON ----------------------------------------------- */
/* ------------------------------------------------------------ */
QJsonObject squirrelDrug::ToJSON() {
    QJsonObject json;

	json["drugName"] = drugName;
	json["dateStart"] = dateStart.toString("yyyy-MM-dd HH:mm:ss");
	json["dateEnd"] = dateEnd.toString("yyyy-MM-dd HH:mm:ss");
	json["doseAmount"] = doseAmount;
	json["doseFrequency"] = doseFrequency;
	json["route"] = route;
    json["DrugClass"] = drugClass;
    json["DoseKey"] = doseKey;
    json["DoseUnit"] = doseUnit;
    json["FrequencyModifier"] = frequencyModifier;
    json["FrequencyValue"] = frequencyValue;
    json["FrequencyUnit"] = frequencyUnit;
    json["Description"] = description;
    json["Rater"] = rater;
    json["Notes"] = notes;
    json["DateRecordEntry"] = dateRecordEntry.toString("yyyy-MM-dd HH:mm:ss");

    return json;
}


/* ------------------------------------------------------------ */
/* ----- PrintDrug -------------------------------------------- */
/* ------------------------------------------------------------ */
/**
 * @brief squirrelDrug::PrintDrug
 */
void squirrelDrug::PrintDrug() {

    utils::Print("\t\t\t----- DRUG -----");
    utils::Print(QString("\t\t\tDrugName: %1").arg(drugName));
    utils::Print(QString("\t\t\tDateStart: %1").arg(dateStart.toString("yyyy-MM-dd HH:mm:ss")));
    utils::Print(QString("\t\t\tDateEnd: %1").arg(dateEnd.toString("yyyy-MM-dd HH:mm:ss")));
    utils::Print(QString("\t\t\tDoseAmount: %1").arg(doseAmount));
    utils::Print(QString("\t\t\tDoseFrequency: %1").arg(doseFrequency));
    utils::Print(QString("\t\t\tRoute: %1").arg(route));
    utils::Print(QString("\t\t\tDrugClass: %1").arg(drugClass));
    utils::Print(QString("\t\t\tDoseKey: %1").arg(doseKey));
    utils::Print(QString("\t\t\tDoseUnit: %1").arg(doseUnit));
    utils::Print(QString("\t\t\tFrequencyModifier: %1").arg(frequencyModifier));
    utils::Print(QString("\t\t\tFrequencyValue: %1").arg(frequencyValue));
    utils::Print(QString("\t\t\tFrequencyUnit: %1").arg(frequencyUnit));
    utils::Print(QString("\t\t\tDescription: %1").arg(description));
    utils::Print(QString("\t\t\tRater: %1").arg(rater));
    utils::Print(QString("\t\t\tNotes: %1").arg(notes));
    utils::Print(QString("\t\t\tDateRecordEntry: %1").arg(dateRecordEntry.toString("yyyy-MM-dd HH:mm:ss")));

}
