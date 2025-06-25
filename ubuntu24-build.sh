#!/bin/sh

# global build variables
if [ -z "$1" ]; then
	QMAKEBIN=~/Qt/6.9.1/gcc_64/bin/qmake
else
	QMAKEBIN=$1
fi

if [ -z "$2" ]; then
	SRCDIR=$PWD/src
else
	SRCDIR=$2
fi

if [ -z "$3" ]; then
	BUILDDIR=$PWD/bin
else
	BUILDDIR=$3
fi

BIT7ZDIR=$PWD/bit7z
ORIGDIR=$PWD

# this script requires make, cmake (3), and qmake
command -v make >/dev/null 2>&1 || { echo -e "\nThis script requires make, but it is not installed\n"; exit 1; }
command -v gcc >/dev/null 2>&1 || { echo -e "\nThis script requires gcc, but it is not installed\n"; exit 1; }

# create the build directory
echo "Creating build directory"
mkdir -p $BUILDDIR

# ----- build pre-requisites -----
command -v cmake >/dev/null 2>&1 || { echo -e "\nThis script requires cmake 3.x. Install using 'yum install cmake' or 'apt-get cmake'.\n"; exit 1; }

# ----- build GDCM library -----
echo -e "\n ----- Building gdcm -----\n"
mkdir -p $BUILDDIR/gdcm
cd $BUILDDIR/gdcm
cmake -DGDCM_BUILD_APPLICATIONS:STRING=NO -DGDCM_BUILD_DOCBOOK_MANPAGES:BOOL=OFF -DGDCM_BUILD_SHARED_LIBS:STRING=YES -DGDCM_BUILD_TESTING:STRING=NO -DGDCM_BUILD_EXAMPLES:STRING=NO $SRCDIR/gdcm
make -j 16

# ----- build bit7z library -----
echo -e "\n ----- Building bit7z -----\n"
echo -e "\n ----- Created path $BUILDDIR/bit7z -----\n"
mkdir -p $BUILDDIR/bit7z
echo -e "\n ----- Running cmake -DBIT7Z_AUTO_FORMAT:BOOL=ON -DBIT7Z_USE_LEGACY_IUNKNOWN=OFF -DBIT7Z_GENERATE_PIC=ON -DCMAKE_CXX_FLAGS:STRING=-fPIC -DCMAKE_C_FLAGS:STRING=-fPIC -S $SRCDIR/bit7z -B $BUILDDIR/bit7z -----\n"
cmake -DBIT7Z_AUTO_FORMAT:BOOL=ON -DBIT7Z_USE_LEGACY_IUNKNOWN=OFF -DBIT7Z_GENERATE_PIC=ON -DCMAKE_CXX_FLAGS:STRING=-fPIC -DCMAKE_C_FLAGS:STRING=-fPIC -S $SRCDIR/bit7z -B $BUILDDIR/bit7z
echo -e "\n ----- chdir to $BUILDDIR/bit7z -----\n"
cd $BUILDDIR/bit7z
echo -e "\n ----- Running cmake --build . --config Release -----\n"
cmake --build . --config Release
cp -uv $SRCDIR/bit7z/lib/x64/libbit7z64.a $BUILDDIR/bit7z/
cp -uv $SRCDIR/bit7z/lib/x64/libbit7z64.a $SRCDIR/bit7z/

#echo -e "\n ----- Building bit7z -----\n"
#mkdir -p $BIT7ZDIR/build
#cd $BIT7ZDIR/build
#cmake .. -DBIT7Z_AUTO_FORMAT:BOOL=ON -DBIT7Z_USE_LEGACY_IUNKNOWN=ON -DBIT7Z_GENERATE_PIC=ON -DCMAKE_CXX_FLAGS:STRING=-fPIC -DCMAKE_C_FLAGS:STRING=-fPIC
#cmake --build . --config Release

#cmake -DBIT7Z_AUTO_FORMAT:BOOL=ON -DCMAKE_CXX_FLAGS:STRING=-fPIC -DCMAKE_C_FLAGS:STRING=-fPIC $SRCDIR/bit7z
#make -j 16
#echo -e "\nCopying bit7z library to $BUILDDIR\n"
#mkdir -pv $BUILDDIR/../bit7z/lib/x64
#cp -uv $SRCDIR/bit7z/lib/x64/* $BUILDDIR/../bit7z/lib/x64
#mkdir -pv $BUILDDIR/bit7z
#cp -uv $SRCDIR/bit7z/lib/x64/* $BUILDDIR/bit7z/

# ----- build squirrel library -----
echo -e "\n ----- Building squirrel library -----\n"
echo $QMAKEBIN -o $BUILDDIR/squirrel/Makefile $SRCDIR/squirrel/squirrellib.pro -spec linux-g++
$QMAKEBIN -o $BUILDDIR/squirrel/Makefile $SRCDIR/squirrel/squirrellib.pro -spec linux-g++
cd $BUILDDIR/squirrel
make -B -j 16

# ----- build squirrel command line utilities -----
echo -e "\n ----- Building squirrel utilities -----\n"
echo $QMAKEBIN -o $BUILDDIR/squirrel/Makefile $SRCDIR/squirrel/squirrel.pro -spec linux-g++
$QMAKEBIN -o $BUILDDIR/squirrel/Makefile $SRCDIR/squirrel/squirrel.pro -spec linux-g++
cd $BUILDDIR/squirrel
make -B -j 16

# install
# try to copy the binaries to their final locations (this may fail because it requires sudo)
cd $ORIGDIR
echo -e "\nCopying libsquirrel to /lib"
sudo cp -uv bin/squirrel/libsquirrel* /lib/

echo -e "\nCopying squirrel to /usr/local/bin"
sudo cp -uv bin/squirrel/squirrel /usr/local/bin/
