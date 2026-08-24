#!/bin/bash
# Builds squirrel on all supported WSL distros sequentially.
# Must be run from Ubuntu 24 WSL (requires wsl.exe).
#
# Other distros access the source via /mnt/wsl/ (shared tmpfs across all
# WSL2 distros in the same VM). Ubuntu 24 builds from the real path directly
# to avoid CMake cache mismatches from prior builds.

SRCDIR="$(pwd)"
SHARED=/mnt/wsl/squirrel-build
WSL_DISTROS=(AlmaLinux-8 AlmaLinux-9 AlmaLinux-10 Ubuntu-20.04 Ubuntu-22.04 Debian)
declare -A RESULTS

echo "Mounting source at $SHARED (shared across all WSL2 distros)..."
sudo mkdir -p $SHARED
sudo mount --bind "$SRCDIR" $SHARED

# Build on the local host (Ubuntu 24) first
echo ""
echo "=========================================="
echo "  Building on Ubuntu 24 (local)"
echo "=========================================="
bash build.sh
if [ $? -eq 0 ]; then
    RESULTS[Ubuntu]="SUCCESS"
else
    RESULTS[Ubuntu]="FAILED"
fi

# Build on each WSL distro
for DISTRO in "${WSL_DISTROS[@]}"; do
    echo ""
    echo "=========================================="
    echo "  Building on $DISTRO"
    echo "=========================================="
    wsl.exe -d "$DISTRO" -- bash -c "cd $SHARED && bash build.sh"
    if [ $? -eq 0 ]; then
        RESULTS[$DISTRO]="SUCCESS"
    else
        RESULTS[$DISTRO]="FAILED"
    fi
    # AlmaLinux 8 uses gcc-toolset which may create root-owned files
    if [ "$DISTRO" = "AlmaLinux-8" ]; then
        sudo chown -R "$USER:$USER" bin/almalinux8 2>/dev/null || true
    fi
done

echo "Unmounting $SHARED..."
sudo umount $SHARED

echo ""
echo "========== Build Summary =========="
printf "  %-20s %s\n" "Ubuntu-24" "${RESULTS[Ubuntu]}"
for DISTRO in "${WSL_DISTROS[@]}"; do
    printf "  %-20s %s\n" "$DISTRO" "${RESULTS[$DISTRO]}"
done
echo "==================================="
