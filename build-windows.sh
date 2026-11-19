#!/bin/bash
# Builds squirrel (CLI + GUI) for Windows and creates the .msi installer, driven
# from WSL. The compile and packaging run with the Windows tools (MSVC, Qt,
# CMake, WiX) through WSL interop; nothing is cross-compiled.
#
# Usage: build-windows.sh [win-repo]
#   win-repo   the squirrel checkout on the Windows drive (default /mnt/c/squirrel)
#
# Steps:
#   1. Copy src/ and LICENSE from this repo into <win-repo> (rsync; never deletes,
#      skips build folders; bin/dcmtk-bin is untouched). SKIP_SYNC=1 skips it.
#   2. Configure + build Release with the Visual Studio 2022 generator in
#      <win-repo>/src/build-msi.
#   3. cpack -> squirrel_<version>_Windows-x64.msi, copied into ./installers/.
#
# The full output goes to ./installers/logs/squirrel_<version>_Windows.log.
#
# Windows-side requirements (paths can be overridden with the variables below):
#   Visual Studio 2022 with the C++ workload
#   Qt 6 for MSVC                  QT_DIR      (default C:/Qt/6.9.3/msvc2022_64)
#   CMake 3.21+                    WIN_CMAKE   (default C:/Program Files/CMake/bin)
#   WiX Toolset v3.14              WIX_DIR     (default <win-repo>/bin/wix314 if it
#                                               exists, else an installed WiX via %WIX%)
#   DCMTK (Release, same MSVC)     DCMTK_DIR   (default C:/Program Files (x86)/DCMTK/cmake,
#                                               the folder holding DCMTKConfig.cmake)
#   64-bit 7-Zip                   C:/Program Files/7-Zip/7z.dll (bundled in the .msi)
set -e
set -o pipefail

SRCDIR="$(cd "$(dirname "$0")" && pwd)"
WINREPO="${1:-/mnt/c/squirrel}"
QT_DIR="${QT_DIR:-C:/Qt/6.9.3/msvc2022_64}"
DCMTK_DIR="${DCMTK_DIR:-C:/Program Files (x86)/DCMTK/cmake}"
WIN_CMAKE="${WIN_CMAKE:-C:/Program Files/CMake/bin}"
SKIP_SYNC="${SKIP_SYNC:-0}"

command -v cmd.exe >/dev/null 2>&1 || { echo "cmd.exe not found: this script must run in WSL with Windows interop enabled"; exit 1; }
command -v rsync   >/dev/null 2>&1 || { echo "This script requires rsync (sudo apt install rsync)"; exit 1; }
[ -d "$WINREPO" ] || { echo "Windows checkout not found: $WINREPO"; exit 1; }
case "$WINREPO" in /mnt/[a-z]/*) ;; *) echo "The Windows checkout must be on a Windows drive (/mnt/<drive>/...), not in the WSL filesystem"; exit 1 ;; esac

CMAKE_EXE="$(wslpath -u "$WIN_CMAKE")/cmake.exe"
CPACK_EXE="$(wslpath -u "$WIN_CMAKE")/cpack.exe"
[ -x "$CMAKE_EXE" ] || { echo "cmake.exe not found in $WIN_CMAKE (set WIN_CMAKE)"; exit 1; }

WIN_SRC="$(wslpath -m "$WINREPO/src")"          # C:/squirrel/src
WIN_BUILD="$WIN_SRC/build-msi"
BUILDDIR="$WINREPO/src/build-msi"

# WiX: a copy unpacked into <win-repo>/bin/wix314 (wix314-binaries.zip, no admin
# needed) is used if present; otherwise cpack finds an installed WiX via %WIX%.
WIX_DIR="${WIX_DIR:-}"
if [ -z "$WIX_DIR" ] && [ -f "$WINREPO/bin/wix314/candle.exe" ]; then
    WIX_DIR="$(wslpath -m "$WINREPO/bin/wix314")"
fi
CPACK_WIX_ARGS=()
[ -n "$WIX_DIR" ] && CPACK_WIX_ARGS=(-D "CPACK_WIX_ROOT=$WIX_DIR")

# Version, as kept by update_version.sh.
ver() { grep -m1 "#define $1 " "$SRCDIR/src/squirrel/squirrelVersion.h" | cut -d'"' -f2; }
VERSION="$(ver UTIL_VERSION_MAJ).$(ver UTIL_VERSION_MIN).$(ver UTIL_BUILD_NUM)"
MSI="squirrel_${VERSION}_Windows-x64.msi"

OUTDIR="$SRCDIR/installers"
LOGDIR="$OUTDIR/logs"
LOG="$LOGDIR/squirrel_${VERSION}_Windows.log"
mkdir -p "$LOGDIR"
exec > >(tee "$LOG") 2>&1
echo "# Windows installer build, version $VERSION"
echo "# Started: $(date '+%Y-%m-%d %H:%M:%S')"
echo "# Source: $SRCDIR -> $WINREPO"

# Windows programs inherit the current directory; give them a Windows one so
# cmd/cmake do not start in a \\wsl.localhost path.
cd "$WINREPO"

# ----- 1. sync source -----------------------------------------------------------
if [ "$SKIP_SYNC" = "1" ]; then
    echo -e "\n ----- Skipping source sync (SKIP_SYNC=1) -----\n"
else
    echo -e "\n ----- Syncing source to $WINREPO -----\n"
    # Only what the build needs: src/ and LICENSE. --checksum: the Windows
    # checkout has CRLF copies of identical files (git autocrlf), so compare
    # content; LF copies are fine for MSVC and git still sees them as unchanged.
    rsync -r --checksum --itemize-changes \
        --exclude='/build*/' --exclude='/Desktop_Qt_*' --exclude='/.qtcreator/' \
        --exclude='*.user' --exclude='*.user.*' --exclude='/tests/' \
        "$SRCDIR/src/" "$WINREPO/src/" | grep -v '^\.' | grep -c . | xargs echo "files updated in src/:" || true
    rsync --checksum "$SRCDIR/LICENSE" "$WINREPO/LICENSE"
fi

# ----- 2. configure + build -------------------------------------------------------
echo -e "\n ----- Configuring (Visual Studio 2022, x64) -----\n"
"$CMAKE_EXE" -S "$WIN_SRC" -B "$WIN_BUILD" -G "Visual Studio 17 2022" -A x64 \
    -DCMAKE_PREFIX_PATH="$QT_DIR" \
    -DDCMTK_DIR="$DCMTK_DIR"

echo -e "\n ----- Building Release -----\n"
"$CMAKE_EXE" --build "$WIN_BUILD" --config Release --parallel

# ----- 3. installer ------------------------------------------------------------------
echo -e "\n ----- Creating $MSI -----\n"
(cd "$BUILDDIR" && "$CPACK_EXE" -C Release -G WIX "${CPACK_WIX_ARGS[@]}")

[ -f "$BUILDDIR/$MSI" ] || { echo "cpack finished but $MSI was not found in $BUILDDIR"; exit 1; }
mkdir -p "$OUTDIR"
cp -v "$BUILDDIR/$MSI" "$OUTDIR/"
echo -e "\nDone: $OUTDIR/$MSI"
echo "# Finished: $(date '+%Y-%m-%d %H:%M:%S')"
