# ------------------------------------------------------------------------------
# replxx (interactive line editor for the 'squirrel explore' REPL)
# ------------------------------------------------------------------------------
# Compiles the vendored replxx sources (src/replxx) directly into the target,
# the same way dcm2niix.pri compiles dcm2niix - no separate CMake/build step.
# replxx provides the readline-style line editing (history, arrow keys, in-line
# editing) used by explore.cpp.
#
# Included by squirrel.pro ONLY (the interactive shell lives in the command line
# utility, not in libsquirrel or the GUI).
#
# Like dcm2niix, replxx is unmodified third-party code, so it is compiled through
# a dedicated extra-compiler that appends -w. This keeps replxx's upstream
# warnings out of the build while leaving squirrel's own warnings visible.
# ------------------------------------------------------------------------------

REPLXXDIR = $$PWD/../replxx

INCLUDEPATH += $$REPLXXDIR/include $$REPLXXDIR/src
# REPLXX_STATIC: we link replxx's objects straight into squirrel (no shared
# library), so tell its headers not to emit dllimport/dllexport decorations.
DEFINES += REPLXX_STATIC

replxx_src = \
    $$REPLXXDIR/src/ConvertUTF.cpp \
    $$REPLXXDIR/src/wcwidth.cpp \
    $$REPLXXDIR/src/conversion.cxx \
    $$REPLXXDIR/src/escape.cxx \
    $$REPLXXDIR/src/history.cxx \
    $$REPLXXDIR/src/prompt.cxx \
    $$REPLXXDIR/src/replxx.cxx \
    $$REPLXXDIR/src/replxx_impl.cxx \
    $$REPLXXDIR/src/terminal.cxx \
    $$REPLXXDIR/src/util.cxx \
    $$REPLXXDIR/src/windows.cxx

replxx.name = replxx
replxx.input = replxx_src
replxx.output = ${QMAKE_VAR_OBJECTS_DIR}replxx_${QMAKE_FILE_BASE}$$QMAKE_EXT_OBJ
# $(CXXFLAGS) carries squirrel's flags and $(DEFINES) (incl. REPLXX_STATIC);
# $(INCPATH) carries the INCLUDEPATH entries above. Trailing -w silences
# replxx's third-party warnings for these translation units only.
replxx.commands = $$QMAKE_CXX $(CXXFLAGS) $(INCPATH) -w -c ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
replxx.variable_out = OBJECTS
replxx.dependency_type = TYPE_C
QMAKE_EXTRA_COMPILERS += replxx
