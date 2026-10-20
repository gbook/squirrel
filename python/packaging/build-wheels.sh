#!/bin/bash
# Build manylinux wheels for squirrel. Requires Docker, or Podman (selected automatically
# when Docker is absent). Wheels are written to python/wheelhouse/.
#
# The container build needs only src/ and python/. cibuildwheel copies its whole project
# directory into the container, so those two are staged into a temp directory first;
# otherwise bin/, installers/ and .git (hundreds of MB) would be copied each time.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OUT="$ROOT/python/wheelhouse"

if ! command -v docker >/dev/null 2>&1 && command -v podman >/dev/null 2>&1; then
    export CIBW_CONTAINER_ENGINE=podman
fi

STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT
tar -C "$ROOT" \
    --exclude='src/bit7z-old' --exclude='src/bit7z/lib' \
    --exclude='python/wheelhouse' --exclude='python/build' \
    --exclude='__pycache__' --exclude='*.egg-info' --exclude='*.pro.user' \
    -cf - src python | tar -C "$STAGE" -xf -

mkdir -p "$OUT"
cd "$STAGE"
python3 -m pip install --user --upgrade cibuildwheel
python3 -m cibuildwheel --package-dir python --config-file python/pyproject.toml \
    --output-dir "$OUT" "$@"
