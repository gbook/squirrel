/* ------------------------------------------------------------------------------
  Squirrel GUI squirrelModel.cpp
  Copyright (C) 2004 - 2026
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

#include "squirrelModel.h"
#include "utils.h"

/* skip empty values so the details pane does not fill up with blank rows */
static void Add(QVector<guiDetail> &d, const QString &name, const QString &value) {
    if (!value.trimmed().isEmpty())
        d.append({name, value});
}

static QString DateTimeStr(const QDateTime &dt) {
    return dt.isValid() ? dt.toString("yyyy-MM-dd HH:mm:ss") : QString();
}

QVector<guiDetail> guiSeries::details() const {
    QVector<guiDetail> d;
    Add(d, "Series number", QString::number(number));
    Add(d, "Description", description);
    Add(d, "Protocol", protocol);
    Add(d, "Date/time", DateTimeStr(dateTime));
    Add(d, "Files", QString::number(fileCount));
    Add(d, "Size", utils::HumanReadableSize(size));
    return d;
}

QVector<guiDetail> guiStudy::details() const {
    QVector<guiDetail> d;
    Add(d, "Study number", QString::number(number));
    Add(d, "Description", description);
    Add(d, "Modality", modality);
    Add(d, "Equipment", equipment);
    Add(d, "Visit type", visitType);
    Add(d, "Date/time", DateTimeStr(dateTime));
    if (ageAtStudy > 0.0)
        Add(d, "Age at study", QString::number(ageAtStudy, 'f', 1));
    Add(d, "Series", QString::number(series.size()));
    Add(d, "Analyses", QString::number(analysisCount));
    return d;
}

QVector<guiDetail> guiSubject::details() const {
    QVector<guiDetail> d;
    Add(d, "Subject ID", id);
    Add(d, "Sex", sex);
    Add(d, "Gender", gender);
    Add(d, "Enrollment group", enrollmentGroup);
    if (dateOfBirth.isValid())
        Add(d, "Date of birth", dateOfBirth.toString("yyyy-MM-dd"));
    Add(d, "Studies", QString::number(studies.size()));
    return d;
}

QVector<guiDetail> guiPackage::details() const {
    QVector<guiDetail> d;
    Add(d, "Package name", name);
    Add(d, "Path", path);
    Add(d, "Description", description);
    Add(d, "Package format", packageFormat);
    Add(d, "Data format", dataFormat);
    Add(d, "squirrel version", squirrelVersion);
    Add(d, "squirrel build", squirrelBuild);
    Add(d, "Created", DateTimeStr(dateTime));
    Add(d, "Subjects", QString::number(subjects.size()));
    Add(d, "Files", QString::number(fileCount));
    Add(d, "Unzipped size", utils::HumanReadableSize(unzipSize));
    Add(d, "Experiments", QString::number(experimentCount));
    Add(d, "Pipelines", QString::number(pipelineCount));
    Add(d, "Group analyses", QString::number(groupAnalysisCount));
    Add(d, "Data dictionaries", QString::number(dataDictionaryCount));
    Add(d, "License", license);
    Add(d, "Changes", changes);
    return d;
}
