#!/bin/bash
# Builds .deb and .rpm installers for squirrel on all supported WSL distros.
# Mirrors build-all.sh. Must be run from Ubuntu 24 WSL (requires wsl.exe).
#
# Run build-all.sh FIRST so bin/<distro>/ artifacts exist: the .deb packages
# bundle the pre-built binaries from bin/<distro>/squirrel. The .rpm packages
# rebuild from source inside each distro via rpmbuild (squirrel.el.spec ->
# build-rpm.sh), so they do not depend on bin/ being current.
#
# As in build-all.sh, other distros access the source via /mnt/wsl/ (shared
# tmpfs across all WSL2 distros in the same VM). Ubuntu 24 works from the real
# path directly.
#
# Finished installers are collected into ./installers/ with distro-tagged names.

SRCDIR="$(pwd)"
SHARED=/mnt/wsl/squirrel-build
OUTDIR="$SRCDIR/installers"
# Force a clean Linux PATH for wsl.exe invocations (see build-all.sh for why:
# Windows PATH entries with spaces break gcc-toolset 'enable' on AlmaLinux 8).
CLEANPATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

# .deb version/package name, kept in sync by update_version.sh
# (e.g. PACKAGE=squirrel_2026.8.466 -> squirrel_2026.8.466.deb)
DEB_PACKAGE=$(grep -m1 '^PACKAGE=' createInstallerDebian.sh | cut -d= -f2)

# Ubuntu 20.04 is intentionally excluded (same reason as build-all.sh).
# Each entry: "<wsl-distro>:<type>:<binpath-or-tag>"
#   deb -> binpath is the pre-built bin/<distro>/squirrel dir
#   rpm -> field is the short dist tag used only for logging
INSTALLERS=(
    "AlmaLinux-8:rpm:el8"
    "AlmaLinux-9:rpm:el9"
    "AlmaLinux-10:rpm:el10"
    "Ubuntu-22.04:deb:bin/ubuntu22/squirrel"
    "Debian:deb:bin/debian12/squirrel"
)
declare -A RESULTS

mkdir -p "$OUTDIR"

echo "Mounting source at $SHARED (shared across all WSL2 distros)..."
sudo mkdir -p $SHARED
sudo mount --bind "$SRCDIR" $SHARED

# Collect a freshly-built .deb (created as $DEB_PACKAGE.deb in the project root)
# into OUTDIR, tagged by distro so parallel deb distros don't clobber each other.
collect_deb() {   # $1 = distro tag
    local tag="$1"
    if [ -f "$SRCDIR/$DEB_PACKAGE.deb" ]; then
        mv -v "$SRCDIR/$DEB_PACKAGE.deb" "$OUTDIR/${DEB_PACKAGE}_${tag}.deb"
        return 0
    fi
    echo "WARNING: expected $DEB_PACKAGE.deb was not produced for $tag"
    return 1
}

# --- Local Ubuntu 24 (.deb) ---
echo ""
echo "=========================================="
echo "  Ubuntu 24 (local): building .deb"
echo "=========================================="
if [ -d "bin/ubuntu24/squirrel" ]; then
    if bash createInstallerDebian.sh bin/ubuntu24/squirrel && collect_deb ubuntu24; then
        RESULTS[Ubuntu-24]="SUCCESS"
    else
        RESULTS[Ubuntu-24]="FAILED"
    fi
else
    echo "SKIP: bin/ubuntu24/squirrel not found (run build-all.sh first)"
    RESULTS[Ubuntu-24]="SKIPPED (no build)"
fi

# --- Each WSL distro ---
for ENTRY in "${INSTALLERS[@]}"; do
    IFS=':' read -r DISTRO TYPE FIELD <<< "$ENTRY"
    echo ""
    echo "=========================================="
    echo "  $DISTRO: building .$TYPE"
    echo "=========================================="

    case "$TYPE" in
        deb)
            if [ ! -d "$FIELD" ]; then
                echo "SKIP: $FIELD not found (run build-all.sh first)"
                RESULTS[$DISTRO]="SKIPPED (no build)"
                continue
            fi
            # Build the .deb inside the distro, then collect on the host side.
            wsl.exe -d "$DISTRO" -- bash -c \
                "cd $SHARED && PATH=$CLEANPATH bash createInstallerDebian.sh $FIELD"
            if [ $? -eq 0 ] && collect_deb "$DISTRO"; then
                RESULTS[$DISTRO]="SUCCESS"
            else
                RESULTS[$DISTRO]="FAILED"
            fi
            ;;
        rpm)
            # Build the .rpm inside the distro (rebuilds from source) and copy the
            # result into the shared OUTDIR. The %{?dist} tag (el8/el9/el10) keeps
            # the filenames distinct, so no manual rename is needed.
            wsl.exe -d "$DISTRO" -- bash -c \
                "cd $SHARED && PATH=$CLEANPATH bash createInstallerRHEL.sh && \
                 mkdir -p $SHARED/installers && \
                 cp -v ~/rpmbuild/RPMS/x86_64/squirrel-*.rpm $SHARED/installers/"
            if [ $? -eq 0 ]; then
                RESULTS[$DISTRO]="SUCCESS"
            else
                RESULTS[$DISTRO]="FAILED"
            fi
            ;;
    esac
done

echo "Unmounting $SHARED..."
sudo umount $SHARED

echo ""
echo "========== Installer Summary =========="
printf "  %-20s %s\n" "Ubuntu-24" "${RESULTS[Ubuntu-24]}"
for ENTRY in "${INSTALLERS[@]}"; do
    IFS=':' read -r DISTRO _ _ <<< "$ENTRY"
    printf "  %-20s %s\n" "$DISTRO" "${RESULTS[$DISTRO]}"
done
echo "======================================="
echo "Installers collected in: $OUTDIR"
ls -1 "$OUTDIR" 2>/dev/null | sed 's/^/  /'
