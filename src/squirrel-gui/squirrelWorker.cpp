/* ------------------------------------------------------------------------------
  Squirrel GUI squirrelWorker.cpp
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

#include "squirrelWorker.h"

#include <QFileInfo>

#include "modify.h"
#include "squirrel.h"
#include "squirrelTypes.h"

squirrelWorker::squirrelWorker(QObject *parent) : QObject(parent) {
}


squirrelWorker::~squirrelWorker() {
    /* the worker is deleted via deleteLater() on its own thread, so the squirrel
       object (and its database connection) is torn down where it was created */
    delete sqrl;
    sqrl = nullptr;
}


/* ------------------------------------------------------------------------------
   DrainLog - move anything the library logged into the GUI's log pane
   ------------------------------------------------------------------------------ */
void squirrelWorker::DrainLog() {
    if (sqrl == nullptr)
        return;

    QString buffer = sqrl->GetLogBuffer();
    if (!buffer.trimmed().isEmpty())
        emit LogMessage(buffer.trimmed());
}


/* ------------------------------------------------------------------------------
   OpenPackage - read an existing squirrel package and snapshot it for the GUI
   ------------------------------------------------------------------------------ */
void squirrelWorker::OpenPackage(QString packagePath) {

    emit OperationStarted(QString("Opening %1 ...").arg(QFileInfo(packagePath).fileName()));

    if (!QFileInfo::exists(packagePath)) {
        emit OperationFinished(false, QString("Package [%1] does not exist").arg(packagePath));
        return;
    }

    /* close anything already open before replacing it */
    delete sqrl;
    sqrl = nullptr;
    currentPath.clear();

    /* quiet=true keeps the library from writing to stdout, which nothing is
       attached to in a GUI. Everything worth showing comes back via GetLogBuffer() */
    sqrl = new squirrel(false, true);
    sqrl->quiet = true;
    sqrl->SetPackagePath(packagePath);
    sqrl->SetFileMode(FileMode::ExistingPackage);

    /* QuickRead skips reading the file lists inside the archive. It is the
       difference between a package opening in seconds and in minutes; the GUI
       shows counts from the database instead. */
    sqrl->SetQuickRead(true);
    sqrl->Read();

    DrainLog();

    if (!sqrl->IsValid()) {
        QString err = QString("[%1] is not a valid squirrel package").arg(packagePath);
        delete sqrl;
        sqrl = nullptr;
        emit PackageLoaded(guiPackage());
        emit OperationFinished(false, err);
        return;
    }

    currentPath = packagePath;

    guiPackage pkg = BuildSnapshot();
    DrainLog();

    emit PackageLoaded(pkg);
    emit OperationFinished(true, QString("Opened %1 (%2 subject(s))").arg(packagePath).arg(pkg.subjects.size()));
}


/* ------------------------------------------------------------------------------
   ClosePackage
   ------------------------------------------------------------------------------ */
void squirrelWorker::ClosePackage() {

    if (sqrl == nullptr) {
        emit OperationFinished(true, "");
        return;
    }

    emit OperationStarted("Closing package ...");

    delete sqrl;
    sqrl = nullptr;
    currentPath.clear();

    emit PackageLoaded(guiPackage());
    emit OperationFinished(true, "Package closed");
}


/* ------------------------------------------------------------------------------
   ValidatePackage
   ------------------------------------------------------------------------------ */
void squirrelWorker::ValidatePackage() {

    if (sqrl == nullptr) {
        emit OperationFinished(false, "No package is open");
        return;
    }

    emit OperationStarted("Validating package ...");

    bool ok = sqrl->Validate();
    DrainLog();

    emit OperationFinished(ok, ok ? "Package is valid" : "Package failed validation - see log for details");
}


/* ------------------------------------------------------------------------------
   MergePackages - the same modify::MergePackages() the command line 'squirrel
   merge' calls
   ------------------------------------------------------------------------------ */
