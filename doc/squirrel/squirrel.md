# squirrel(1) — Squirrel Data Package Utilities

## Synopsis

```
squirrel <command> [options] [arguments]
```

## Description

`squirrel` is a command-line tool for creating, inspecting, and modifying **squirrel data packages** (`.sqrl` files). A squirrel package is a self-contained archive that stores neuroimaging and associated data — subjects, studies, imaging series, analyses, pipelines, observations, interventions, experiments, and data dictionaries — together with a JSON manifest.

The following commands are available:

| Command | Description |
|---|---|
| `dicom2squirrel` | Convert a DICOM directory into a squirrel package |
| `bids2squirrel` | Convert a BIDS directory into a squirrel package |
| `info` | Display information about a package or its contents |
| `modify` | Add, remove, or update objects within a package |
| `extract` | Extract objects from a package to disk |
| `validate` | Check whether a package is valid and readable |

---

## Commands

### dicom2squirrel

Convert a directory of DICOM files into a squirrel package.

```
squirrel dicom2squirrel <dicomdirectory> <package> [options]
```

**Arguments**

| Argument | Description |
|---|---|
| `dicomdirectory` | Path to the input directory containing DICOM files |
| `package` | Path to the output squirrel package (`.sqrl` extension added if omitted) |

**Options**

| Option | Description |
|---|---|
| `-d`, `--debug` | Enable debug logging |
| `-q`, `--quiet` | Suppress header and progress output |
| `--dataformat <format>` | Output data format (see below). Default: `nifti4dgz` |
| `--dirformat <format>` | Directory naming within the package. `orig` uses original subject IDs; `seq` uses sequential numbers. Default: `orig` |

**Data formats** (`--dataformat`)

| Value | Description |
|---|---|
| `anon` | Anonymized DICOM |
| `nifti4d` | NIfTI 4D |
| `nifti4dgz` | NIfTI 4D gzip-compressed *(default)* |
| `nifti3d` | NIfTI 3D |
| `nifti3dgz` | NIfTI 3D gzip-compressed |
| `orig` | Original DICOM files, unmodified |

**Examples**

```
squirrel dicom2squirrel /data/dicoms /output/study.sqrl
squirrel dicom2squirrel /data/dicoms /output/study.sqrl --dataformat nifti4d --dirformat orig
squirrel dicom2squirrel /data/dicoms /output/study.sqrl --dataformat anon -q
```

---

### bids2squirrel

Convert a BIDS-formatted directory into a squirrel package.

```
squirrel bids2squirrel <bidsdirectory> <package> [options]
```

**Arguments**

| Argument | Description |
|---|---|
| `bidsdirectory` | Path to the root BIDS directory |
| `package` | Path to the output squirrel package |

**Options**

| Option | Description |
|---|---|
| `-d`, `--debug` | Enable debug logging |
| `-q`, `--quiet` | Suppress header and progress output |
| `--overwrite` | Overwrite an existing package at the output path |
| `--debugsql` | Log all SQL statements (for troubleshooting) |

**Examples**

```
squirrel bids2squirrel /data/bids_study /output/study.sqrl
squirrel bids2squirrel /data/bids_study /output/study.sqrl --overwrite
```

---

### info

Display information about a package or its contents.

```
squirrel info <package> [options]
```

**Arguments**

| Argument | Description |
|---|---|
| `package` | Path to the squirrel package |

**Options**

| Option | Description |
|---|---|
| `-d`, `--debug` | Enable debug logging |
| `--object <type>` | Object type to display (see below). Default: `package` |
| `--subjectid <id>` | Filter by subject ID |
| `--studynum <num>` | Filter by study number (requires `--subjectid`) |
| `--dataset <type>` | Amount of detail: `id`, `basic`, or `full`. Default: `full` |
| `--format <format>` | Output format: `list` or `csv`. Default: `csv` |

**Object types** (`--object`)

`all`  `package`  `subject`  `study`  `series`  `observation`  `intervention`  `experiment`  `pipeline`  `groupanalysis`  `datadictionary`

**Examples**

```
squirrel info study.sqrl
squirrel info study.sqrl --object subject --dataset full --format csv
squirrel info study.sqrl --object study --subjectid S1234
squirrel info study.sqrl --object series --subjectid S1234 --studynum 1
```

---

### modify

Add, remove, update, or otherwise transform objects within an existing package.

```
squirrel modify <package> --operation <op> --object <type> [options]
```

**Arguments**

| Argument | Description |
|---|---|
| `package` | Path to the squirrel package |

**Options**

