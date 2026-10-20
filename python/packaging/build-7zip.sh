#!/bin/bash
# Builds the 7-Zip shared library (7z.so) that the wheel ships, from the official
# source, and copies it and its license files to <output dir>.
#
# The version must be a modern 7-Zip (23+) to match bit7z's BIT7Z_USE_LEGACY_IUNKNOWN=OFF
# ABI (see before-all.sh). 7-Zip is LGPL; the RAR code in it is under the unRAR
# restriction, so unRarLicense.txt must stay with any redistribution.
#
# Usage: build-7zip.sh <output dir> [work dir]
set -euo pipefail

OUT=${1:?usage: build-7zip.sh <output dir> [work dir]}
WORK=${2:-$(mktemp -d)}
VERSION=23.01
SHA256=d789bf8f092f0931302a52f48971247cca00c778804a01d73b0c38c427075d8c

mkdir -p "$OUT" "$WORK"
cd "$WORK"
curl -fsSL -o 7zip.tar.gz "https://github.com/ip7z/7zip/archive/refs/tags/$VERSION.tar.gz"
echo "$SHA256  7zip.tar.gz" | sha256sum -c -
tar xzf 7zip.tar.gz

cd "7zip-$VERSION/CPP/7zip/Bundles/Format7zF"
make -f makefile.gcc -j"$(nproc)"
cp _o/7z.so "$OUT/7z.so"
cp "$WORK/7zip-$VERSION/DOC/License.txt" "$OUT/7zip-License.txt"
cp "$WORK/7zip-$VERSION/DOC/unRarLicense.txt" "$OUT/7zip-unRarLicense.txt"
cp "$WORK/7zip-$VERSION/DOC/copying.txt" "$OUT/7zip-LGPL-copying.txt"
echo "7z.so ($VERSION) written to $OUT"
