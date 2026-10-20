"""Python interface to the squirrel data package library."""

import os
from pathlib import Path
from typing import Iterable, Optional, Union

# Wheels ship Qt's SQLite driver plugin next to this file. Qt reads QT_PLUGIN_PATH
# when the extension module creates its QCoreApplication, so set it before import.
_plugins = Path(__file__).parent / "qt_plugins"
if _plugins.is_dir():
    os.environ["QT_PLUGIN_PATH"] = os.pathsep.join(
        [str(_plugins)] + [p for p in os.environ.get("QT_PLUGIN_PATH", "").split(os.pathsep) if p])

# Wheels also ship a 7-Zip library, matching the ABI libsquirrel was built against.
# SQUIRREL_7ZIP_LIB (read when a Squirrel object is created) takes precedence over the
# system 7-Zip; a value the user has already set is left alone.
_lib7z = Path(__file__).parent / "lib7z" / "7z.so"
if _lib7z.is_file():
    os.environ.setdefault("SQUIRREL_7ZIP_LIB", str(_lib7z))

from . import _squirrel
from ._squirrel import (
    Analysis,
    DataDictionary,
    Experiment,
    GroupAnalysis,
    Intervention,
    Observation,
    ObjectType,
    Pipeline,
    Series,
    SquirrelError,
    Study,
    Subject,
)

__all__ = [
    "Analysis", "DataDictionary", "Experiment", "GroupAnalysis", "Intervention",
    "ObjectType", "Observation", "Pipeline", "Series", "Squirrel", "SquirrelError",
    "Study", "Subject", "load", "new",
]

PathLike = Union[str, Path]


def _row_id(obj_or_id) -> int:
    """Accept either an object (anything with a row_id) or a raw row ID."""
    return obj_or_id if isinstance(obj_or_id, int) else obj_or_id.row_id


class Squirrel(_squirrel.Squirrel):
    """A squirrel package held in memory.

    Objects are added with the add_* methods, which take the object's fields as
    keyword arguments (snake_case) and return the stored object:

        pkg = squirrel.new("out.sqrl", name="My study")
        subj = pkg.add_subject("S001", sex="F")
        study = pkg.add_study(subj, modality="MR")
        pkg.add_series(study, description="T1", files=["t1.nii.gz"])
        pkg.write()

    A Squirrel object holds a Qt SQL connection and must be used from a single thread.
    """

    def _add(self, cls, **fields):
        obj = cls(self)
        for key, value in fields.items():
            setattr(obj, key, value)
        obj.store()
        return obj

    def add_subject(self, id: str, **fields) -> Subject:
        return self._add(Subject, id=id, **fields)

    def add_study(self, subject: Union[Subject, int], study_number: Optional[int] = None, **fields) -> Study:
        """Add a study to a subject. study_number defaults to the next unused number."""
        if study_number is None:
            parent = subject if isinstance(subject, Subject) else self.get_subject(subject)
            study_number = parent.next_study_number()
        return self._add(Study, subject_row_id=_row_id(subject), study_number=study_number, **fields)

    def add_series(self, study: Union[Study, int], series_number: Optional[int] = None,
                   files: Optional[Iterable[PathLike]] = None, **fields) -> Series:
        """Add a series to a study. series_number defaults to the next unused number.

        files are copied into the package when it is written.
        """
        if series_number is None:
            parent = study if isinstance(study, Study) else self.get_study(study)
            series_number = parent.next_series_number()
        if files is not None:
            files = [str(Path(f).expanduser()) for f in files]
            fields["staged_files"] = files
        return self._add(Series, study_row_id=_row_id(study), series_number=series_number, **fields)

    def add_observation(self, subject: Union[Subject, int], name: str, **fields) -> Observation:
        return self._add(Observation, subject_row_id=_row_id(subject), name=name, **fields)

    def add_intervention(self, subject: Union[Subject, int], name: str, **fields) -> Intervention:
        return self._add(Intervention, subject_row_id=_row_id(subject), name=name, **fields)

    def add_analysis(self, study: Union[Study, int], name: str, **fields) -> Analysis:
        return self._add(Analysis, study_row_id=_row_id(study), analysis_name=name, **fields)

    def add_experiment(self, name: str, files: Optional[Iterable[PathLike]] = None, **fields) -> Experiment:
        if files is not None:
            files = [str(Path(f).expanduser()) for f in files]
            fields["staged_files"] = files
        return self._add(Experiment, name=name, **fields)

    def add_pipeline(self, name: str, version: int = 1, **fields) -> Pipeline:
        return self._add(Pipeline, name=name, version=version, **fields)

    def add_group_analysis(self, name: str, **fields) -> GroupAnalysis:
        return self._add(GroupAnalysis, name=name, **fields)

    def add_data_dictionary(self, name: str, **fields) -> DataDictionary:
        return self._add(DataDictionary, name=name, **fields)

    def write(self, path: Optional[PathLike] = None) -> None:
        """Write the package to disk (to `path`, or to package_path if not given)."""
        if path is not None:
            self.package_path = str(Path(path).expanduser())
        super().write()


def new(path: PathLike, name: Optional[str] = None, overwrite: bool = False, debug: bool = False,
        data_format: str = "orig", **fields) -> Squirrel:
    """Create an empty package that will be written to `path`.

    data_format controls how series files are stored. The default, "orig", copies them
    exactly as given. The library's own default, "nifti4dgz", converts DICOM files to
    NIfTI and fails on anything else.

    Other header fields (description, license, ...) can be passed as keywords.
    """
    pkg = Squirrel(debug=debug, quiet=True)
    pkg.data_format = data_format
    pkg.set_file_mode(False)
    pkg.set_overwrite_package(overwrite)
    pkg.package_path = str(Path(path).expanduser())
    if name is not None:
        pkg.name = name
    for key, value in fields.items():
        setattr(pkg, key, value)
    return pkg


def load(path: PathLike, quick: bool = False, debug: bool = False) -> Squirrel:
    """Open an existing squirrel package and read it into memory.

    quick=True skips reading the per-series params files, which is faster.
    Raises SquirrelError if the package cannot be read.
    """
    pkg = Squirrel(debug=debug, quiet=True)
    pkg.set_file_mode(True)
    pkg.set_quick_read(quick)
    pkg.package_path = str(Path(path).expanduser())
    pkg.read()
    return pkg
