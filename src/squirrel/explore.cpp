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
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <unistd.h>

#include <QString>
#include <QStringList>
#include <QRegularExpression>

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

#include "explore.h"
#include "squirrel.h"
#include "squirrelSubject.h"
#include "squirrelStudy.h"
#include "squirrelSeries.h"

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

/* ----- stdout capture ------------------------------------------------------- */
/* The ls/info handlers above (and the library's PrintPackage/PrintDetails) write
   with printf. In the FTXUI full-screen UI we must not let that reach the real
   terminal, so we temporarily redirect fd 1 to a temp file, run the handler,
   then read the captured text back to append into the transcript pane. A temp
   file (not a pipe) avoids any buffer-full deadlock on large output. */
static std::string CaptureStdout(const std::function<void()> &fn) {
    fflush(stdout);
    FILE *tmp = tmpfile();
    if (!tmp) { fn(); return std::string(); }
    int saved = dup(STDOUT_FILENO);
    dup2(fileno(tmp), STDOUT_FILENO);

    fn();

    fflush(stdout);
    dup2(saved, STDOUT_FILENO);
    close(saved);

    fseek(tmp, 0, SEEK_END);
    long n = ftell(tmp);
    fseek(tmp, 0, SEEK_SET);
    std::string out;
    if (n > 0) {
        out.resize(size_t(n));
        size_t got = fread(&out[0], 1, size_t(n), tmp);
        out.resize(got);
    }
    fclose(tmp);
    return out;
}

/* ----- main loop (FTXUI full-screen UI) ------------------------------------- */

