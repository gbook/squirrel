QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    packagedialog.cpp \
    subjectDialog.cpp

HEADERS += \
    mainwindow.h \
    packagedialog.h \
    subjectDialog.h

FORMS += \
    mainwindow.ui \
    objectDetails.ui \
    packagedialog.ui \
    subjectDialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# GDCM library
GDCMBIN = ../../bin/gdcm
GDCMSRC = ../gdcm/Source
win32:CONFIG(release, debug|release): LIBS += -L$$GDCMBIN/bin/Release/
else:win32:CONFIG(debug, debug|release): LIBS += -L$$GDCMBIN/bin/Debug/
unix:
INCLUDEPATH += $$GDCMSRC/Attribute
INCLUDEPATH += $$GDCMSRC/Common
INCLUDEPATH += $$GDCMSRC/DataDictionary
INCLUDEPATH += $$GDCMSRC/DataStructureAndEncodingDefinition
INCLUDEPATH += $$GDCMSRC/InformationObjectDefinition
INCLUDEPATH += $$GDCMSRC/MediaStorageAndFileFormat
INCLUDEPATH += $$GDCMSRC/MessageExchangeDefinition
INCLUDEPATH += $$GDCMBIN/Source/Common # for gdcmConfigure.h
HEADERS += $$GDCMBIN/Source/Common/gdcmConfigure.h

LIBS += -lgdcmMSFF \
-lgdcmCommon \
-lgdcmDICT \
-lgdcmDSED \
-lgdcmIOD \
-lgdcmMEXD \
-lgdcmcharls \
-lgdcmexpat \
-lgdcmjpeg12 \
-lgdcmjpeg16 \
-lgdcmjpeg8 \
-lgdcmopenjp2 \
-lgdcmzlib \
-lsocketxx

# squirrel Library
SQUIRRELBIN = ../../bin/squirrel
LIBS += -L$$SQUIRRELBIN -lsquirrel
INCLUDEPATH += ../squirrel
DEPENDPATH += $$SQUIRRELBIN
*msvc* { # visual studio spec filter
    QMAKE_CXXFLAGS += -MP
}
