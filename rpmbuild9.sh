#!/bin/sh

cd ~
rm -rfv main.zip squirrel-main rpmbuild
rpmdev-setuptree
wget https://github.com/gbook/squirrel/archive/main.zip
unzip main.zip
mv squirrel-main/* rpmbuild/SOURCES/
cp -v rpmbuild/SOURCES/squirrel.el9.spec rpmbuild/SPECS/
cd rpmbuild/SPECS
QA_RPATHS=$((0x0002|0x0010)) rpmbuild -bb squirrel.el9.spec