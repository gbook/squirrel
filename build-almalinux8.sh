#!/bin/bash
set -e

echo "AlmaLinux 8 detected. Enabling gcc-toolset-13"
source /opt/rh/gcc-toolset-13/enable

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
	BUILDDIR=$PWD/bin/almalinux8
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
cmake -DBIT7Z_AUTO_FORMAT:BOOL=ON -DBIT7Z_USE_LEGACY_IUNKNOWN=ON -DBIT7Z_GENERATE_PIC=ON -DCMAKE_CXX_FLAGS:STRING=-fPIC -DCMAKE_C_FLAGS:STRING=-fPIC -S $SRCDIR/bit7z -B $BUILDDIR/bit7z
cd $BUILDDIR/bit7z
cmake --build . --config Release
cp -uv $SRCDIR/bit7z/lib/x64/libbit7z64.a $BUILDDIR/bit7z/ || true
cp -uv $SRCDIR/bit7z/lib/x64/libbit7z64.a $SRCDIR/bit7z/ || true

# ----- dcm2niix (DICOM -> Nifti) -----
# No build step needed here: the dcm2niix sources are compiled directly into
# squirrellib/squirrel via src/squirrel/dcm2niix.pri (shared by both .pro files),
# so DICOM conversion is in-process and does not need an external dcm2niix binary.

# ----- build squirrel library -----
echo -e "\n ----- Building squirrel library -----\n"
$QMAKEBIN -o $BUILDDIR/squirrel/Makefile $SRCDIR/squirrel/squirrellib.pro -spec linux-g++
cd $BUILDDIR/squirrel
make -j 16

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
    sudo cp -v "$SQUIRREL_BIN" /usr/local/bin/squirrel
    sudo ldconfig
else
    echo "Warning: squirrel binary not found at $SQUIRREL_BIN, skipping install"
fi

# ----- install squirrel-gui to /usr/local/bin -----
SQUIRREL_GUI_BIN="$BUILDDIR/squirrel-gui/squirrel-gui"
if [ -f "$SQUIRREL_GUI_BIN" ]; then
    echo -e "\n ----- Installing squirrel-gui to /usr/local/bin -----\n"
    sudo cp -v "$SQUIRREL_GUI_BIN" /usr/local/bin/squirrel-gui
fi

echo -e "\nBuild complete. Output: $BUILDDIR\n"
