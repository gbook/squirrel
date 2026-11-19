#!/bin/bash
# Builds squirrel (bit7z, FTXUI, libsquirrel, the squirrel CLI and squirrel-gui) for Debian 12
# with CMake, using the top-level project in src/CMakeLists.txt.
#
# Usage: build-debian12.sh [qt-dir] [src-dir] [build-dir]
#   qt-dir     Qt install prefix (default ~/Qt/6.9.3/gcc_64). A path to its
#              bin/qmake is also accepted, for older callers.
#   src-dir    default ./src
#   build-dir  default ./bin/debian12
# Set SKIP_CLI=1 to build only libsquirrel (no FTXUI, no CLI, no GUI, no install).
set -e

SKIP_CLI="${SKIP_CLI:-0}"

QTDIR="${1:-$HOME/Qt/6.9.3/gcc_64}"
case "$QTDIR" in */bin/qmake) QTDIR="$(dirname "$(dirname "$QTDIR")")" ;; esac
SRCDIR="${2:-$PWD/src}"
BUILDDIR="${3:-$PWD/bin/debian12}"

# bit7z must match the 7-Zip library loaded at runtime. squirrel builds and ships
# its own modern 7-Zip (7z.so, next to the binaries), so this is OFF on every
# distro. ON (legacy p7zip 16.02 ABI) would segfault inside 7z.so as soon as a
# package is written.
BIT7Z_LEGACY_IUNKNOWN=OFF

command -v make  >/dev/null 2>&1 || { echo -e "\nThis script requires make, but it is not installed\n"; exit 1; }
command -v gcc   >/dev/null 2>&1 || { echo -e "\nThis script requires gcc, but it is not installed\n"; exit 1; }
command -v cmake >/dev/null 2>&1 || { echo -e "\nThis script requires cmake 3.16 or newer\n"; exit 1; }

if [ "$SKIP_CLI" = "1" ]; then BUILD_CLI=OFF; BUILD_GUI=OFF; else BUILD_CLI=ON; BUILD_GUI=ON; fi

# CMakeCache.txt stores the absolute source path. build-all.sh builds the local
# host from the real path but the WSL distros through the /mnt/wsl bind mount,
# and cmake aborts if the spelling differs ("CMakeCache.txt directory ... is
# different"). Drop a cache written for another path so configure re-runs clean.
if [ -f "$BUILDDIR/CMakeCache.txt" ] && ! grep -qxF "CMAKE_HOME_DIRECTORY:INTERNAL=$SRCDIR" "$BUILDDIR/CMakeCache.txt"; then
	echo "Removing stale CMake cache in $BUILDDIR"
	rm -rf "$BUILDDIR/CMakeCache.txt" "$BUILDDIR/CMakeFiles"
fi
mkdir -p "$BUILDDIR"

echo -e "\n ----- Configuring squirrel (CMake) -----\n"
cmake -S "$SRCDIR" -B "$BUILDDIR" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_PREFIX_PATH="$QTDIR" \
	-DBIT7Z_USE_LEGACY_IUNKNOWN=$BIT7Z_LEGACY_IUNKNOWN \
	-DSQUIRREL_BUILD_CLI=$BUILD_CLI \
	-DSQUIRREL_BUILD_GUI=$BUILD_GUI

echo -e "\n ----- Building squirrel -----\n"
cmake --build "$BUILDDIR" -j 16

# ----- install squirrel to /usr/local/bin -----
SQUIRREL_BIN="$BUILDDIR/squirrel/squirrel"
if [ "$SKIP_CLI" = "1" ]; then
    echo -e "\n ----- Skipping squirrel install (SKIP_CLI=1) -----\n"
elif [ -f "$SQUIRREL_BIN" ]; then
    echo -e "\n ----- Installing squirrel to /usr/local/bin -----\n"
    sudo cp -v "$SQUIRREL_BIN" /usr/local/bin/squirrel || echo "WARNING: could not install squirrel to /usr/local/bin (compile succeeded; install skipped)"
    # the 7-Zip library squirrel loads (built next to the binary)
    sudo cp -v "$BUILDDIR"/squirrel/7z.so "$BUILDDIR"/squirrel/7zip-*.txt /usr/local/bin/ || echo "WARNING: could not install 7z.so to /usr/local/bin"
    sudo ldconfig || true
else
    echo "Warning: squirrel binary not found at $SQUIRREL_BIN, skipping install"
fi

# ----- install squirrel-gui to /usr/local/bin (built only when Qt Widgets is available) -----
SQUIRREL_GUI_BIN="$BUILDDIR/squirrel-gui/squirrel-gui"
if [ "$SKIP_CLI" != "1" ] && [ -f "$SQUIRREL_GUI_BIN" ]; then
    echo -e "\n ----- Installing squirrel-gui to /usr/local/bin -----\n"
    sudo cp -v "$SQUIRREL_GUI_BIN" /usr/local/bin/squirrel-gui || echo "WARNING: could not install squirrel-gui to /usr/local/bin"
fi

echo -e "\nBuild complete. Output: $BUILDDIR\n"
