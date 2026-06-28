#!/bin/bash
set -e

echo "RHEL/Rocky 8 detected. Enabling gcc-toolset-10"
source /opt/rh/gcc-toolset-10/enable

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
	BUILDDIR=$PWD/bin/rockylinux8
else
	BUILDDIR=$3
fi

ORIGDIR=$PWD

command -v make >/dev/null 2>&1 || { echo -e "\nThis script requires make, but it is not installed\n"; exit 1; }
command -v gcc >/dev/null 2>&1 || { echo -e "\nThis script requires gcc, but it is not installed\n"; exit 1; }
command -v cmake >/dev/null 2>&1 || { echo -e "\nThis script requires cmake 3.x.\n"; exit 1; }

echo "Creating build directory: $BUILDDIR"
mkdir -p $BUILDDIR

# ----- build GDCM library -----
#echo -e "\n ----- Building gdcm -----\n"
#mkdir -p $BUILDDIR/gdcm
#cd $BUILDDIR/gdcm
#cmake -DGDCM_BUILD_APPLICATIONS:STRING=NO -DGDCM_BUILD_DOCBOOK_MANPAGES:BOOL=OFF -DGDCM_BUILD_SHARED_LIBS:STRING=YES -DGDCM_BUILD_TESTING:STRING=NO -DGDCM_BUILD_EXAMPLES:STRING=NO $SRCDIR/gdcm
#make -j 16

# ----- build bit7z library -----
echo -e "\n ----- Building bit7z -----\n"
mkdir -p $BUILDDIR/bit7z
cmake -DBIT7Z_AUTO_FORMAT:BOOL=ON -DBIT7Z_USE_LEGACY_IUNKNOWN=ON -DBIT7Z_GENERATE_PIC=ON -DCMAKE_CXX_FLAGS:STRING=-fPIC -DCMAKE_C_FLAGS:STRING=-fPIC -S $SRCDIR/bit7z -B $BUILDDIR/bit7z
cd $BUILDDIR/bit7z
cmake --build . --config Release
cp -uv $SRCDIR/bit7z/lib/x64/libbit7z64.a $BUILDDIR/bit7z/ || true
cp -uv $SRCDIR/bit7z/lib/x64/libbit7z64.a $SRCDIR/bit7z/ || true

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

echo -e "\nBuild complete. Output: $BUILDDIR\n"
