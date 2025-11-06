#!/bin/sh

PACKAGE=squirrel_2025.6.370
LIBDIR=$PACKAGE/lib/x86_64-linux-gnu/
BINDIR=$PACKAGE/usr/local/bin/
DEBDIR=$PACKAGE/DEBIAN

mkdir -p $LIBDIR
mkdir -p $BINDIR
mkdir -p $DEBDIR

# try to copy the binaries to their final locations (this may fail because it requires sudo, but its not a critical step to build)
cp -uv bin/squirrel/libsquirrel* $LIBDIR
cp -uv bin/gdcm/bin/libgdcm* $LIBDIR
cp -uv ~/Qt/6.9.1/gcc_64/lib/libQt6Sql.so* ~/Qt/6.9.1/gcc_64/lib/libQt6Network.so* ~/Qt/6.9.1/gcc_64/lib/libQt6Core.so* $LIBDIR
cp -uv ~/Qt/6.9.1/gcc_64/lib/libicui18n.so.56 ~/Qt/6.9.1/gcc_64/lib/libicuuc.so.56 ~/Qt/6.9.1/gcc_64/lib/libicudata.so.56 $LIBDIR

cp -uv bin/squirrel/squirrel $BINDIR

echo "Package: squirrel
Version: 2025.6.370
Section: base
Priority: optional
Architecture: amd64
Maintainer: Greg Book <gregory.a.book@gmail.com>
Description: Squirrel utilities" > $DEBDIR/control

dpkg-deb --build $PACKAGE