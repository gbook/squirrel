# The squirrel format
squirrel is a neuroimaging data format to share every step of a neuroimaging analysis: experimental methods, raw data, analysis pipelines, and analyzed results. Multiple subjects, multiple sessions, multiple timepoints, clinical trial details, imaging and non-imaging data. If a user so desired, the squirrel format could be used to store the results of an entire R01 project from beginning to end in a single human-readable file. There is no miniumum amount of data needed to create a squirrel file: for example you can create a squirrel file with only an analysis pipeline and no data.

# Background
The squirrel neuroimaging data sharing format was developed as part of the open-source [Neuroinformatics Database](http://docs.neuroinfodb.org/docs). The squirrel format specification is also open-source and freely available to include in open-source and commercial projects.

# Installation
Pre-built packages are provided for common Linux distributions. Download the package matching your distribution, then install it with your package manager.

Releases are available here https://github.com/gbook/squirrel/releases

## RHEL / AlmaLinux / Rocky (RHEL 8, 9, 10)
Install the `.rpm` matching your major release (`el8`, `el9`, or `el10`):

```bash
# RHEL 8
sudo dnf install ./squirrel-2026.8.482-1.el8.x86_64.rpm

# RHEL 9
sudo dnf install ./squirrel-2026.8.482-1.el9.x86_64.rpm

# RHEL 10
sudo dnf install ./squirrel-2026.8.482-1.el10.x86_64.rpm
```

Dependencies (`openssl`, `p7zip`, `p7zip-plugins`) are resolved automatically.
The `p7zip` packages are in the EPEL repository, so enable it first if needed:

```bash
sudo dnf install epel-release
```

## Ubuntu / Debian
Install the `.deb` matching your distribution:

```bash
# Ubuntu 22
sudo apt install ./squirrel_2026.8.482_ubuntu22.deb

# Ubuntu 24
sudo apt install ./squirrel_2026.8.482_ubuntu24.deb

# Debian 12
sudo apt install ./squirrel_2026.8.482_debian12.deb
```

Once installed, verify with:

```bash
squirrel --version
```

# Documentation
https://docs.neuroinfodb.org/nidb/contribute/squirrel-data-sharing-format

## Man page and web documentation

The CLI reference is maintained in `doc/squirrel/squirrel.md`. This file is the single source of truth for both the man page and the web documentation page.

When CLI options change, update `squirrel.md` and regenerate the man page:

```bash
make -C doc/squirrel man
```

This requires [pandoc](https://pandoc.org). Commit both `squirrel.md` and the regenerated `squirrel.1` so the man page can be installed without pandoc as a dependency.

To preview the rendered man page locally:

```bash
make -C doc/squirrel view
```

