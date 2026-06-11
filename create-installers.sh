#!/bin/bash
# Creates installers for all supported WSL distros sequentially.
# Must be run from Ubuntu WSL (requires wsl.exe).
# Run build-all.sh first to ensure bin/<distro>/ artifacts are up to date.

WINPATH=$(wslpath -w "$(pwd)")

DISTROS=(Ubuntu Debian RockyLinux-8 RockyLinux-9)
declare -A RESULTS

for DISTRO in "${DISTROS[@]}"; do
    echo ""
    echo "=========================================="
    echo "  Creating installer for $DISTRO"
    echo "=========================================="
    case "$DISTRO" in
        Ubuntu)
            if wsl.exe -d Ubuntu --cd "$WINPATH" -- bash createInstallerDebian.sh bin/ubuntu24/squirrel; then
                RESULTS[$DISTRO]="SUCCESS"
            else
                RESULTS[$DISTRO]="FAILED"
            fi ;;
        Debian)
            if wsl.exe -d Debian --cd "$WINPATH" -- bash createInstallerDebian.sh bin/debian12/squirrel; then
                RESULTS[$DISTRO]="SUCCESS"
            else
                RESULTS[$DISTRO]="FAILED"
            fi ;;
        RockyLinux-8|RockyLinux-9)
            if wsl.exe -d "$DISTRO" --cd "$WINPATH" -- bash createInstallerRHEL.sh; then
                RESULTS[$DISTRO]="SUCCESS"
            else
                RESULTS[$DISTRO]="FAILED"
            fi ;;
    esac
done

echo ""
echo "========== Installer Summary =========="
for DISTRO in "${DISTROS[@]}"; do
    printf "  %-20s %s\n" "$DISTRO" "${RESULTS[$DISTRO]}"
done
echo "======================================="
