/* ------------------------------------------------------------------------------
  Squirrel GUI squirrelModel.h
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

#ifndef SQUIRRELMODEL_H
#define SQUIRRELMODEL_H

#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QVector>

/* ------------------------------------------------------------------------------
   Plain snapshot structs describing an open package.

   The squirrel object itself owns a QSqlDatabase connection, and QSqlDatabase is
   bound to the thread that opened it. The squirrel instance therefore never
   leaves squirrelWorker's thread; instead the worker walks the package once and
   emits one of these snapshots, which the GUI thread is free to read. Anything
   the GUI needs to display has to be copied in here - do not hand a
   squirrelSubject (or any object holding a rowID it will later query) across the
   thread boundary.
   ------------------------------------------------------------------------------ */

/* one name/value row shown in the details pane */
struct guiDetail {
    QString name;
    QString value;
};

struct guiSeries {
    qint64 rowID = -1;
    qint64 number = -1;
    QString description;
    QString protocol;
    QDateTime dateTime;
    qint64 fileCount = 0;
    qint64 size = 0;

    QVector<guiDetail> details() const;
};

struct guiStudy {
    qint64 rowID = -1;
    int number = -1;
    QString description;
    QString modality;
    QString equipment;
    QString visitType;
    QDateTime dateTime;
    double ageAtStudy = 0.0;
    QList<guiSeries> series;
    int analysisCount = 0;

    QVector<guiDetail> details() const;
};

struct guiSubject {
    qint64 rowID = -1;
    QString id;
    QString sex;
    QString gender;
    QString enrollmentGroup;
    QDate dateOfBirth;
    QList<guiStudy> studies;

    QVector<guiDetail> details() const;
};

struct guiPackage {
    QString path;
    QString name;
    QString description;
    QString dataFormat;
    QString packageFormat;
    QString squirrelVersion;
    QString squirrelBuild;
    QString license;
    QString readme;
    QString changes;
    QDateTime dateTime;
    qint64 fileCount = 0;
    qint64 unzipSize = 0;
    QList<guiSubject> subjects;

    /* counts of objects the tree does not expand, shown in the details pane */
    int experimentCount = 0;
    int pipelineCount = 0;
    int groupAnalysisCount = 0;
    int dataDictionaryCount = 0;

    bool isOpen = false;

    QVector<guiDetail> details() const;
};

Q_DECLARE_METATYPE(guiPackage)

#endif // SQUIRRELMODEL_H
