/* ------------------------------------------------------------------------------
  Squirrel explore.cpp
  Copyright (C) 2004 - 2025
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

/* ------------------------------------------------------------------------------
  Interactive 'squirrel explore' shell.

  The package is opened once via squirrel::Read(), which loads only the
  metadata manifest (squirrel.json) into an in-memory database - the large data
  files are NOT extracted. Every command below therefore just walks the
  already-loaded object lists, so navigation is instant.

  Commands (prototype):
    ls   [subjectID [studyNum]]            list subjects / studies / series
    info [subjectID [studyNum [seriesNum]]] show package/subject/study/series detail
    help                                   list commands
    quit | exit                            leave (Ctrl-D also works)

  Line editing (history, arrow keys, in-line editing) is provided by replxx.
  ------------------------------------------------------------------------------ */

#include <cstdio>
#include <cerrno>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QDir>

#include "replxx.hxx"
#include "explore.h"
#include "squirrel.h"
#include "squirrelSubject.h"
#include "squirrelStudy.h"
#include "squirrelSeries.h"

using Replxx = replxx::Replxx;

/* ----- small local helpers -------------------------------------------------- */
/* squirrelSubject/Study/Series have no default constructor (they need a database
   UUID), so rather than copy found objects out we return the index of the match
   within a caller-held list and let the caller reference it in place. */

static int IndexOfSubject(const QList<squirrelSubject> &subjects, const QString &id) {
    for (int i = 0; i < subjects.size(); ++i)
        if (subjects[i].ID.compare(id, Qt::CaseInsensitive) == 0)
            return i;
    return -1;
}

static int IndexOfStudy(const QList<squirrelStudy> &studies, int studyNum) {
    for (int i = 0; i < studies.size(); ++i)
        if (studies[i].StudyNumber == studyNum)
            return i;
    return -1;
}

static int IndexOfSeries(const QList<squirrelSeries> &series, qint64 seriesNum) {
    for (int i = 0; i < series.size(); ++i)
        if (series[i].SeriesNumber == seriesNum)
            return i;
    return -1;
}

static void PrintHelp() {
    printf("\n");
    printf("  Commands:\n");
    printf("    ls                              list all subjects\n");
    printf("    ls <subjectID>                  list studies for a subject\n");
    printf("    ls <subjectID> <studyNum>       list series for a study\n");
    printf("\n");
    printf("    info                            show package summary\n");
    printf("    info <subjectID>                show subject detail\n");
    printf("    info <subjectID> <studyNum>     show study detail\n");
    printf("    info <subjectID> <studyNum> <seriesNum>   show series detail\n");
    printf("\n");
    printf("    help                            show this help\n");
    printf("    quit | exit                     leave the shell (Ctrl-D works too)\n");
    printf("\n");
}

/* ----- ls ------------------------------------------------------------------- */

