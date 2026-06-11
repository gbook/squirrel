#!/bin/bash
# Detects the current distro and delegates to the appropriate build script.
# Arguments are passed through: $1=qmake path, $2=src dir, $3=build dir

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [ ! -f /etc/os-release ]; then
    echo "Cannot detect distro: /etc/os-release not found"
    exit 1
fi

. /etc/os-release

case "$ID" in
    ubuntu)
        exec bash "$SCRIPT_DIR/build-ubuntu24.sh" "$@" ;;
    debian)
        exec bash "$SCRIPT_DIR/build-debian12.sh" "$@" ;;
    rhel|rocky|almalinux)
        case "$VERSION_ID" in
            8*) exec bash "$SCRIPT_DIR/build-rockylinux8.sh" "$@" ;;
            9*) exec bash "$SCRIPT_DIR/build-rockylinux9.sh" "$@" ;;
            *)  echo "Unsupported RHEL-family version: $VERSION_ID ($PRETTY_NAME)"; exit 1 ;;
        esac ;;
    *)
        echo "Unsupported distro: $PRETTY_NAME"
        exit 1 ;;
esac
