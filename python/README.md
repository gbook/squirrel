# squirrel (Python)

```python
import squirrel
pkg = squirrel.load("study.sqrl")
for subj in pkg.get_subject_list():
    print(subj.id, subj.sex)
```

Build (the C++ library is built separately with qmake, see `src/squirrel/squirrellib.pro`):

```bash
python -m venv .venv && . .venv/bin/activate
pip install . --config-settings=cmake.define.QT_ROOT=$HOME/Qt/6.9.3/gcc_64
```

Thread safety: a `Squirrel` object holds a Qt SQL connection and must be used from the
thread that created it.

## Building wheels

Portable Linux wheels are built inside a `manylinux_2_28` (AlmaLinux 8) container with
[cibuildwheel](https://cibuildwheel.pypa.io), which requires Docker or Podman:

```bash
python/packaging/build-wheels.sh        # wheels land in python/wheelhouse/
```

`packaging/before-all.sh` builds Qt (via `aqtinstall`), DCMTK, bit7z and `libsquirrel.a` in the
container; `auditwheel repair` then copies Qt, ICU, DCMTK and OpenSSL into the wheel's
`squirrel.libs/`. Qt's SQLite driver is a plugin, so it is shipped in `squirrel/qt_plugins/`
and located through `QT_PLUGIN_PATH`, which `squirrel/__init__.py` sets.

`makeAllInstallers.sh` runs this as its last step and collects the wheels in `installers/`
(set `SKIP_WHEELS=1` to leave them out; it skips them itself if Docker/Podman is missing).
`update_version.sh` keeps `version` in `pyproject.toml` in step with the rest.

A wheel built directly on a development machine (`pip wheel .`) is tied to that machine's glibc
and is only for local testing.

## 7-Zip

libsquirrel loads a 7-Zip shared library (`7z.so`) at runtime. Wheels ship their own, built from
the official 7-Zip 23.01 source by `packaging/build-7zip.sh`, so no system 7-Zip library is needed. It
lives in `squirrel/lib7z/` together with its licenses (LGPL, plus the unRAR restriction), which
must be kept with any redistribution. `squirrel/__init__.py` points the `SQUIRREL_7ZIP_LIB`
environment variable at it; libsquirrel checks that variable before the system locations
(`/usr/lib/7zip`, `/usr/libexec/p7zip`, ...). Set `SQUIRREL_7ZIP_LIB` yourself to use a different
library.

bit7z is compiled for one 7-Zip ABI (`BIT7Z_USE_LEGACY_IUNKNOWN`), and a mismatch crashes inside
`7z.so`. The bundled library is modern 7-Zip, so the wheel build uses `OFF`
(`packaging/before-all.sh`). Do not point `SQUIRREL_7ZIP_LIB` at an old p7zip 16.02 library.

A source install (`pip install .`) has no bundled library unless you pass
`--config-settings=cmake.define.SQUIRREL_7ZIP_DIR=<dir containing 7z.so>`; otherwise the system
7-Zip is used.
