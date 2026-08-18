# Squirrel File Flow

There are **three distinct path concepts** and they interact through the `Write()` process.

---

## The Three Path Types

```
DISK PATH                VIRTUAL PATH               ARCHIVE PATH
(where the file lives    (logical location inside   (actual path in .sqrl zip)
 on your filesystem)      the package)

/data/scans/scan1.dcm → data/ABC001/1/2/scan1.dcm → data/ABC001/1/2/scan1.dcm
                         ↑ computed by VirtualPath()   ↑ same as virtual path
```

---

## Where Each Type Lives in the Code

| Concept | Storage | How set | How read |
|---|---|---|---|
| **Disk path** | `StagedFiles` DB table → `object.stagedFiles` (QStringList) | `object.stagedFiles.append(path)` + `object.Store()` | `utils::GetStagedFileList()` |
| **Virtual path** | Each object's DB row (`VirtualPath` column) | Computed by `VirtualPath()`, stored at `Store()` | `object.VirtualPath()` |
| **Archive path** | Inside `.sqrl` file | Written by `Write()` | `bit7z` reader |

---

## Virtual Path Patterns (by object type)

```
Series:         data/{subjectDir}/{studyDir}/{seriesDir}/
                     e.g. data/ABC001/1/2/

Analysis:       data/{subjectDir}/{studyDir}/{PipelineName}/
                     e.g. data/ABC001/1/my_pipeline/

Pipeline:       pipelines/{PipelineName}/
Experiment:     experiments/{ExperimentName}/
GroupAnalysis:  group-analysis/{GroupAnalysisName}/
DataDictionary: data-dictionary/{DataDictionaryName}/
```

`{subjectDir}` is either the subject ID string (`orig` mode) or a sequence number.  
`{studyDir}` is either the study number or a sequence number.  
`{seriesDir}` is either the series number or a sequence number.

---

## Staged File Flow (step by step with a concrete example)

```
STEP 1 — Files exist on disk
─────────────────────────────────────────────────────
  /tmp/scan_session/0001.dcm
  /tmp/scan_session/0002.dcm
  /tmp/scan_session/0003.dcm

STEP 2 — Series object is created and files are staged
─────────────────────────────────────────────────────
  squirrelSeries series(dbUUID);
  series.SeriesNumber = 2;
  series.studyRowID   = 7;          // links to Study 1 of ABC001
  series.stagedFiles  = ["/tmp/scan_session/0001.dcm",
                         "/tmp/scan_session/0002.dcm",
                         "/tmp/scan_session/0003.dcm"];
  series.Store();
    ↓
  StagedFiles table:
  ┌──────────────┬─────────────┬──────────────────────────────────┐
  │ ObjectType   │ ObjectRowID │ StagedPath                       │
  ├──────────────┼─────────────┼──────────────────────────────────┤
  │ "series"     │ 12          │ /tmp/scan_session/0001.dcm       │
  │ "series"     │ 12          │ /tmp/scan_session/0002.dcm       │
  │ "series"     │ 12          │ /tmp/scan_session/0003.dcm       │
  └──────────────┴─────────────┴──────────────────────────────────┘

STEP 3 — VirtualPath() is computed (at Write time)
─────────────────────────────────────────────────────
  series.VirtualPath()
    → "data/ABC001/1/2"    (subjectID=ABC001, studyNum=1, seriesNum=2)

STEP 4 — Write() copies to working directory
─────────────────────────────────────────────────────
  /tmp/squirrel-xK7pQ2m8wR/         ← workingDir (temp)
    data/
      ABC001/
        1/
          2/
            0001.dcm                ← copied from disk path
            0002.dcm
            0003.dcm
            params.json             ← written from series.params

STEP 5 — Working dir is compressed into .sqrl archive
─────────────────────────────────────────────────────
  CompressDirectoryToArchive(workingDir, "mypackage.sqrl")

  mypackage.sqrl                    ← final archive
    squirrel.json
    data/
      ABC001/
        1/
          2/
            0001.dcm
            0002.dcm
            0003.dcm
            params.json
```

---

## Non-series objects (experiments, pipelines, etc.)

For these objects, `Write()` does not handle them in the Step 1 series loop. Instead, staged
file pairs are collected during the JSON-building phase:

```
STEP 2b — stagedFiles collected during JSON-building phase
─────────────────────────────────────────────────────
  for (auto &e : experiments) {
      stagedFiles += e.GetStagedFileList();   // returns {diskPath, virtualPath} pairs
  }

  GetStagedFileList() returns:
  [  ("/home/user/my_task_scripts/run.sh",   "experiments/MyTask"),
     ("/home/user/my_task_scripts/stim.zip", "experiments/MyTask")  ]
         ↑ first = disk path                  ↑ second = virtual dir

STEP 4b — Each pair is copied to workingDir/{virtualPath}/{filename}
─────────────────────────────────────────────────────
  /tmp/squirrel-xK7pQ2m8wR/
    experiments/
      MyTask/
        run.sh
        stim.zip
```

---

## New vs Existing Package (the key difference)

```
NEW PACKAGE (FileMode::NewPackage)
  1. Write series files into workingDir temp tree
  2. Copy other staged files into workingDir temp tree
  3. Write squirrel.json into workingDir
  4. CompressDirectoryToArchive(workingDir → .sqrl)
  5. Delete workingDir

EXISTING PACKAGE (FileMode::ExistingPackage)
  1. Skip series file copy (data already in archive)
  2. AddFilesToArchive(diskPaths, archivePaths, .sqrl)   ← bit7z in-place update
  3. UpdateMemoryFileToArchive(json, "squirrel.json", .sqrl)
  ⚠ No temp dir involved; files go directly into the existing archive
```

---

## Implications for Add/Remove when Modifying Packages

**To add files to an existing package:**
```cpp
sqrl->Read();                          // load from .sqrl into in-memory DB
series.stagedFiles.append("/new/file.dcm");
series.Store();                        // writes path to StagedFiles table
sqrl->Write();                         // or WriteUpdate() for metadata-only
```

**To remove files from an existing package:**  
There is no direct "delete file from archive" path today. `RemoveStagedFileList()` removes
the DB record, but any file already baked into the archive is untouched. A full `Write()` in
`NewPackage` mode effectively rebuilds from scratch. For existing packages, removal would
require bit7z archive editing (not currently wired up).

**`WriteUpdate()` vs `Write()`:**  
- `WriteUpdate()` — updates `squirrel.json` in the archive only; no file copying
- `Write()` on `ExistingPackage` — adds/updates files AND updates `squirrel.json`