| Option | Description |
|---|---|
| `-d`, `--debug` | Enable debug logging |
| `-q`, `--quiet` | Suppress header and progress output |
| `--operation <op>` | Operation to perform (see below) |
| `--object <type>` | Object type to operate on (see below) |
| `--datapath <path>` | Path to data for the new object. Wildcards supported (e.g. `/path/*.dcm`) |
| `--objectid <id>` | ID, name, or number of the existing object to modify or remove |
| `--subjectid <id>` | Parent subject ID. Required when adding or removing a study, series, observation, intervention, or analysis |
| `--studynum <num>` | Parent study number. Required when adding or removing a series or analysis (`--subjectid` also required) |
| `--seriesnum <num>` | Parent series number. Required when updating a series (`--subjectid` and `--studynum` also required) |
| `--objectdata <string>` | URL-encoded key=value string specifying object metadata (e.g. `SubjectID=S1234&Sex=M`) |
| `--variablelist <type>` | Print all settable variable names for the given object type and exit |
| `--digits <num>` | Number of digits in the renumbered ID (e.g. `4` → `0001`…`9999`). Default: auto-sized to fit the largest generated number. Used with `renumber` |
| `--startnum <num>` | Starting number for renumbering. Default: `1`. Used with `renumber` |
| `--prefix <string>` | Prefix prepended to renumbered IDs (e.g. `sub` → `sub0001`, `sub0002`). Used with `renumber` |
| `--random` | Randomly assign new IDs instead of sorting subjects ascending before renumbering. Used with `renumber` |

**Operations** (`--operation`)

| Value | Description |
|---|---|
| `add` | Add a new object to the package |
| `remove` | Remove an existing object (and its children) from the package |
| `update` | Update fields of an existing object |
| `splitbymodality` | Split the package into separate packages, one per imaging modality |
| `removephi` | Strip Protected Health Information (dates, IDs) from the package |
| `renumber` | Reassign subject IDs sequentially (1–N); original ID is moved to AlternateIDs |

**Object types** (`--object`)

`package`  `subject`  `study`  `series`  `analysis`  `intervention`  `observation`  `experiment`  `pipeline`  `groupanalysis`  `datadictionary`

**Object data format** (`--objectdata`)

Metadata is passed as a URL-encoded string of `key=value` pairs joined by `&`. The available keys depend on the object type; use `--variablelist <type>` to see them.

```
SubjectID=S1234&DateOfBirth=1999-12-31&Sex=M&Gender=M
StudyNumber=1&Modality=MR&Description=Baseline
```

**Examples**

```
# Update subject demographics
squirrel modify study.sqrl --operation update --object subject \
    --objectid S1234 --objectdata 'DateOfBirth=1999-12-31&Sex=M'

# Add a new subject
squirrel modify study.sqrl --operation add --object subject \
    --objectdata 'SubjectID=S9999&Sex=F&DateOfBirth=1985-06-15'

# Remove a series
squirrel modify study.sqrl --operation remove --object series \
    --subjectid S1234 --studynum 1 --objectid 3

# Split package by imaging modality
squirrel modify study.sqrl --operation splitbymodality

# Remove PHI
squirrel modify study.sqrl --operation removephi

# Renumber subjects 1–N (sorted ascending, auto-sized IDs)
squirrel modify study.sqrl --operation renumber

# Renumber with 4-digit IDs starting at 1001 and a prefix
squirrel modify study.sqrl --operation renumber --digits 4 --startnum 1001 --prefix sub

# Renumber with random assignment
squirrel modify study.sqrl --operation renumber --random

# List settable variables for the study object
squirrel modify study.sqrl --variablelist study
```

---

### extract

Extract one or more objects from a package to a directory on disk.

```
squirrel extract <package> --object <type> --objectid <id> [options]
```

**Arguments**

| Argument | Description |
|---|---|
| `package` | Path to the squirrel package |

**Options**

| Option | Description |
|---|---|
| `-d`, `--debug` | Enable debug logging |
| `-q`, `--quiet` | Suppress header and progress output |
| `--object <type>` | Object type to extract (see below) |
| `--outdir <path>` | Destination directory for extracted files |
| `--objectid <id>` | ID, name, or number of the object to extract |
| `--subjectid <id>` | Parent subject ID. Required when extracting a study, series, or analysis |
| `--studynum <num>` | Parent study number. Required when extracting a series or analysis (`--subjectid` also required) |

**Object types** (`--object`)

`subject`  `study`  `series`  `analysis`  `experiment`  `pipeline`  `groupanalysis`  `datadictionary`

**Examples**

```
# Extract a subject
squirrel extract study.sqrl --object subject --objectid S1234ABC --outdir /tmp/out

# Extract a specific series
squirrel extract study.sqrl --object series --objectid 1 \
    --subjectid S1234ABC --studynum 1 --outdir /tmp/out

# Extract an experiment
squirrel extract study.sqrl --object experiment --objectid MyExperiment --outdir /tmp/out
```

---

### validate

Check whether a squirrel package is readable and structurally valid.

```
squirrel validate <package>
```

**Arguments**

| Argument | Description |
|---|---|
| `package` | Path to the squirrel package to validate |

**Examples**

```
squirrel validate study.sqrl
```

---

## Global Options

These options are available for all commands.

| Option | Description |
|---|---|
| `-h`, `--help` | Show help text and exit |
| `--version` | Show version information and exit |

---

## Object Hierarchy

A squirrel package organizes data in the following hierarchy:

```
package
├── subject
│   ├── study
│   │   ├── series
│   │   └── analysis
│   ├── observation
│   └── intervention
├── experiment
├── pipeline
├── group-analysis
└── data-dictionary
```

---

## Author

Gregory A Book &lt;gregory.book@hhchealth.org&gt;  
Olin Neuropsychiatry Research Center, Hartford Hospital