static void HandleLs(squirrel &sqrl, const QStringList &args) {
    /* ls -> subjects */
    if (args.isEmpty()) {
        QList<squirrelSubject> subjects = sqrl.GetSubjectList();
        if (subjects.isEmpty()) {
            printf("  (no subjects in package)\n");
            return;
        }
        printf("  %-20s %-6s %-16s %8s\n", "SubjectID", "Sex", "Group", "#Studies");
        printf("  %-20s %-6s %-16s %8s\n", "---------", "---", "-----", "--------");
        for (squirrelSubject &s : subjects) {
            int nStudies = sqrl.GetStudyList(s.GetObjectID()).size();
            printf("  %-20s %-6s %-16s %8d\n",
                   s.ID.toStdString().c_str(),
                   s.Sex.toStdString().c_str(),
                   s.EnrollmentGroup.toStdString().c_str(),
                   nStudies);
        }
        printf("\n  %d subject(s)\n", int(subjects.size()));
        return;
    }

    /* ls <subjectID> -> studies */
    QList<squirrelSubject> subjects = sqrl.GetSubjectList();
    int si = IndexOfSubject(subjects, args[0]);
    if (si < 0) {
        printf("  Subject '%s' not found. Try 'ls' to list subjects.\n", args[0].toStdString().c_str());
        return;
    }
    squirrelSubject &subj = subjects[si];

    if (args.size() == 1) {
        QList<squirrelStudy> studies = sqrl.GetStudyList(subj.GetObjectID());
        if (studies.isEmpty()) {
            printf("  (no studies for subject %s)\n", subj.ID.toStdString().c_str());
            return;
        }
        printf("  Studies for subject %s:\n", subj.ID.toStdString().c_str());
        printf("  %-8s %-10s %8s  %s\n", "StudyNum", "Modality", "#Series", "Description");
        printf("  %-8s %-10s %8s  %s\n", "--------", "--------", "-------", "-----------");
        for (squirrelStudy &st : studies) {
            int nSeries = sqrl.GetSeriesList(st.GetObjectID()).size();
            printf("  %-8d %-10s %8d  %s\n",
                   st.StudyNumber,
                   st.Modality.toStdString().c_str(),
                   nSeries,
                   st.Description.toStdString().c_str());
        }
        printf("\n  %d study(ies)\n", int(studies.size()));
        return;
    }

    /* ls <subjectID> <studyNum> -> series */
    bool ok = false;
    int studyNum = args[1].toInt(&ok);
    if (!ok) {
        printf("  Invalid study number '%s'.\n", args[1].toStdString().c_str());
        return;
    }
    QList<squirrelStudy> studies = sqrl.GetStudyList(subj.GetObjectID());
    int sti = IndexOfStudy(studies, studyNum);
    if (sti < 0) {
        printf("  Study %d not found for subject %s.\n", studyNum, subj.ID.toStdString().c_str());
        return;
    }
    squirrelStudy &study = studies[sti];
    const QList<squirrelSeries> series = sqrl.GetSeriesList(study.GetObjectID());
    if (series.isEmpty()) {
        printf("  (no series for subject %s study %d)\n", subj.ID.toStdString().c_str(), studyNum);
        return;
    }
    printf("  Series for subject %s, study %d:\n", subj.ID.toStdString().c_str(), studyNum);
    printf("  %-9s %-16s  %s\n", "SeriesNum", "Protocol", "Description");
    printf("  %-9s %-16s  %s\n", "---------", "--------", "-----------");
    for (const squirrelSeries &se : series) {
        printf("  %-9lld %-16s  %s\n",
               (long long)se.SeriesNumber,
               se.Protocol.toStdString().c_str(),
               se.Description.toStdString().c_str());
    }
    printf("\n  %d series\n", int(series.size()));
}

/* ----- info ----------------------------------------------------------------- */

