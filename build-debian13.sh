#!/bin/bash
set -e

SKIP_GUI=1

if [ -z "$1" ]; then
	QMAKEBIN=~/Qt/6.9.3/gcc_64/bin/qmake
else
	QMAKEBIN=$1
fi

if [ -z "$2" ]; then
	SRCDIR=$PWD/src
else
	SRCDIR=$2
fi

if [ -z "$3" ]; then
	BUILDDIR=$PWD/bin/debian13
else
	BUILDDIR=$3
fi

ORIGDIR=$PWD

command -v make >/dev/null 2>&1 || { echo -e "\nThis script requires make, but it is not installed\n"; exit 1; }
command -v gcc >/dev/null 2>&1 || { echo -e "\nThis script requires gcc, but it is not installed\n"; exit 1; }
command -v cmake >/dev/null 2>&1 || { echo -e "\nThis script requires cmake 3.x.\n"; exit 1; }

echo "Creating build directory: $BUILDDIR"
mkdir -p $BUILDDIR

# ----- build bit7z library -----
echo -e "\n ----- Building bit7z -----\n"
mkdir -p $BUILDDIR/bit7z
# CMakeCache.txt stores absolute source/binary paths. build-all.sh configures the
# local host from the real path but the WSL distros via the /mnt/wsl bind mount, so
# a cache written under a different path spelling makes cmake abort ("CMakeCache.txt
# directory ... is different"). Drop a stale/mismatched cache so configure re-runs clean.
if [ -f "$BUILDDIR/bit7z/CMakeCache.txt" ] && ! grep -qxF "CMAKE_HOME_DIRECTORY:INTERNAL=$SRCDIR/bit7z" "$BUILDDIR/bit7z/CMakeCache.txt"; then
	echo "Removing stale bit7z CMake cache in $BUILDDIR/bit7z"
	rm -rf "$BUILDDIR/bit7z"
	mkdir -p "$BUILDDIR/bit7z"
fi
cmake -DBIT7Z_AUTO_FORMAT:BOOL=ON -DBIT7Z_USE_LEGACY_IUNKNOWN=OFF -DBIT7Z_GENERATE_PIC=ON -DCMAKE_CXX_FLAGS:STRING=-fPIC -DCMAKE_C_FLAGS:STRING=-fPIC -S $SRCDIR/bit7z -B $BUILDDIR/bit7z
cd $BUILDDIR/bit7z
# bit7z (cmake/OutputOptions.cmake) forces its archive into the SHARED source
# tree ($SRCDIR/bit7z/lib/x64), which build-all.sh bind-mounts across every
# distro. Delete any archive left by another distro/toolchain so the build below
# relinks libbit7z64.a from THIS distro's objects. Otherwise e.g. AlmaLinux 8
# (gcc-toolset-10) links a lib built by Ubuntu 24 (GCC 13) and fails on missing
# GCC12/glibc2.32 symbols (exception_ptr::_M_release, __libc_single_threaded, ...).
rm -f "$SRCDIR/bit7z/lib/x64/"*.a "$SRCDIR/bit7z/"*.a 2>/dev/null || true
cmake --build . --config Release
cp -v $SRCDIR/bit7z/lib/x64/libbit7z64.a $BUILDDIR/bit7z/ || true
cp -v $SRCDIR/bit7z/lib/x64/libbit7z64.a $SRCDIR/bit7z/ || true

# ----- build FTXUI library (static; for the interactive 'squirrel explore' shell) -----
# FTXUI is built as static libs and linked into the CLI via ftxui.pri (same
# pattern as bit7z). Unlike bit7z it writes its .a files into the build dir (not
# the shared source tree), so no cross-distro archive cleanup is needed. The
# stale-cache guard mirrors bit7z: under build-all the WSL distros configure via
# the /mnt/wsl bind-mount path, so a cache written under a different path
# spelling would make cmake abort.
echo -e "\n ----- Building FTXUI -----\n"
mkdir -p $BUILDDIR/ftxui
if [ -f "$BUILDDIR/ftxui/CMakeCache.txt" ] && ! grep -qxF "CMAKE_HOME_DIRECTORY:INTERNAL=$SRCDIR/ftxui" "$BUILDDIR/ftxui/CMakeCache.txt"; then
	echo "Removing stale FTXUI CMake cache in $BUILDDIR/ftxui"
	rm -rf "$BUILDDIR/ftxui"
	mkdir -p "$BUILDDIR/ftxui"
