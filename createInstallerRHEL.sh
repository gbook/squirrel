#!/bin/sh
# Creates an RPM installer from the local source code.
# Must be run from the squirrel project root directory.

SRCDIR=$(pwd)

rm -rfv ~/rpmbuild
rpmdev-setuptree

echo "Copying local source to rpmbuild/SOURCES..."
cp -rv $SRCDIR/src $SRCDIR/build-rpm.sh $SRCDIR/squirrel.el.spec ~/rpmbuild/SOURCES/
cp -v $SRCDIR/squirrel.el.spec ~/rpmbuild/SPECS/

cd ~/rpmbuild/SPECS
QA_RPATHS=$((0x0002|0x0010)) rpmbuild -bb squirrel.el.spec