static void HandleInfo(squirrel &sqrl, const QStringList &args) {
    /* info -> package summary (PrintPackage writes to stdout itself) */
    if (args.isEmpty()) {
        sqrl.PrintPackage();
        return;
    }

    QList<squirrelSubject> subjects = sqrl.GetSubjectList();
    int si = IndexOfSubject(subjects, args[0]);
    if (si < 0) {
        printf("  Subject '%s' not found. Try 'ls' to list subjects.\n", args[0].toStdString().c_str());
        return;
    }
    squirrelSubject &subj = subjects[si];

    /* info <subjectID> -> subject detail (PrintDetails writes to stdout) */
    if (args.size() == 1) {
        subj.PrintDetails();
        return;
    }

    bool ok = false;
    int studyNum = args[1].toInt(&ok);
    if (!ok) {
        printf("  Invalid study number '%s'.\n", args[1].toStdString().c_str());
        return;
    }
    QList<squirrelStudy> studies = sqrl.GetStudyList(subj.GetObjectID());
    int sti = IndexOfStudy(studies, studyNum);
    if (sti < 0) {
        printf("  Study %d not found for subject %s.\n", studyNum, subj.ID.toStdString().c_str());
        return;
    }
    squirrelStudy &study = studies[sti];

    /* info <subjectID> <studyNum> -> study detail */
    if (args.size() == 2) {
        printf("\n  ----- STUDY -----\n");
        printf("  Subject:     %s\n", subj.ID.toStdString().c_str());
        printf("  StudyNumber: %d\n", study.StudyNumber);
        printf("  Modality:    %s\n", study.Modality.toStdString().c_str());
        printf("  Description: %s\n", study.Description.toStdString().c_str());
        printf("  VisitType:   %s\n", study.VisitType.toStdString().c_str());
        printf("  AgeAtStudy:  %g\n", study.AgeAtStudy);
        printf("  #Series:     %d\n\n", int(sqrl.GetSeriesList(study.GetObjectID()).size()));
        return;
    }

    /* info <subjectID> <studyNum> <seriesNum> -> series detail */
    qint64 seriesNum = args[2].toLongLong(&ok);
    if (!ok) {
        printf("  Invalid series number '%s'.\n", args[2].toStdString().c_str());
        return;
    }
    QList<squirrelSeries> serieslist = sqrl.GetSeriesList(study.GetObjectID());
    int sei = IndexOfSeries(serieslist, seriesNum);
    if (sei < 0) {
        printf("  Series %lld not found for subject %s study %d.\n",
               (long long)seriesNum, subj.ID.toStdString().c_str(), studyNum);
        return;
    }
    squirrelSeries &series = serieslist[sei];
    printf("\n  ----- SERIES -----\n");
    printf("  Subject:      %s\n", subj.ID.toStdString().c_str());
    printf("  StudyNumber:  %d\n", study.StudyNumber);
    printf("  SeriesNumber: %lld\n", (long long)series.SeriesNumber);
    printf("  Protocol:     %s\n", series.Protocol.toStdString().c_str());
    printf("  Description:  %s\n", series.Description.toStdString().c_str());
    printf("  SeriesUID:    %s\n\n", series.SeriesUID.toStdString().c_str());
}

/* ----- main loop ------------------------------------------------------------ */

int explore::Run(const QString &packagePath) {
    /* Open the package (metadata only). quiet=true so Read() does not spew the
       normal library log into the interactive session. */
    squirrel sqrl(false, true);
    sqrl.SetPackagePath(packagePath);
    sqrl.SetFileMode(FileMode::ExistingPackage);
    if (!sqrl.Read()) {
        fprintf(stderr, "Could not open squirrel package '%s'. Is it a valid package?\n",
                packagePath.toStdString().c_str());
        return 1;
    }

    printf("\nsquirrel explore - %s\n", packagePath.toStdString().c_str());
    printf("Package: %s   Subjects: %d\n",
           sqrl.PackageName.isEmpty() ? "(unnamed)" : sqrl.PackageName.toStdString().c_str(),
           int(sqrl.GetSubjectList().size()));
    printf("Type 'help' for commands, 'quit' to exit.\n\n");

    /* set up replxx */
    Replxx rx;
    rx.install_window_change_handler();
    rx.set_max_history_size(1000);
    const QString historyFile = QDir::homePath() + "/.squirrel_explore_history";
    rx.history_load(historyFile.toStdString());

    const char *prompt = "squirrel> ";
    for (;;) {
        const char *cinput = nullptr;
        do {
            cinput = rx.input(prompt);
        } while ((cinput == nullptr) && (errno == EAGAIN)); /* transient (e.g. window resize) */

        if (cinput == nullptr) { /* EOF / Ctrl-D */
            printf("\n");
            break;
        }

        QString line = QString::fromUtf8(cinput).trimmed();
        if (line.isEmpty())
            continue;

        rx.history_add(cinput);

        QStringList tok = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        const QString cmd = tok.takeFirst().toLower(); /* remaining tok = args */

        if (cmd == "quit" || cmd == "exit" || cmd == "q") {
            break;
        }
        else if (cmd == "help" || cmd == "?") {
            PrintHelp();
        }
        else if (cmd == "ls") {
            HandleLs(sqrl, tok);
        }
        else if (cmd == "info") {
            HandleInfo(sqrl, tok);
        }
        else {
            printf("  Unknown command '%s'. Type 'help' for the list of commands.\n",
                   cmd.toStdString().c_str());
        }
    }

    rx.history_save(historyFile.toStdString());
    return 0;
}