fi
cmake -S $SRCDIR/ftxui -B $BUILDDIR/ftxui -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DFTXUI_BUILD_EXAMPLES=OFF -DFTXUI_BUILD_TESTS=OFF -DFTXUI_BUILD_DOCS=OFF -DFTXUI_ENABLE_INSTALL=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build $BUILDDIR/ftxui --config Release -j 16

# ----- dcm2niix (DICOM -> Nifti) -----
# No build step needed here: the dcm2niix sources are compiled directly into
# squirrellib/squirrel via src/squirrel/dcm2niix.pri (shared by both .pro files),
# so DICOM conversion is in-process and does not need an external dcm2niix binary.

# ----- build squirrel library -----
echo -e "\n ----- Building squirrel library -----\n"
$QMAKEBIN -o $BUILDDIR/squirrel/Makefile $SRCDIR/squirrel/squirrellib.pro -spec linux-g++
cd $BUILDDIR/squirrel
make -B -j 16

# ----- build squirrel command line utilities -----
echo -e "\n ----- Building squirrel utilities -----\n"
$QMAKEBIN -o $BUILDDIR/squirrel/Makefile $SRCDIR/squirrel/squirrel.pro -spec linux-g++
cd $BUILDDIR/squirrel
make -B -j 16

# ----- build squirrel GUI -----
# The GUI is optional. It needs the Qt Widgets module and OpenGL development
# files, which are not installed on every build host, so a GUI failure is a
# warning rather than a build failure - the command line utility is the primary
# artifact. Set SKIP_GUI=1 to skip this step entirely.
#
# The GUI builds into its own directory, which MUST remain a sibling of
# $BUILDDIR/squirrel and $BUILDDIR/bit7z: squirrel-gui.pro and squirrel-deps.pri
# locate libsquirrel and bit7z through build-directory-relative paths
# (../squirrel and ../bit7z), not through absolute ones.
if [ "$SKIP_GUI" = "1" ]; then
	echo -e "\n ----- Skipping squirrel GUI (SKIP_GUI=1) -----\n"
elif [ ! -d "$($QMAKEBIN -query QT_INSTALL_HEADERS)/QtWidgets" ]; then
	echo -e "\n ----- Skipping squirrel GUI: Qt Widgets headers not found -----\n"
else
	echo -e "\n ----- Building squirrel GUI -----\n"
	if ( $QMAKEBIN -o $BUILDDIR/squirrel-gui/Makefile $SRCDIR/squirrel-gui/squirrel-gui.pro -spec linux-g++ && cd $BUILDDIR/squirrel-gui && make -B -j 16 ); then
		echo -e "\n ----- squirrel GUI built -----\n"
	else
		echo -e "\n ----- WARNING: squirrel GUI failed to build. Continuing without it. -----\n"
	fi
fi

cd $ORIGDIR

# ----- install squirrel to /usr/local/bin -----
SQUIRREL_BIN="$BUILDDIR/squirrel/squirrel"
if [ -f "$SQUIRREL_BIN" ]; then
    echo -e "\n ----- Installing squirrel to /usr/local/bin -----\n"
    sudo cp -v "$SQUIRREL_BIN" /usr/local/bin/squirrel || echo "WARNING: could not install squirrel to /usr/local/bin (compile succeeded; install skipped)"
    sudo ldconfig || true
else
    echo "Warning: squirrel binary not found at $SQUIRREL_BIN, skipping install"
fi

# ----- install squirrel-gui to /usr/local/bin -----
SQUIRREL_GUI_BIN="$BUILDDIR/squirrel-gui/squirrel-gui"
if [ -f "$SQUIRREL_GUI_BIN" ]; then
    echo -e "\n ----- Installing squirrel-gui to /usr/local/bin -----\n"
    sudo cp -v "$SQUIRREL_GUI_BIN" /usr/local/bin/squirrel-gui || echo "WARNING: could not install squirrel-gui to /usr/local/bin"
fi

echo -e "\nBuild complete. Output: $BUILDDIR\n"
