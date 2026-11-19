#!/bin/bash
# Compile step used inside the rpm build (squirrel.el.spec %build). Builds
# bit7z, FTXUI, libsquirrel and the squirrel CLI with CMake from
# src/CMakeLists.txt. Unlike build-<distro>.sh it does not install anything;
# the spec's %install section picks up $BUILDDIR/squirrel/squirrel.
#
# Usage: build-rpm.sh [qt-dir] [src-dir] [build-dir]
#   qt-dir     Qt install prefix (default ~/Qt/6.9.3/gcc_64). A path to its
#              bin/qmake is also accepted, for older callers.
#   src-dir    default ./src
#   build-dir  default ./bin
set -e

if grep -q -i "release 8" /etc/redhat-release 2>/dev/null; then
	echo "RHEL 8 detected. Enabling gcc-toolset-10"
	source /opt/rh/gcc-toolset-10/enable
fi

QTDIR="${1:-$HOME/Qt/6.9.3/gcc_64}"
case "$QTDIR" in */bin/qmake) QTDIR="$(dirname "$(dirname "$QTDIR")")" ;; esac
SRCDIR="${2:-$PWD/src}"
BUILDDIR="${3:-$PWD/bin}"

# The rpm ships its own modern 7-Zip (7z.so, built by the CMake project next to
# the squirrel binary), so bit7z uses the modern ABI.
BIT7Z_LEGACY_IUNKNOWN=OFF

command -v make  >/dev/null 2>&1 || { echo -e "\nThis script requires make, but it is not installed\n"; exit 1; }
command -v gcc   >/dev/null 2>&1 || { echo -e "\nThis script requires gcc, but it is not installed\n"; exit 1; }
command -v cmake >/dev/null 2>&1 || { echo -e "\nThis script requires cmake 3.16 or newer\n"; exit 1; }

mkdir -p "$BUILDDIR"

echo -e "\n ----- Configuring squirrel (CMake) -----\n"
cmake -S "$SRCDIR" -B "$BUILDDIR" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_PREFIX_PATH="$QTDIR" \
	-DBIT7Z_USE_LEGACY_IUNKNOWN=$BIT7Z_LEGACY_IUNKNOWN \
	-DSQUIRREL_BUILD_CLI=ON \
	-DSQUIRREL_BUILD_GUI=OFF

echo -e "\n ----- Building squirrel -----\n"
cmake --build "$BUILDDIR" -j 16

echo -e "\nBuild complete. Output: $BUILDDIR\n"