void squirrelWorker::MergePackages(QStringList inputPaths, QString outputPath, bool testOnly, bool renumberSubjects, int digits) {

    emit OperationStarted(testOnly ? "Testing merge ..." : "Merging packages ...");

    QString m;
    modify mod;
    bool ok = mod.MergePackages(inputPaths, outputPath, testOnly, renumberSubjects, digits, m);

    if (!m.trimmed().isEmpty())
        emit LogMessage(m.trimmed());

    if (ok && !testOnly)
        emit OperationFinished(true, QString("Merged %1 package(s) into %2").arg(inputPaths.size()).arg(outputPath));
    else if (ok)
        emit OperationFinished(true, "Merge test completed - see log for details");
    else
        emit OperationFinished(false, "Merge failed - see log for details");
}


/* ------------------------------------------------------------------------------
   BuildSnapshot - copy everything the GUI displays out of the library objects

   Runs on the worker thread; every field is copied by value so the returned
   struct is safe to hand to the GUI thread.
   ------------------------------------------------------------------------------ */
guiPackage squirrelWorker::BuildSnapshot() {

    guiPackage pkg;
    if (sqrl == nullptr)
        return pkg;

    pkg.isOpen = true;
    pkg.path = currentPath;
    pkg.name = sqrl->PackageName;
    pkg.description = sqrl->Description;
    pkg.dataFormat = sqrl->DataFormat;
    pkg.packageFormat = sqrl->PackageFormat;
    pkg.squirrelVersion = sqrl->SquirrelVersion;
    pkg.squirrelBuild = sqrl->SquirrelBuild;
    pkg.license = sqrl->License;
    pkg.readme = sqrl->Readme;
    pkg.changes = sqrl->Changes;
    pkg.dateTime = sqrl->Datetime;
    pkg.fileCount = sqrl->GetFileCount();
    pkg.unzipSize = sqrl->GetUnzipSize();

    pkg.experimentCount = sqrl->GetObjectCount(ObjectType::Experiment);
    pkg.pipelineCount = sqrl->GetObjectCount(ObjectType::Pipeline);
    pkg.groupAnalysisCount = sqrl->GetObjectCount(ObjectType::GroupAnalysis);
    pkg.dataDictionaryCount = sqrl->GetObjectCount(ObjectType::DataDictionary);

    QList<squirrelSubject> subjects = sqrl->GetSubjectList();
    for (squirrelSubject &subject : subjects) {

        guiSubject gsubject;
        gsubject.rowID = subject.GetObjectID();
        gsubject.id = subject.ID;
        gsubject.sex = subject.Sex;
        gsubject.gender = subject.Gender;
        gsubject.enrollmentGroup = subject.EnrollmentGroup;
        gsubject.dateOfBirth = subject.DateOfBirth;

        QList<squirrelStudy> studies = sqrl->GetStudyList(gsubject.rowID);
        for (squirrelStudy &study : studies) {

            guiStudy gstudy;
            gstudy.rowID = study.GetObjectID();
            gstudy.number = study.StudyNumber;
            gstudy.description = study.Description;
            gstudy.modality = study.Modality;
            gstudy.equipment = study.Equipment;
            gstudy.visitType = study.VisitType;
            gstudy.dateTime = study.DateTime;
            gstudy.ageAtStudy = study.AgeAtStudy;
            gstudy.analysisCount = sqrl->GetAnalysisList(gstudy.rowID).size();

            QList<squirrelSeries> seriesList = sqrl->GetSeriesList(gstudy.rowID);
            for (squirrelSeries &series : seriesList) {

                guiSeries gseries;
                gseries.rowID = series.GetObjectID();
                gseries.number = series.SeriesNumber;
                gseries.description = series.Description;
                gseries.protocol = series.Protocol;
                gseries.dateTime = series.DateTime;
                gseries.fileCount = series.FileCount;
                gseries.size = series.Size;

                gstudy.series.append(gseries);
            }

            gsubject.studies.append(gstudy);
        }

        pkg.subjects.append(gsubject);
    }

    return pkg;
}
