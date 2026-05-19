# The squirrel format
squirrel is a neuroimaging data format to share every step of a neuroimaging analysis: experimental methods, raw data, analysis pipelines, and analyzed results. Multiple subjects, multiple sessions, multiple timepoints, clinical trial details, imaging and non-imaging data. If a user so desired, the squirrel format could be used to store the results of an entire R01 project from beginning to end in a single human-readable file. There is no miniumum amount of data needed to create a squirrel file: for example you can create a squirrel file with only an analysis pipeline and no data.

# Background
The squirrel neuroimaging data sharing format was developed as part of the open-source [Neuroinformatics Database](http://docs.neuroinfodb.org/docs). The squirrel format specification is also open-source and freely available to include in open-source and commercial projects.

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

