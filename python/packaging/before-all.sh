#!/bin/bash
# Builds everything the squirrel wheel links against, inside a manylinux_2_28
# (AlmaLinux 8) container: Qt, DCMTK, then bit7z and libsquirrel.a. Runs once per
# container (cibuildwheel "before-all"); the results are reused by every Python
# version's wheel build.
#
# Usage: before-all.sh <project dir>     (the repo root, /project under cibuildwheel)
set -euo pipefail

PROJECT=${1:-/project}
DEPS=/opt/squirrel-deps
QT_VERSION=${QT_VERSION:-6.9.3}
DCMTK_VERSION=${DCMTK_VERSION:-3.7.0}
# Must match the 7-Zip library the wheel ships (built below by build-7zip.sh, a modern
# 7-Zip), so OFF. ON is the p7zip 16.02 ABI. A mismatch crashes inside 7z.so.
BIT7Z_LEGACY_IUNKNOWN=${BIT7Z_LEGACY_IUNKNOWN:-OFF}

mkdir -p "$DEPS"

yum install -y openssl-devel zlib-devel libxml2-devel
PY=/opt/python/cp312-cp312/bin
$PY/pip install cmake ninja aqtinstall
export PATH=$PY:$PATH

# ----- Qt (Core + Sql, via aqtinstall) -----
if [ ! -f /opt/Qt/$QT_VERSION/gcc_64/lib/cmake/Qt6/Qt6Config.cmake ]; then
    aqt install-qt linux desktop "$QT_VERSION" linux_gcc_64 -O /opt/Qt
fi

# ----- DCMTK (shared, into /usr/local like the other build scripts expect) -----
if [ ! -f /usr/local/lib/libdcmdata.so ] && [ ! -f /usr/local/lib64/libdcmdata.so ]; then
    curl -fsSL "https://github.com/DCMTK/dcmtk/archive/refs/tags/DCMTK-$DCMTK_VERSION.tar.gz" | tar xz -C "$DEPS"
    cmake -G Ninja -S "$DEPS/dcmtk-DCMTK-$DCMTK_VERSION" -B "$DEPS/dcmtk-build" \
        -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DBUILD_APPS=OFF \
        -DDCMTK_WITH_OPENSSL=ON -DDCMTK_WITH_XML=ON -DDCMTK_WITH_ZLIB=ON \
        -DDCMTK_WITH_TIFF=OFF -DDCMTK_WITH_PNG=OFF -DDCMTK_WITH_SNDFILE=OFF \
        -DCMAKE_INSTALL_PREFIX=/usr/local
    cmake --build "$DEPS/dcmtk-build"
    cmake --install "$DEPS/dcmtk-build"
fi
echo /usr/local/lib > /etc/ld.so.conf.d/squirrel-local.conf
echo /usr/local/lib64 >> /etc/ld.so.conf.d/squirrel-local.conf
ldconfig

# ----- 7-Zip shared library, bundled into the wheel -----
"$PROJECT/python/packaging/build-7zip.sh" "$DEPS/7zip" "$DEPS/7zip-work"

# ----- bit7z + libsquirrel.a (CMake, library only: no FTXUI, no CLI) -----
# src/CMakeLists.txt builds bit7z as part of the same project and keeps its
# archive in the build directory, so the outputs are:
#   $DEPS/build/squirrel/libsquirrel.a
#   $DEPS/build/bit7z/libbit7z64.a
# pyproject.toml points python/CMakeLists.txt at $DEPS/build (SQUIRREL_BUILD_DIR).
cmake -G Ninja -S "$PROJECT/src" -B "$DEPS/build" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="/opt/Qt/$QT_VERSION/gcc_64" \
    -DBIT7Z_USE_LEGACY_IUNKNOWN="$BIT7Z_LEGACY_IUNKNOWN" \
    -DSQUIRREL_BUILD_CLI=OFF
cmake --build "$DEPS/build"
ls -l "$DEPS/build/squirrel/libsquirrel.a" "$DEPS/build/bit7z/libbit7z64.a"
