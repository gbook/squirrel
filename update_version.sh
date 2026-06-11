#!/bin/bash

YEAR=$(date +%Y)
MONTH=$(date +%-m)
COMMITS=$(git rev-list --count HEAD)
VERSION="${YEAR}.${MONTH}.${COMMITS}"

echo "Updating version to $VERSION"

# squirrelVersion.h
sed -i "s/#define UTIL_VERSION_MAJ \"[^\"]*\"/#define UTIL_VERSION_MAJ \"${YEAR}\"/" src/squirrel/squirrelVersion.h
sed -i "s/#define UTIL_VERSION_MIN \"[^\"]*\"/#define UTIL_VERSION_MIN \"${MONTH}\"/" src/squirrel/squirrelVersion.h
sed -i "s/#define UTIL_BUILD_NUM \"[^\"]*\"/#define UTIL_BUILD_NUM \"${COMMITS}\"/" src/squirrel/squirrelVersion.h

# squirrel.el.spec
sed -i "s/^Version:.*/Version:        ${VERSION}/" squirrel.el.spec

# createInstallerDebian.sh
sed -i "s/^PACKAGE=squirrel_.*/PACKAGE=squirrel_${VERSION}/" createInstallerDebian.sh
sed -i "s/^Version:.*/Version: ${VERSION}/" createInstallerDebian.sh

echo "Done."
