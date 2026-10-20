"""Python interface to the squirrel data package library."""

from pathlib import Path
from typing import Union

from ._squirrel import ObjectType, Squirrel, SquirrelError, Subject

__all__ = ["ObjectType", "Squirrel", "SquirrelError", "Subject", "load"]


def load(path: Union[str, Path], quick: bool = False, debug: bool = False) -> Squirrel:
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
