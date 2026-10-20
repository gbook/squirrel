# Using squirrel from Python

The `squirrel` Python package lets you read, create, and edit squirrel packages from a
Python script, without running the command line tool. It wraps the same C++ library that
the `squirrel` utility uses, so the results are identical.

This page assumes **no Python experience**. If you already know Python, skip to
[Reading a package](#reading-a-package).

- [Setting up](#setting-up)
- [Running Python code](#running-python-code)
- [How a package is organized](#how-a-package-is-organized)
- [Reading a package](#reading-a-package)
- [Creating a package](#creating-a-package)
- [Changing an existing package](#changing-an-existing-package)
- [Extracting files](#extracting-files)
- [Errors](#errors)
- [Reference](#reference)
- [Things to know](#things-to-know)
- [Troubleshooting](#troubleshooting)

---

## Setting up

### 1. Check that Python is installed

```bash
python3 --version
```

You need **Python 3.9 or newer**. Most Linux systems already have it.

### 2. Make a virtual environment

A virtual environment is a private folder of Python libraries for one project. It keeps
`squirrel` from interfering with anything else on your computer, and it is easy to delete
and start over. You create it once per project:

```bash
python3 -m venv ~/squirrel-env
```

Every time you open a new terminal, **activate** it before using squirrel:

```bash
source ~/squirrel-env/bin/activate
```

Your prompt changes to start with `(squirrel-env)`. Type `deactivate` to leave it.

### 3. Install squirrel

Install the wheel file (a `.whl` file, which is Python's package format) you were given or
built. The file name will be something like `squirrel-2026.10.503-cp312-...-manylinux_2_28_x86_64.whl`:

```bash
pip install /path/to/squirrel-2026.10.503-cp312-manylinux_2_28_x86_64.whl
```

`pip` is Python's installer. It installs into whichever environment is active, so make
sure you activated the virtual environment first. To build wheels yourself, or to install
from the source code, see [`python/README.md`](../python/README.md).

### 4. Check that it works

```bash
python -c "import squirrel; print(squirrel.Squirrel().squirrel_version)"
```

This should print a version number such as `1.0`. If it prints an error instead, see
[Troubleshooting](#troubleshooting).

## Running Python code

There are two ways. Use whichever you prefer.

**A script.** Put code in a text file, for example `explore.py`, and run it:

```bash
python explore.py
```

**Interactively.** Type `python` to get a prompt (`>>>`) where you can try one line at a
time and see the result immediately. Type `exit()` to leave. This is the best way to
experiment:

```
$ python
>>> import squirrel
>>> pkg = squirrel.load("study.sqrl")
>>> pkg.name
'Demo'
```

Every example below can be typed either way. A few Python basics you will see:

- `import squirrel` makes the library available. Put it at the top of every script.
- `x = 5` stores a value under the name `x`.
- `for item in things:` repeats the indented lines below it once per item.
- `#` starts a comment.
- Indentation (four spaces) matters. It is how Python knows which lines belong to a `for`.

## How a package is organized

A squirrel package holds **subjects**. Each subject has **studies** (visits or imaging
sessions), and each study has **series** (individual scans or recordings, with their
files). Subjects can also have **observations** and **interventions**. A package may also
hold experiments, pipelines, analyses, group analyses and data dictionaries.

```
package
 └── subject S001
      ├── study 1
      │    ├── series 1   (files)
      │    └── series 2   (files)
      ├── observation   (e.g. a test score)
      └── intervention  (e.g. a drug)
```

Every object has a **`row_id`**, a number the package assigns when the object is stored.
It is how you ask for the children of an object:
`pkg.get_study_list(subject.row_id)` means "the studies belonging to this subject". It is
**not** the subject's ID string (`subject.id`, such as `"S001"`) and it is not the study
or series number. Row IDs are only meaningful within one package.

## Reading a package

### Load it

```python
import squirrel

pkg = squirrel.load("study.sqrl")
```

`load` accepts a string or a `pathlib.Path`. If the file is missing or damaged it raises
`SquirrelError` (see [Errors](#errors)). Pass `quick=True` to skip reading the per-series
parameter files, which is faster for large packages when you do not need them.

### Package information

```python
print(pkg.name)           # 'Demo'
print(pkg.description)    # 'demo pkg'
print(pkg.datetime)       # 2026-10-20 11:59:27   (a Python datetime)
print(pkg.data_format)    # 'orig'

print(pkg.print())        # a text summary of the package, like `squirrel info`
print(pkg.print_tree())   # the same, as a tree of every subject/study/series
```

### Walk through the contents

The `get_..._list` functions return ordinary Python lists, so you can loop over them:

```python
for subject in pkg.get_subject_list():
    print(subject.id, subject.sex, subject.date_of_birth)

    for study in pkg.get_study_list(subject.row_id):
        print("  study", study.study_number, study.modality, study.datetime)

        for series in pkg.get_series_list(study.row_id):
            print("    series", series.series_number, series.description)
            for f in series.files:
                print("      ", f)
```

Output for a small package:

```
S001 F 1990-05-17
  study 1 MR 2026-10-20 11:59:27
    series 1 T1
       data/S001/1/1/t1.nii
       data/S001/1/1/params.json
```

Subjects also have observations and interventions:

```python
for obs in pkg.get_observation_list(subject.row_id):
    print(obs.name, obs.value, obs.date_start)

for drug in pkg.get_intervention_list(subject.row_id):
    print(drug.name, drug.dose_amount, drug.dose_unit)
```

And the package-level lists need no argument: `get_experiment_list()`,
`get_pipeline_list()`, `get_group_analysis_list()`, `get_data_dictionary_list()`.

### Find one thing

`find_...` returns a `row_id`, or `-1` if there is no match. Pass that to `get_...`:

```python
row = pkg.find_subject("S001")
if row < 0:
    print("no such subject")
else:
    subject = pkg.get_subject(row)

series_row = pkg.find_series("S001", 1, 1)   # subject ID, study number, series number
series = pkg.get_series(series_row)
print(series.params)                          # a dictionary, e.g. {'TR': '2000'}
```

### Counts

```python
print(pkg.get_object_count(squirrel.ObjectType.SUBJECT))   # number of subjects
print(pkg.get_object_count(squirrel.ObjectType.SERIES))
print(pkg.count_studies(subject.row_id))                    # studies for one subject
```

### Dates

Dates come back as standard Python `datetime.date` and `datetime.datetime` objects, and you
can pass the same kinds of values in. A missing date is `None`.

```python
import datetime
print(subject.date_of_birth.year)
study.datetime = datetime.datetime(2025, 3, 1, 9, 30)
```

## Creating a package

```python
import datetime
import squirrel

pkg = squirrel.new("study.sqrl", name="My study", description="Pilot data")

subject = pkg.add_subject("S001", sex="F", date_of_birth=datetime.date(1990, 5, 17))

study = pkg.add_study(subject, modality="MR", description="Baseline scan")

pkg.add_series(study, description="T1", files=["/data/S001/t1.nii.gz"],
               params={"TR": "2000", "TE": "3.2"})

pkg.add_observation(subject, "MMSE", value="29", date_start=datetime.datetime(2025, 1, 2))
pkg.add_intervention(subject, "Aspirin", dose_amount=81, dose_unit="mg",
                     date_start=datetime.datetime(2025, 1, 2))

pkg.write()
```

Nothing is saved to disk until `pkg.write()`. If `study.sqrl` already exists, `write()`
refuses to replace it and raises `SquirrelError`; pass `overwrite=True` to `squirrel.new` to
replace it.

How the `add_` functions work:

- The first argument identifies the parent (a subject for a study, a study for a series)
  and the rest are fields, written exactly as in the [reference tables](#reference).
  A misspelled field name raises an error rather than being ignored.
- `add_study` and `add_series` number themselves (study 1, 2, 3...). Pass
  `study_number=` or `series_number=` to choose.
- They **return the stored object**. Keep it if you want to add children to it, as `study`
  is used above.
- Adding something that already exists, such as a second subject `"S001"`, raises
  `SquirrelError`.
- You can pass a parent object or just its `row_id`: `pkg.add_study(subject.row_id)`.
- `files=[...]` lists files on your disk. They are copied into the package when you call
  `write()`, so they must still exist then. Each series also gets a `params.json`.

The other `add_` functions are `add_analysis(study, name)`, `add_experiment(name)`,
`add_pipeline(name, version=1)`, `add_group_analysis(name)` and `add_data_dictionary(name)`.

### Data format: `orig` vs `nifti`

`squirrel.new` stores files exactly as given (`data_format="orig"`). The underlying library's
own default, `"nifti4dgz"`, instead converts **DICOM** files to NIfTI while writing and fails
on anything that is not DICOM. Use it when your series folders contain DICOM:

```python
pkg = squirrel.new("study.sqrl", data_format="nifti4dgz")
pkg.add_series(study, files=sorted(glob.glob("/data/S001/dicom/*.dcm")))
```

The other values are `anon`, `anonfull`, `nifti3d`, `nifti3dgz` and `nifti4d`.

## Changing an existing package

Objects you get back from `get_...` are **copies**. Changing a field does not change the
package until you call `store()`, and the change does not reach the file on disk until you
call `write_update()`:

```python
pkg = squirrel.load("study.sqrl")

subject = pkg.get_subject(pkg.find_subject("S001"))
subject.notes = "Reviewed 2026-10-20"
subject.store()            # save the change into the package in memory

pkg.write_update()         # write the changes to study.sqrl
```

Adding works the same way. Load, add, then `write_update()`:

```python
pkg = squirrel.load("study.sqrl")
pkg.add_subject("S002", sex="M")
pkg.write_update()
```

To remove something, give its type and row ID:

```python
row = pkg.find_subject("S002")
pkg.remove_object(squirrel.ObjectType.SUBJECT, row)
pkg.write_update()
```

Removing a subject also removes its studies and series.

## Extracting files

```python
pkg = squirrel.load("study.sqrl")

# Everything in the package
pkg.extract("/tmp/unpacked")

# One subject (with all its studies and series), or just one study or series
subject_row = pkg.find_subject("S001")
pkg.extract_object(squirrel.ObjectType.SUBJECT, subject_row, "/tmp/subject-S001")
```

`extract_object` writes into a folder that must already exist. Use `ObjectType.STUDY` or
`ObjectType.SERIES` with their row IDs for smaller pieces.

## Errors

When something goes wrong, squirrel raises `squirrel.SquirrelError`. Catch it with
`try` / `except`:

```python
import squirrel

try:
    pkg = squirrel.load("missing.sqrl")
except squirrel.SquirrelError as e:
    print("Could not open the package:", e)
```

The message is the library's log, so it is long; the reason is usually in the last line
(for example `File missing.sqrl does not exist`). If you do not catch the error, the script
stops and Python prints it.

`SquirrelError` is a kind of `RuntimeError`. Typing a field name that does not exist
(`subject.sexx = "F"`) raises `AttributeError` instead.

To check a package without raising, `pkg.validate()` returns `True` or `False`, and
`pkg.get_log()` returns everything the library has logged.

## Reference

### Package: `squirrel.Squirrel`

Create with `squirrel.new(path, ...)` or `squirrel.load(path)`.

| Fields (read/write) | |
|---|---|
| `name`, `description`, `license`, `readme`, `notes`, `changes` | Header text |
| `datetime` | When the package was created |
| `data_format` | See [Data format](#data-format-orig-vs-nifti) |
| `subject_dir_format`, `study_dir_format`, `series_dir_format` | `orig` or `seq` |
| `package_path` | Where the package is read from or written to |

| Method | What it does |
|---|---|
| `write()` | Write a new package to disk |
| `write_update()` | Write changes to an existing package |
| `validate()` | `True` if the package is valid |
| `print()`, `print_tree()` | Text summaries |
| `get_subject_list()` | All subjects |
| `get_study_list(subject_row_id)` | A subject's studies |
| `get_series_list(study_row_id)` | A study's series |
| `get_analysis_list(study_row_id)` | A study's analyses |
| `get_observation_list(subject_row_id)`, `get_intervention_list(subject_row_id)` | A subject's observations / interventions |
| `get_experiment_list()`, `get_pipeline_list()`, `get_group_analysis_list()`, `get_data_dictionary_list()` | Package-level lists |
| `get_subject(row_id)`, `get_study(row_id)`, `get_series(row_id)`, ... | One object, by row ID (also `analysis`, `observation`, `intervention`, `experiment`, `pipeline`, `group_analysis`, `data_dictionary`) |
| `find_subject(id)` | Row ID, or `-1` |
| `find_study(subject_id, study_number)` | |
| `find_series(subject_id, study_number, series_number)` | |
| `find_analysis(subject_id, study_number, name)` | |
| `find_experiment(name)`, `find_group_analysis(name)`, `find_data_dictionary(name)` | |
| `find_pipeline(name, version=-1)` | |
| `find_study_by_uid(uid)`, `find_series_by_uid(uid)` | |
| `find_observation(subject_id, name, date_start)`, `find_intervention(...)` | |
| `get_object_count(type)` | Number of objects of an `ObjectType` |
| `count_studies(subject_row_id)`, `count_series(study_row_id)`, `count_observations(...)`, `count_interventions(...)` | |
| `add_subject`, `add_study`, `add_series`, `add_observation`, `add_intervention`, `add_analysis`, `add_experiment`, `add_pipeline`, `add_group_analysis`, `add_data_dictionary` | Create and store an object |
| `remove_object(type, row_id)` | Delete an object and its children |
| `extract(dir)`, `extract_object(type, row_id, dir)` | See [Extracting](#extracting-files) |
| `get_log()`, `get_progress()` | Log text; progress of a long operation, 0–100 |

`squirrel.ObjectType` has `SUBJECT`, `STUDY`, `SERIES`, `ANALYSIS`, `OBSERVATION`,
`INTERVENTION`, `EXPERIMENT`, `PIPELINE`, `GROUP_ANALYSIS` and `DATA_DICTIONARY` (and a few
internal ones).

### Objects

Every object also has `row_id` (read-only), `store()`, and `is_valid()`.

| Class | Fields |
|---|---|
| **Subject** | `id`, `guid`, `alternate_ids` (a list), `date_of_birth`, `sex`, `gender`, `ethnicity1`, `ethnicity2`, `enrollment_group`, `enrollment_status`, `notes`, `sequence_number` |
| **Study** | `study_number`, `datetime`, `modality`, `description`, `visit_type`, `equipment`, `study_uid`, `age_at_study`, `height`, `weight`, `day_number`, `time_point`, `notes`, `sequence_number`, `subject_row_id` |
| **Series** | `series_number`, `datetime`, `description`, `protocol`, `series_uid`, `params` (a dictionary), `run`, `size`, `file_count`, `files` (read-only), `staged_files`, `bids_entity`, `bids_suffix`, `bids_task`, `bids_run`, `bids_phase_encoding_direction`, `behavioral_size`, `behavioral_file_count`, `beh_files` (read-only), `staged_beh_files`, `study_row_id`, `subject_row_id`, `experiment_row_id`, `sequence_number` |
| **Observation** | `name`, `value`, `type`, `date_start`, `date_end`, `duration`, `instrument_name`, `rater`, `description`, `notes`, `date_record_create`, `date_record_entry`, `date_record_modify`, `subject_row_id` |
| **Intervention** | `name`, `dose_amount`, `dose_unit`, `dose_frequency`, `dose_key`, `dose_string`, `administration_route`, `intervention_class`, `date_start`, `date_end`, `rater`, `description`, `notes`, `date_record_create`, `date_record_entry`, `date_record_modify`, `subject_row_id` |
| **Analysis** | `analysis_name`, `pipeline_name`, `pipeline_version`, `status`, `status_message`, `successful`, `hostname`, `series_count`, `size`, `run_time`, `setup_time`, `date_start`, `date_end`, `date_cluster_start`, `date_cluster_end`, `staged_files`, `study_row_id`, `pipeline_row_id` |
| **Experiment** | `name`, `size`, `file_count`, `staged_files` |
| **Pipeline** | `name`, `version`, `description`, `notes`, `create_date`, `directory`, `directory_structure`, `primary_script`, `secondary_script`, `result_script`, `complete_files`, `analysis_level`, `search_*`, `setup_*`, `cluster_*`, `staged_files` |
| **GroupAnalysis** | `name`, `datetime`, `description`, `notes`, `size`, `file_count`, `staged_files` |
| **DataDictionary** | `name`, `size`, `file_count`, `staged_files` |

`staged_files` is a list of file paths on your disk that will be copied into the package
when it is written. `files` is the list of files already inside a package.

## Things to know

- **Use one thread.** A `Squirrel` object holds a database connection that belongs to the
  thread that created it. Do not share it between Python threads.
- **Keep the package alive.** Objects from `get_...` are tied to their package. If you let the
  package go (for example by reassigning `pkg`) and then call `store()` on an old object, it
  fails with an error about a missing database connection.
- **Progress messages appear in the terminal.** The library prints progress while reading and
  writing, and Python cannot silence it. To hide it, redirect when running:
  `python script.py > /dev/null 2>&1` (this also hides your own `print` output).
- **`write_update()` prints SQL errors that are safe to ignore.** When a subject has
  observations or interventions, it may print `UNIQUE constraint failed: Observation...`
  lines. The library is re-saving rows that already exist, and the data is unchanged.
- **`write()` fills in counts for you.** It recalculates `size` and `file_count`, and adds a
  `params.json` to every series, so a series with one data file reports two files.
- **Not available yet from Python:** a pipeline's data steps and a data dictionary's items.
  Use the `squirrel modify` command for those.

## Troubleshooting

**`ModuleNotFoundError: No module named 'squirrel'`**
Python cannot see the library. Activate your virtual environment
(`source ~/squirrel-env/bin/activate`) and check with `pip list` that `squirrel` is
installed. The `python` you run must be the one in that environment.

**`ImportError: ... libQt6Core.so.6: cannot open shared object file`**
You are using a wheel that was built on a developer machine rather than a portable one, and
it cannot find Qt. Use a wheel built with `python/packaging/build-wheels.sh`, or ask whoever
built it.

**`SquirrelError: ... database connection ... does not exist`**
You are storing an object whose package has been discarded. See *Keep the package alive*
above.

**`SquirrelError: Subject [S001] already exists`**
A subject with that ID is already in the package. Use `pkg.get_subject(pkg.find_subject("S001"))`
to get it instead.

**`SquirrelError` when writing with `Unable to find any DICOM images`**
The package's `data_format` is `nifti4dgz` (DICOM conversion) but the files are not DICOM.
Create the package with `squirrel.new(..., data_format="orig")`, which is the default.

**`pip install` says the wheel "is not a supported wheel on this platform"**
The wheel is for a different Python version or operating system. The `cp312` in the file
name means Python 3.12; check yours with `python --version`.
