# Using libsquirrel in your own C++ program

`libsquirrel` is the library behind the `squirrel` command line utilities. Everything
the CLI can do — read, write, validate, merge, modify, extract, convert — is exposed
as ordinary C++ classes, so a third program gets identical behavior by linking the
library rather than shelling out to the `squirrel` binary.

The GUI in `src/squirrel-gui` is a worked example of everything on this page.

- [Requirements](#requirements)
- [Building the library](#building-the-library)
- [Linking it: qmake](#linking-it-qmake)
- [Linking it: CMake](#linking-it-cmake)
- [Linking it: plain g++](#linking-it-plain-g)
- [Hello, package](#hello-package)
- [The API in brief](#the-api-in-brief)
- [Running the utility operations](#running-the-utility-operations)
- [Threading rules](#threading-rules)
- [Logging](#logging)
- [Gotchas](#gotchas)

---

## Requirements

`libsquirrel` is a Qt library, not a plain C++ one. Your program needs:

| Requirement | Notes |
|---|---|
| **Qt 6** (Core + Sql) | `QtSql` is not optional — a squirrel package *is* a SQLite database, and the library keeps a `QSqlDatabase` connection open. Make sure Qt's SQLite driver is present. |
| A **`QCoreApplication`** | Must exist before you construct a `squirrel` object. `QApplication` counts. |
| **DCMTK** | DICOM reading. Expected in `/usr/local` on Linux, `C:\Program Files (x86)\DCMTK` on Windows. |
| **bit7z** | LZMA compression of the package archive. Built by each `build-<distro>.sh`. |
| **C++17** | The library is built with `CONFIG += c++17`. |

On Linux the library also has **dcm2niix compiled into it** (see `src/squirrel/dcm2niix.pri`),
so in-process DICOM→NIfTI conversion works without an external `dcm2niix` binary. On
Windows that is not the case and `ConvertDicom()` falls back to invoking `dcm2niix.exe`.

## Building the library

```bash
qmake -o $BUILDDIR/squirrel/Makefile src/squirrel/squirrellib.pro -spec linux-g++
cd $BUILDDIR/squirrel && make -j
```

This produces `libsquirrel.a` (Linux; `CONFIG += staticlib`) or `squirrel.lib` in
`bin/squirrel` (MSVC). The `build-<distro>.sh` scripts in the repo root do this for you,
along with building bit7z first, then the CLI and the GUI. (The GUI step is skipped with
a warning if Qt Widgets is unavailable, and can be suppressed with `SKIP_GUI=1`; it never
fails the build.)

**`libsquirrel` is a static archive on Linux.** That single fact drives most of what
follows: a static archive does not record its own dependencies, so *your* final link
line has to repeat every library squirrel was built against.

## Linking it: qmake

Easiest path — include the shared dependency file and let it supply bit7z, DCMTK and
zlib for you:

```qmake
QT += core sql

SQUIRRELSRC = /path/to/squirrel/src/squirrel
INCLUDEPATH += $$SQUIRRELSRC

# -lsquirrel must come BEFORE squirrel-deps.pri.
# GNU ld resolves archives in command line order: it takes one pass over
# libsquirrel.a, notes the symbols still undefined, and expects to find them in
# libraries listed AFTER it. Put -lsquirrel last and you get a wall of
# "undefined reference to dcmtk..." errors.
LIBS += -L/path/to/build/squirrel -lsquirrel

include($$SQUIRRELSRC/squirrel-deps.pri)
```

Two things to know about `squirrel-deps.pri`:

- **Its paths are relative to the build directory, not to the .pri file.** `LZMABIN = ../bit7z`
  assumes your project is built in a directory that is a *sibling* of the bit7z build
  directory — the layout `build-<distro>.sh` creates. If you build somewhere else, set
  the paths yourself instead of including the file.
- **Do not also include `dcm2niix.pri`.** Those objects are already inside `libsquirrel`;
  including it again compiles them a second time and you get duplicate symbols at link.

## Linking it: CMake

```cmake
cmake_minimum_required(VERSION 3.16)
project(myapp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)          # only if your own code uses Q_OBJECT

find_package(Qt6 REQUIRED COMPONENTS Core Sql)

set(SQUIRREL_SRC  /path/to/squirrel/src/squirrel)
set(SQUIRREL_LIB  /path/to/build/squirrel/libsquirrel.a)
set(BIT7Z_DIR     /path/to/build/bit7z)

add_executable(myapp main.cpp)
target_include_directories(myapp PRIVATE ${SQUIRREL_SRC} /usr/local/include)

# DCMTK's archives reference each other circularly, so a single pass is not
# enough; wrap them in a link group.
set(DCMTK_LIBS
    cmr dcmdata dcmdsig dcmect dcmfg dcmimage dcmimgle dcmiod dcmjpeg dcmjpls
    dcmnet dcmpmap dcmpstat dcmqrdb dcmrt dcmseg dcmsr dcmtkcharls dcmtls
    dcmtract dcmwlm dcmxml i2d ijg8 ijg12 ijg16 oficonv oflog ofstd)

target_link_directories(myapp PRIVATE /usr/local/lib /usr/local/lib64 ${BIT7Z_DIR})
target_link_libraries(myapp PRIVATE
    ${SQUIRREL_LIB}
    bit7z64
    -Wl,--start-group ${DCMTK_LIBS} -Wl,--end-group
    z dl
    Qt6::Core Qt6::Sql)
```

## Linking it: plain g++

For reference, this is what the qmake-generated link line reduces to:

```bash
g++ -std=c++17 -fPIC main.cpp -o myapp \
  -I/path/to/squirrel/src/squirrel -I/usr/local/include \
  -I$QTDIR/include -I$QTDIR/include/QtCore -I$QTDIR/include/QtSql \
  -L/path/to/build/squirrel -lsquirrel \
  -L/path/to/build/bit7z -lbit7z64 -ldl \
  -L/usr/local/lib -L/usr/local/lib64 \
  -Wl,--start-group \
    -lcmr -ldcmdata -ldcmdsig -ldcmect -ldcmfg -ldcmimage -ldcmimgle -ldcmiod \
    -ldcmjpeg -ldcmjpls -ldcmnet -ldcmpmap -ldcmpstat -ldcmqrdb -ldcmrt -ldcmseg \
    -ldcmsr -ldcmtkcharls -ldcmtls -ldcmtract -ldcmwlm -ldcmxml -li2d \
    -lijg8 -lijg12 -lijg16 -loficonv -loflog -lofstd \
  -Wl,--end-group \
  -lz \
  -L$QTDIR/lib -lQt6Sql -lQt6Core -lpthread \
  -Wl,-rpath,/usr/local/lib -Wl,-rpath,/usr/local/lib64 -Wl,-rpath,$QTDIR/lib
```

The `-rpath` entries matter: DCMTK is a set of shared objects under `/usr/local`, which
is not on the default loader path on most distros.

## Hello, package

Open an existing package and print its subjects:

```cpp
#include <QCoreApplication>
#include <iostream>
#include "squirrel.h"
#include "squirrelTypes.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);   // required before any squirrel object

    squirrel sqrl(false /*debug*/, true /*quickRead*/);
    sqrl.quiet = true;                  // don't write to stdout
    sqrl.SetPackagePath("/data/study.sqrl");
    sqrl.SetFileMode(FileMode::ExistingPackage);
    sqrl.SetQuickRead(true);            // skip the archive file listing
    sqrl.Read();

    if (!sqrl.IsValid()) {
        std::cerr << "not a valid squirrel package\n";
        return 1;
    }

    std::cout << "Package: " << sqrl.PackageName.toStdString() << "\n";

    for (squirrelSubject &subject : sqrl.GetSubjectList()) {
        std::cout << "  subject " << subject.ID.toStdString() << "\n";

        for (squirrelStudy &study : sqrl.GetStudyList(subject.GetObjectID())) {
            std::cout << "    study " << study.StudyNumber
                      << " (" << study.Modality.toStdString() << ")\n";

            for (squirrelSeries &series : sqrl.GetSeriesList(study.GetObjectID()))
                std::cout << "      series " << series.SeriesNumber << " "
                          << series.Description.toStdString() << "\n";
        }
    }
    return 0;
}
```

That opening sequence — `SetPackagePath` → `SetFileMode` → `SetQuickRead` → `Read` →
check `IsValid()` — is the one the CLI uses in `info.cpp`, and it is the one to copy.
Skipping `SetFileMode(ExistingPackage)` will not do what you want.

`SetQuickRead(true)` skips reading the file lists inside the archive. For anything that
only needs metadata — a listing, a tree view, counts — it is the difference between
opening in seconds and opening in minutes. Turn it off only when you need the per-object
file lists.

## The API in brief

Everything below is declared in `squirrel.h` unless noted.

**Package lifecycle**

| Call | Purpose |
|---|---|
| `squirrel(bool debug=false, bool quickRead=false)` | Constructor. Creates a uniquely-named database connection, so several packages can be open at once. |
| `SetPackagePath(QString)` / `SetFileMode(FileMode)` | `FileMode` is `NewPackage` or `ExistingPackage`. |
| `Read()` | Open an existing package. |
| `Write()` / `WriteUpdate()` | Write a new package / rewrite an opened one. |
| `Validate()` | Check package integrity. |
| `Extract(QString destDir, QString &m)` | Unpack the whole package. |
| `IsValid()`, `IsModified()`, `GetPackagePath()` | State. |

**Package metadata** is exposed as plain public members, not getters:
`PackageName`, `Description`, `DataFormat`, `PackageFormat`, `License`, `Readme`,
`Changes`, `Notes`, `Datetime`, `SquirrelVersion`, `SquirrelBuild`, and the
`SubjectDirFormat` / `StudyDirFormat` / `SeriesDirFormat` trio. Read or assign them
directly.

**Walking the hierarchy.** Objects are addressed by `rowID` (a `qint64`), obtained from
`GetObjectID()` on any object or from the `Find*` calls:

```cpp
QList<squirrelSubject>  GetSubjectList();
QList<squirrelStudy>    GetStudyList(qint64 subjectRowID);
QList<squirrelSeries>   GetSeriesList(qint64 studyRowID);
QList<squirrelAnalysis> GetAnalysisList(qint64 studyRowID);
QList<squirrelObservation>   GetObservationList(qint64 subjectRowID);
QList<squirrelIntervention>  GetInterventionList(qint64 subjectRowID);
QList<squirrelExperiment>    GetExperimentList();
QList<squirrelPipeline>      GetPipelineList();
QList<squirrelGroupAnalysis> GetGroupAnalysisList();
QList<squirrelDataDictionary> GetDataDictionaryList();

qint64 FindSubject(QString id);
qint64 FindStudy(QString subjectID, int studyNum);
qint64 FindSeries(QString subjectID, int studyNum, int seriesNum);
qint64 FindSeriesByUID(QString seriesUID);   // also FindStudyByUID()
qint64 FindPipeline(QString name, int version = -1);
```

A negative return from any `Find*` means "not found".

**Counts and sizes:** `GetObjectCount(ObjectType)`, `GetFileCount()`, `GetUnzipSize()`,
`GetFreeDiskSpace()`.

**Mutation:** `RemoveObject(ObjectType, qint64 rowID)`, `ExtractObject(...)`,
`AddStagedFiles(...)`, and the `ResequenceSubjects()` / `ResequenceStudies()` /
`ResequenceSeries()` family.

`ObjectType`, `FileMode`, `PrintFormat`, `DatasetType` and the `infoQuery` / `modification`
structs all live in `squirrelTypes.h`.

## Running the utility operations

The verbs behind the CLI subcommands are separate small classes. They take a package
*path*, open it themselves, and report back through a `QString &m` out-parameter — so you
do not need an open `squirrel` object to call them.

```cpp
#include "modify.h"

QString m;
modify mod;

// same call as: squirrel merge a.sqrl b.sqrl -o merged.sqrl
bool ok = mod.MergePackages({"a.sqrl", "b.sqrl"}, "merged.sqrl",
                            false /*testOnly*/, true /*renumberSubjects*/,
                            4 /*digits*/, m);
if (!ok)
    std::cerr << m.toStdString() << "\n";
```

Other operations go through a `modification` struct (`squirrelTypes.h`), where
`operation` is one of `add`, `remove`, `update`, `splitbymodality`, `removephi`,
`renumber`:

```cpp
modification mod_;
mod_.operation = "removephi";
mod_.object    = ObjectType::Subject;
mod_.objectID  = "S0001";

modify().DoModify("/data/study.sqrl", mod_, m);
```

`DoModify()` dispatches to `AddObject()`, `RemoveObject()`, `UpdateObject()`,
`SplitByModality()`, `RemovePHI()` or `RenumberSubjects()`; call those directly if you
prefer. Similarly `info::DisplayInfo(path, infoQuery, m)`, and the `extract`, `convert`
and `bids` classes.

Pass `testOnly = true` to `MergePackages()` to have it report what it would do without
writing anything — worth exposing in any UI you build.

## Threading rules

**A `squirrel` object belongs to the thread that created it, permanently.** It holds a
`QSqlDatabase` connection, and Qt forbids using a connection from any thread other than
the one that opened it. Violations do not always crash — they can silently return empty
result sets, which is worse.

If you have a UI, do not open packages on the UI thread; reading a multi-gigabyte archive
will freeze it for minutes. The pattern used by `src/squirrel-gui`:

1. A worker `QObject` (`squirrelWorker`) is `moveToThread()`-ed onto a `QThread`.
2. The `squirrel` instance is created, used and destroyed entirely inside that object.
3. The UI asks for work by emitting signals connected to the worker's slots (queued
   automatically across threads).
4. Results come back as **plain copied structs** (`squirrelModel.h`), never as pointers
   into library objects and never as a `squirrelSubject` still carrying a `rowID` the UI
   would later query.

Register any struct you send through a queued signal:

```cpp
qRegisterMetaType<guiPackage>("guiPackage");
```

Several packages *can* be open simultaneously — `databaseUUID` gives each its own
connection name — as long as each stays on its own thread.

## Logging

The library keeps an internal log rather than writing to stdout when you ask it to:

```cpp
sqrl.quiet = true;                 // suppress console output
...
QString newText = sqrl.GetLogBuffer();   // drains: returns only what's new
QString all     = sqrl.GetLog();         // everything so far
sqrl.SetDebug(true);                     // verbose; SetDebugSQL(true) for queries
```

`GetLogBuffer()` is the one to poll from a UI — it returns and clears, so repeated calls
give you a stream rather than an ever-growing blob. The `modify` / `info` / `extract`
classes report through their `QString &m` out-parameter instead.

## Gotchas

- **Construct `QCoreApplication` first.** The `squirrel` constructor opens its database
  immediately, and without a `QCoreApplication` in place Qt refuses:
  `qt.sql.qsqldatabase: QSqlDatabase requires a QCoreApplication`. The object is then
  unusable.
- **Link order.** `-lsquirrel` before the DCMTK/bit7z group. See above.
- **Don't include `dcm2niix.pri`** if you link the library — duplicate symbols.
- **`SetQuickRead(true)`** unless you actually need per-object file lists.
- **Check `IsValid()` after `Read()`.** `Read()` itself returns `bool`, but `IsValid()`
  is what the CLI tests, and a package can read without being usable.
- **`ObjectTypeToString()` / `ObjectTypeToEnum()`** are `static` on `squirrel` — useful
  for turning user input into an `ObjectType`.
- **Windows**: DICOM→NIfTI conversion shells out to `dcm2niix.exe`, which must be on
  `PATH`. On Linux it is in-process.
- The library is **not thread-safe within a single instance**. One thread per `squirrel`
  object.

---

For a complete, compiling consumer of everything described here, read
`src/squirrel-gui/squirrelWorker.cpp` (library calls, threading, snapshotting) and
`src/squirrel-gui/squirrel-gui.pro` (linking).
