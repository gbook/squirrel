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
        case "$VERSION_ID" in
            22*) exec bash "$SCRIPT_DIR/build-ubuntu22.sh" "$@" ;;
            24*) exec bash "$SCRIPT_DIR/build-ubuntu24.sh" "$@" ;;
            *)   echo "Unsupported Ubuntu version: $VERSION_ID ($PRETTY_NAME). Supported: 22.04, 24.04 (Qt 6.9.3 requires Ubuntu 22.04+)."; exit 1 ;;
        esac ;;
    debian)
        case "$VERSION_ID" in
            12) exec bash "$SCRIPT_DIR/build-debian12.sh" "$@" ;;
            13) exec bash "$SCRIPT_DIR/build-debian13.sh" "$@" ;;
            *)  echo "Unsupported Debian version: $VERSION_ID ($PRETTY_NAME)"; exit 1 ;;
        esac ;;
    almalinux)
        case "$VERSION_ID" in
            8*) exec bash "$SCRIPT_DIR/build-almalinux8.sh" "$@" ;;
            9*) exec bash "$SCRIPT_DIR/build-almalinux9.sh" "$@" ;;
            10*) exec bash "$SCRIPT_DIR/build-almalinux10.sh" "$@" ;;
            *)  echo "Unsupported AlmaLinux version: $VERSION_ID ($PRETTY_NAME)"; exit 1 ;;
        esac ;;
    rocky)
        echo "RockyLinux is no longer supported. Please use AlmaLinux."
        exit 1 ;;
    *)
        echo "Unsupported distro: $PRETTY_NAME"
        exit 1 ;;
esac
