Name:           squirrel
Version:        2024.11.284
Release:        1%{?dist}
Summary:        Squirrel neuroimaging utilities

License:        GPLv3
URL:            http://github.com/gbook/squirrel
#Source0:        

BuildArch:	x86_64
BuildRequires:  gcc, cmake, make
Requires:       openssl, p7zip

%description
squirrel library and utilities is a data format to allow aggregation and dissemination of neuroscience data.

%build # This section does the building. all the binary files will end up in %{builddir}
%{_sourcedir}/build-rpm.sh ~/Qt/6.6.3/gcc_64/bin/qmake %{_sourcedir}/src %{_builddir}/bin

%install # This section installs the files to the BUILDROOT dir, which is basically a copy of what the user's computer will look like after the RPM installs
mkdir -p %{buildroot}/usr/local/bin
mkdir -p %{buildroot}/usr/lib
mkdir -p %{buildroot}/usr/bin/sqldrivers

cp -f %{_builddir}/bin/squirrel/squirrel %{buildroot}/usr/local/bin/ # copy squirrel util binary
cp -f %{_builddir}/bin/squirrel/libsquirrel.so.1 %{buildroot}/usr/lib/ # copy squirrel lib
cp -f %{_builddir}/bin/gdcm/bin/libgdcmMSFF.so.3.0 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmopenjp2.so.7 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmuuid.so.3.0 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmzlib.so.3.0 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmcharls.so.2 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmCommon.so.3.0 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmDICT.so.3.0 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmDSED.so.3.0 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmexpat.so.2.0 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmIOD.so.3.0 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmjpeg8.so.3.0 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmjpeg12.so.3.0 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmjpeg16.so.3.0 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libgdcmMEXD.so.3.0 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f %{_builddir}/bin/gdcm/bin/libsocketxx.so.1.2 %{buildroot}/usr/lib/ # copy GDCM libs
cp -f ~/Qt/6.6.3/gcc_64/lib/libQt6Core.so.6 %{buildroot}/usr/lib/ # copy Qt libs
cp -f ~/Qt/6.6.3/gcc_64/lib/libicudata.so.56 %{buildroot}/usr/lib/ # copy Qt libs
cp -f ~/Qt/6.6.3/gcc_64/lib/libicui18n.so.56 %{buildroot}/usr/lib/ # copy Qt libs
cp -f ~/Qt/6.6.3/gcc_64/lib/libicuuc.so.56 %{buildroot}/usr/lib/ # copy Qt libs
cp -f ~/Qt/6.6.3/gcc_64/plugins/sqldrivers/libqsqlite.so %{buildroot}/usr/bin/sqldrivers/ # copy SQLite driver

# This section LISTS the files that are available once everything is installed, but this is NOT the specification for what files will be installed...
%files
/usr/lib/*
/usr/local/bin/*