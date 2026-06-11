#!/bin/bash
# Builds squirrel on all supported WSL distros sequentially.
# Must be run from Ubuntu WSL (requires wsl.exe).
#
# Other distros access the source via /mnt/wsl/ (shared tmpfs across all
# WSL2 distros in the same VM). Ubuntu builds from the real path directly
# to avoid CMake cache mismatches from prior builds.

SRCDIR="$(pwd)"
SHARED=/mnt/wsl/squirrel-build
DISTROS=(Ubuntu Debian RockyLinux-8 RockyLinux-9)
declare -A RESULTS

echo "Mounting source at $SHARED (shared across all WSL2 distros)..."
sudo mkdir -p $SHARED
sudo mount --bind "$SRCDIR" $SHARED

for DISTRO in "${DISTROS[@]}"; do
    echo ""
    echo "=========================================="
    echo "  Building on $DISTRO"
    echo "=========================================="
    if [ "$DISTRO" = "Ubuntu" ]; then
        bash build.sh
        RESULT=$?
    else
        wsl.exe -d "$DISTRO" -- bash -c "cd $SHARED && bash build.sh"
        RESULT=$?
    fi
    if [ $RESULT -eq 0 ]; then
        RESULTS[$DISTRO]="SUCCESS"
    else
        RESULTS[$DISTRO]="FAILED"
    fi
    if [ "$DISTRO" = "RockyLinux-8" ]; then
        sudo chown -R "$USER:$USER" bin/rockylinux8 2>/dev/null || true
    fi
done

echo "Unmounting $SHARED..."
sudo umount $SHARED

echo ""
echo "========== Build Summary =========="
for DISTRO in "${DISTROS[@]}"; do
    printf "  %-20s %s\n" "$DISTRO" "${RESULTS[$DISTRO]}"
done
echo "==================================="
