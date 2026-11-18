# ------------------------------------------------------------------------------
# squirrel-gui: Qt Widgets viewer for squirrel packages (view only in v1)
# ------------------------------------------------------------------------------
# Configured from ../CMakeLists.txt, which provides the squirrellib target.

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(squirrel-gui WIN32
    main.cpp
    mainwindow.cpp      mainwindow.h
    packageworker.cpp   packageworker.h
    packagesnapshot.cpp packagesnapshot.h
    startpage.cpp       startpage.h
    detailspane.cpp     detailspane.h
    propertygrid.cpp    propertygrid.h
    extractdialog.cpp   extractdialog.h
    loadingdialog.cpp   loadingdialog.h
    imageviewer.cpp     imageviewer.h
    imageview.cpp       imageview.h
    dicomslice.cpp      dicomslice.h
    theme.cpp           theme.h
    icons.cpp           icons.h
    settings.cpp        settings.h
)

# squirrellib carries Qt Core/Sql, DCMTK and bit7z with it.
target_link_libraries(squirrel-gui PRIVATE squirrellib Qt6::Widgets)

if(UNIX)
    # Same runtime search path as the CLI: DCMTK lives in /usr/local.
    set_target_properties(squirrel-gui PROPERTIES
        BUILD_RPATH "/usr/local/lib;/usr/local/lib64"
        INSTALL_RPATH "/usr/local/lib;/usr/local/lib64")
endif()
