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
# The Python wheels (manylinux, built in a container) are collected there too.
# Each target's full build output is also written to ./installers/logs/
# (e.g. squirrel_2026.11.515_AlmaLinux-8.log), along with a summary file, so a
# failure in the summary can be traced back to its log.

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
declare -A LOGS

LOGDIR="$OUTDIR/logs"
mkdir -p "$OUTDIR" "$LOGDIR"

# Log file for a target, versioned so earlier runs are kept.
logfile() {   # $1 = target name
    echo "$LOGDIR/${DEB_PACKAGE}_$1.log"
}

# Start a fresh log for a target with a header, and remember its path.
start_log() {   # $1 = target name
    local log
    log=$(logfile "$1")
    LOGS[$1]="$log"
    {
        echo "# $1 installer build"
        echo "# Started: $(date '+%Y-%m-%d %H:%M:%S')"
        echo ""
    } > "$log"
}

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
start_log Ubuntu-24
if [ -d "bin/ubuntu24/squirrel" ]; then
    { bash createInstallerDebian.sh bin/ubuntu24/squirrel && collect_deb ubuntu24; } 2>&1 | tee -a "${LOGS[Ubuntu-24]}"
    if [ "${PIPESTATUS[0]}" -eq 0 ]; then
        RESULTS[Ubuntu-24]="SUCCESS"
    else
        RESULTS[Ubuntu-24]="FAILED"
    fi
else
    echo "SKIP: bin/ubuntu24/squirrel not found (run build-all.sh first)" | tee -a "${LOGS[Ubuntu-24]}"
    RESULTS[Ubuntu-24]="SKIPPED (no build)"
fi

# --- Each WSL distro ---
for ENTRY in "${INSTALLERS[@]}"; do
    IFS=':' read -r DISTRO TYPE FIELD <<< "$ENTRY"
    echo ""
    echo "=========================================="
    echo "  $DISTRO: building .$TYPE"
    echo "=========================================="
    start_log "$DISTRO"
    LOG="${LOGS[$DISTRO]}"

    case "$TYPE" in
        deb)
            if [ ! -d "$FIELD" ]; then
                echo "SKIP: $FIELD not found (run build-all.sh first)" | tee -a "$LOG"
                RESULTS[$DISTRO]="SKIPPED (no build)"
                continue
            fi
            # Build the .deb inside the distro, then collect on the host side.
            {
                wsl.exe -d "$DISTRO" -- bash -c \
                    "cd $SHARED && PATH=$CLEANPATH bash createInstallerDebian.sh $FIELD" \
                    && collect_deb "$DISTRO"
            } 2>&1 | tee -a "$LOG"
            if [ "${PIPESTATUS[0]}" -eq 0 ]; then
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
                 cp -v ~/rpmbuild/RPMS/x86_64/squirrel-*.rpm $SHARED/installers/" \
                2>&1 | tee -a "$LOG"
            if [ "${PIPESTATUS[0]}" -eq 0 ]; then
                RESULTS[$DISTRO]="SUCCESS"
            else
                RESULTS[$DISTRO]="FAILED"
            fi
            ;;
    esac
done

echo "Unmounting $SHARED..."
sudo umount $SHARED

# --- Python wheels ---
# manylinux wheels are built in a container (python/packaging/build-wheels.sh), so they
# do not depend on the WSL distros above or on bin/. Needs Docker or Podman; set
# SKIP_WHEELS=1 to leave them out.
echo ""
echo "=========================================="
echo "  Python wheels (manylinux container)"
echo "=========================================="
start_log Python-wheels
if [ "$SKIP_WHEELS" = "1" ]; then
    RESULTS[Python-wheels]="SKIPPED (SKIP_WHEELS=1)"
elif ! command -v docker >/dev/null 2>&1 && ! command -v podman >/dev/null 2>&1; then
    echo "SKIP: neither docker nor podman found" | tee -a "${LOGS[Python-wheels]}"
    RESULTS[Python-wheels]="SKIPPED (no docker/podman)"
else
    rm -rf python/wheelhouse
    { bash python/packaging/build-wheels.sh && cp -v python/wheelhouse/*.whl "$OUTDIR/"; } \
        2>&1 | tee -a "${LOGS[Python-wheels]}"
    if [ "${PIPESTATUS[0]}" -eq 0 ]; then
        RESULTS[Python-wheels]="SUCCESS"
    else
        RESULTS[Python-wheels]="FAILED"
    fi
fi

# Append the result to each log, then print the summary to the console and to
# a summary file next to the logs. Non-successful targets show their log path.
SUMMARY="$LOGDIR/${DEB_PACKAGE}_summary.txt"
TARGETS=(Ubuntu-24)
for ENTRY in "${INSTALLERS[@]}"; do
    IFS=':' read -r DISTRO _ _ <<< "$ENTRY"
    TARGETS+=("$DISTRO")
done
TARGETS+=(Python-wheels)

for T in "${TARGETS[@]}"; do
    if [ -n "${LOGS[$T]}" ]; then
        {
            echo ""
            echo "# Finished: $(date '+%Y-%m-%d %H:%M:%S')"
            echo "# Result: ${RESULTS[$T]}"
        } >> "${LOGS[$T]}"
    fi
done

{
    echo ""
    echo "========== Installer Summary =========="
    echo "  Run finished: $(date '+%Y-%m-%d %H:%M:%S')"
    for T in "${TARGETS[@]}"; do
        printf "  %-20s %s\n" "$T" "${RESULTS[$T]}"
        if [ "${RESULTS[$T]}" != "SUCCESS" ] && [ -n "${LOGS[$T]}" ]; then
            printf "  %-20s   log: %s\n" "" "${LOGS[$T]}"
        fi
    done
    echo "======================================="
} | tee "$SUMMARY"
echo "Logs and summary written to: $LOGDIR"
echo "Installers collected in: $OUTDIR"
ls -1 "$OUTDIR" 2>/dev/null | sed 's/^/  /'