int explore::Run(const QString &packagePath) {
    using namespace ftxui;

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

    const QString pkgName = sqrl.PackageName.isEmpty() ? QString("(unnamed)") : sqrl.PackageName;
    const int nSubjects = int(sqrl.GetSubjectList().size());

    /* transcript: the scrolling history of commands and their output */
    std::vector<std::string> transcript;
    auto addLine = [&](const std::string &block) {
        /* split into individual lines so the scroll pane measures height right */
        std::string cur;
        for (char ch : block) {
            if (ch == '\n') { transcript.push_back(cur); cur.clear(); }
            else if (ch != '\r') cur.push_back(ch);
        }
        if (!cur.empty()) transcript.push_back(cur);
    };
    addLine("Type 'help' for commands, 'quit' to exit.");
    addLine("");

    auto screen = ScreenInteractive::Fullscreen();

    /* Scrollback state. 'scroll' is how many lines we are scrolled UP from the
       bottom (0 == pinned to the newest line). 'paneBox'/'contentHeight' are
       filled in during render so the event handler can clamp and page. Any new
       output snaps back to the bottom (scroll = 0), like a normal shell. */
    int scroll = 0;
    int contentHeight = 0;
    int viewH = 0;       /* visible rows in the transcript pane (measured) */
    Box paneBox;

    /* command history (recalled with plain Up/Down). histPos indexes cmdHistory;
       histPos == cmdHistory.size() means "editing a fresh line", whose partial
       text is stashed in 'draft' so Down can bring it back. */
    std::vector<std::string> cmdHistory;
    int histPos = 0;
    std::string draft;

    /* the command bar: a single-line Input at the bottom. cursorPos is bound to
       the Input so we can move the caret to end-of-line when recalling history. */
    std::string command;
    int cursorPos = 0;
    InputOption opt = InputOption::Default();
    opt.content = &command;
    opt.cursor_position = &cursorPos;
    opt.placeholder = "type a command  (help, ls, info, quit)";
    opt.multiline = false;
    opt.on_enter = [&] {
        QString line = QString::fromUtf8(command.c_str()).trimmed();
        command.clear();
        cursorPos = 0;
        if (line.isEmpty())
            return;

        /* record in history (skip consecutive duplicates); reset the cursor */
        std::string cmdStr = line.toStdString();
        if (cmdHistory.empty() || cmdHistory.back() != cmdStr)
            cmdHistory.push_back(cmdStr);
        histPos = int(cmdHistory.size());
        draft.clear();

        scroll = 0; /* running a command snaps the view back to the bottom */
        addLine("> " + line.toStdString());

        QStringList tok = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        const QString cmd = tok.takeFirst().toLower();

        if (cmd == "quit" || cmd == "exit" || cmd == "q") {
            screen.Exit();
            return;
        }
        else if (cmd == "help" || cmd == "?") {
            addLine(CaptureStdout([&] { PrintHelp(); }));
        }
        else if (cmd == "ls") {
            addLine(CaptureStdout([&] { HandleLs(sqrl, tok); }));
        }
        else if (cmd == "info") {
            addLine(CaptureStdout([&] { HandleInfo(sqrl, tok); }));
        }
        else {
            addLine("  Unknown command '" + cmd.toStdString() +
                 "'. Type 'help' for the list of commands.");
        }
        addLine("");
    };
    auto input = Input(opt);

    /* compose: title / transcript (scrollable) / command box.

       The transcript is scrolled by explicit windowing: we measure the pane
       height (viewH) via reflect(), then render only the slice of lines the
       window should show. 'scroll' is the number of lines above the bottom;
       scroll == 0 shows the newest lines (pinned to bottom). This gives exact
       1:1 wheel/key scrolling and clamps cleanly at the first line. */
    auto renderer = Renderer(input, [&] {
        contentHeight = int(transcript.size());
        if (viewH <= 0) viewH = 10;                 /* until first measured    */
        int maxScroll = std::max(0, contentHeight - viewH);
        if (scroll > maxScroll) scroll = maxScroll;
        if (scroll < 0) scroll = 0;

        int start = std::max(0, contentHeight - viewH - scroll);
        int end   = std::min(contentHeight, start + viewH);
        Elements lines;
        lines.reserve(size_t(end - start));
        for (int i = start; i < end; ++i)
            lines.push_back(text(transcript[size_t(i)]));

        Element pane = vbox(std::move(lines)) | flex | reflect(paneBox);
        viewH = paneBox.y_max - paneBox.y_min + 1;  /* for the next frame      */

        std::string title = " squirrel explore   " + pkgName.toStdString() +
                            "   (" + std::to_string(nSubjects) + " subjects) ";
        std::string hint = scroll > 0
            ? "  [scrolled up " + std::to_string(scroll) + "/" + std::to_string(maxScroll) +
                  " - wheel / Ctrl+Down / PageDown to return to bottom]"
            : "";

        return vbox({
                   text(title) | bold | color(Color::CyanLight),
                   separator(),
                   pane,
                   separator(),
                   hbox({ text("> ") | color(Color::GreenLight), input->Render() | flex }) | border,
                   text(hint) | dim,
               }) |
               flex;
    });

    /* Scroll handling wraps the whole component. Mouse wheel and Ctrl+Up/Down
       scroll a line (well, a few) at a time; PageUp/PageDown move by a visible
       page. Everything else falls through to the Input so typing still works. */
    auto root = CatchEvent(renderer, [&](Event e) {
        const int maxScroll = std::max(0, contentHeight - viewH);
        const int page = viewH > 1 ? viewH - 1 : 1;

        /* plain Up/Down: walk the command history (Ctrl+Up/Down scroll instead) */
        if (e == Event::ArrowUp) {
            if (histPos > 0) {
                if (histPos == int(cmdHistory.size()))
                    draft = command;            /* stash the in-progress line   */
                histPos--;
                command = cmdHistory[size_t(histPos)];
                cursorPos = int(command.size());
            }
            return true;
        }
        if (e == Event::ArrowDown) {
            if (histPos < int(cmdHistory.size())) {
                histPos++;
                command = (histPos == int(cmdHistory.size()))
                              ? draft
                              : cmdHistory[size_t(histPos)];
                cursorPos = int(command.size());
                return true;
            }
            return false;
        }

        if (e.is_mouse()) {
            if (e.mouse().button == Mouse::WheelUp)   { scroll = std::min(maxScroll, scroll + 3); return true; }
            if (e.mouse().button == Mouse::WheelDown) { scroll = std::max(0, scroll - 3);          return true; }
            return false;
        }
        if (e == Event::ArrowUpCtrl)   { scroll = std::min(maxScroll, scroll + 1);    return true; }
        if (e == Event::ArrowDownCtrl) { scroll = std::max(0, scroll - 1);            return true; }
        if (e == Event::PageUp)        { scroll = std::min(maxScroll, scroll + page); return true; }
        if (e == Event::PageDown)      { scroll = std::max(0, scroll - page);         return true; }
        return false;
    });

    screen.Loop(root);
    return 0;
}
