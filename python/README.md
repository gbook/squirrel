# squirrel (Python)

```python
import squirrel
pkg = squirrel.load("study.sqrl")
for subj in pkg.get_subject_list():
    print(subj.id, subj.sex)
```

Build (the C++ library is built separately with qmake, see `src/squirrel/squirrellib.pro`):

```bash
python -m venv .venv && . .venv/bin/activate
pip install . --config-settings=cmake.define.QT_ROOT=$HOME/Qt/6.9.3/gcc_64
```

Thread safety: a `Squirrel` object holds a Qt SQL connection and must be used from the
thread that created it.
