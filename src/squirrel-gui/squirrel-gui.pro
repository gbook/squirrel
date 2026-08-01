# Use this file to build the squirrel GUI
#
# This project LINKS libsquirrel rather than recompiling the squirrel sources,
# so the GUI and the command line utility run the exact same merge/modify/info
# code. Build squirrellib.pro first.
#
# libsquirrel is a static archive on Linux, so this project must also link every
# dependency the library was built against - that is what squirrel-deps.pri
# provides, and why -lsquirrel is added BEFORE that include (GNU ld resolves
# archives in command line order).

QT += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG -= app_bundle
CONFIG += silent

TARGET = squirrel-gui
TEMPLATE = app

SQUIRRELSRC = $$PWD/../squirrel
INCLUDEPATH += $$SQUIRRELSRC

# ----- libsquirrel -----
# SQUIRRELLIB is the directory holding libsquirrel.a / squirrel.lib, and like the
# bit7z paths in squirrel-deps.pri it is relative to the BUILD directory. The
# default assumes the layout the build-<distro>.sh scripts produce, i.e. this
# project is built in $BUILDDIR/squirrel-gui and the library in $BUILDDIR/squirrel.
# Override it from the command line if your layout differs:
#     qmake SQUIRRELLIB=/abs/path/to/libdir squirrel-gui.pro
isEmpty(SQUIRRELLIB) {
    win32: SQUIRRELLIB = ../../bin/squirrel
    else:  SQUIRRELLIB = ../squirrel
}
LIBS += -L$$SQUIRRELLIB -lsquirrel
DEPENDPATH += $$SQUIRRELLIB

# bit7z (LZMA) + DCMTK. Must follow -lsquirrel.
include($$SQUIRRELSRC/squirrel-deps.pri)

# NOTE: dcm2niix.pri is deliberately NOT included here. Those objects are already
# inside libsquirrel; including it again would compile them a second time.

*msvc* { # visual studio spec filter
    QMAKE_CXXFLAGS += -MP
}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mergeDialog.cpp \
    squirrelModel.cpp \
    squirrelWorker.cpp

HEADERS += \
    mainwindow.h \
    mergeDialog.h \
    squirrelModel.h \
    squirrelWorker.h

FORMS += \
    mainwindow.ui \
    mergeDialog.ui

RESOURCES += \
    squirrel-gui.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
