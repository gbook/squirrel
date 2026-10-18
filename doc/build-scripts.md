# Squirrel Build & Packaging Scripts

This document describes every `*.sh` script in the project root: what it does,
and whether it is meant to be run **directly by you** or is an **internal
helper** called only by other scripts.

All scripts assume they are run from the squirrel project root. The batch
orchestrators (`build-all.sh`, `makeAllInstallers.sh`) must be run from the
Ubuntu 24 host distro, because they drive the other WSL distros via `wsl.exe`.

---

## Top-level scripts (run these directly)

| Script | Purpose |
|--------|---------|
| `build.sh` | **The everyday local build** — run this to compile and test your changes on the distro you are currently in. It detects the distro (via `/etc/os-release`) and delegates to the matching `build-<distro>.sh`. Also the entry point `build-all.sh` calls on each distro. |
| `build-all.sh` | Builds the squirrel library + utilities on **every supported WSL distro** (AlmaLinux 8/9/10, Ubuntu 22.04, Debian, plus local Ubuntu 24). Bind-mounts the source at `/mnt/wsl/squirrel-build` so all distros share one tree, then delegates to `build.sh` inside each. Outputs to `bin/<distro>/`. |
| `makeAllInstallers.sh` | Builds the `.deb` and `.rpm` **installers** for every supported WSL distro. Debs bundle the pre-built `bin/<distro>/` binaries; rpms rebuild from source. Collects results into `installers/`. **Run `build-all.sh` first** so the deb artifacts exist. |
| `update_version.sh` | Recomputes the version string (`YEAR.MONTH.<git-commit-count>`) and writes it into `squirrelVersion.h`, `squirrel.el.spec`, and `createInstallerDebian.sh`. Run before cutting a release build. |
| `createInstallerDebian.sh` | Builds a single `.deb` from a pre-built binary dir (arg: path to `bin/<distro>/squirrel`, default `bin/squirrel`). Usable standalone for a one-off deb. |
| `createInstallerRHEL.sh` | Builds a single `.rpm` from local source via `rpmbuild` + `squirrel.el.spec`. Usable standalone for a one-off rpm on a RHEL-family distro. |
| `createInstallerRHEL-legacy.sh` | Legacy standalone rpm build that **downloads `main.zip` from GitHub** instead of using local source. Kept for reference; prefer `createInstallerRHEL.sh`. |

> `create-installers.sh` is **dead** (superseded by `makeAllInstallers.sh`) and is
> intentionally omitted here.

---

## Internal helpers (not called directly)

These are invoked by the scripts above, not by you.

- **`build-<distro>.sh`** — the nine per-distro build scripts. Each performs the
  actual compile (bit7z → squirrel library → squirrel utilities → optional GUI)
  for one distro and writes to `bin/<distro>/`. Called by `build.sh`.
- **`build-rpm.sh`** — the compile step used *inside* the rpm build. It is
  referenced by the `%build` section of `squirrel.el.spec`, which `rpmbuild`
  runs when `createInstallerRHEL.sh` executes.

### Call tree

```
build-all.sh                         (build everything, all distros)
└── build.sh                         (dispatch by /etc/os-release)
    ├── build-ubuntu22.sh
    ├── build-ubuntu24.sh
    ├── build-debian12.sh
    ├── build-debian13.sh
    ├── build-almalinux8.sh
    ├── build-almalinux9.sh
    └── build-almalinux10.sh

makeAllInstallers.sh                 (build all installers, all distros)
├── createInstallerDebian.sh         (.deb, per deb distro)
└── createInstallerRHEL.sh           (.rpm, per RHEL distro)
    └── rpmbuild  ─ squirrel.el.spec %build
        └── build-rpm.sh             (compile step, internal to rpm build)

update_version.sh                    (standalone: bump version strings)
createInstallerRHEL-legacy.sh        (standalone: rpm from GitHub zip)
```

Legend: indentation = "is called by the script above it". Everything under
`build.sh` and `build-rpm.sh` is an **internal helper**; the roots of each
block are the scripts you run directly.
