#!/bin/bash
# Build manylinux wheels for squirrel. Requires Docker (or Podman, with
# CIBW_CONTAINER_ENGINE=podman). Wheels are written to python/wheelhouse/.
set -euo pipefail
cd "$(dirname "$0")/../.."          # repo root: the container needs src/ as well as python/
python3 -m pip install --user --upgrade cibuildwheel
python3 -m cibuildwheel --package-dir python --config-file python/pyproject.toml \
    --output-dir python/wheelhouse "$@"
