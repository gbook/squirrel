#!/bin/bash
# Builds everything the squirrel wheel links against, inside a manylinux_2_28
# (AlmaLinux 8) container: Qt, DCMTK, bit7z, and libsquirrel.a. Runs once per
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
if [ ! -x /opt/Qt/$QT_VERSION/gcc_64/bin/qmake ]; then
    aqt install-qt linux desktop "$QT_VERSION" linux_gcc_64 -O /opt/Qt
fi
QMAKE=/opt/Qt/$QT_VERSION/gcc_64/bin/qmake

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

# ----- bit7z -----
# bit7z forces its archive into the source tree (src/bit7z/lib/x64), which the
# python CMakeLists reads from by default. Clear any archive built elsewhere.
rm -f "$PROJECT"/src/bit7z/lib/x64/*.a "$PROJECT"/src/bit7z/*.a
cmake -G Ninja -S "$PROJECT/src/bit7z" -B "$DEPS/bit7z-build" -DCMAKE_BUILD_TYPE=Release \
    -DBIT7Z_AUTO_FORMAT=ON -DBIT7Z_USE_LEGACY_IUNKNOWN="$BIT7Z_LEGACY_IUNKNOWN" -DBIT7Z_GENERATE_PIC=ON \
    -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_C_FLAGS=-fPIC
cmake --build "$DEPS/bit7z-build"
# squirrel-deps.pri finds bit7z at ../bit7z relative to the libsquirrel build directory
mkdir -p "$DEPS/bit7z"
cp "$PROJECT/src/bit7z/lib/x64/libbit7z64.a" "$DEPS/bit7z/"
ln -sfn "$PROJECT/src/bit7z/include" "$DEPS/bit7z/include"

# ----- libsquirrel.a (qmake, as for the command line tool) -----
mkdir -p "$DEPS/squirrel"
cd "$DEPS/squirrel"
"$QMAKE" "$PROJECT/src/squirrel/squirrellib.pro" -spec linux-g++ CONFIG+=release
make -j"$(nproc)"
ls -l "$DEPS/squirrel/libsquirrel.a"
