cmake_minimum_required(VERSION 3.20)
project(squirrel_python LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# The C++ library itself is still built with qmake (src/squirrel/squirrellib.pro).
# This file only builds the Python extension and links the prebuilt libsquirrel.a.
get_filename_component(SQUIRREL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(SQUIRREL_LIB_DIR "${SQUIRREL_ROOT}/bin/squirrel" CACHE PATH "directory containing libsquirrel.a")
set(BIT7Z_LIB_DIR    "${SQUIRREL_ROOT}/src/bit7z/lib/x64" CACHE PATH "directory containing libbit7z64.a")
set(DCMTK_PREFIX     "/usr/local"                    CACHE PATH "DCMTK install prefix")

# Qt install prefix. Appended (not set) so scikit-build-core's own prefix path,
# which locates pybind11, is preserved.
set(QT_ROOT "$ENV{HOME}/Qt/6.9.3/gcc_64" CACHE PATH "Qt 6 install prefix")
list(APPEND CMAKE_PREFIX_PATH "${QT_ROOT}")

find_package(Python 3.9 REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Sql)

pybind11_add_module(_squirrel src/bindings.cpp)

target_include_directories(_squirrel PRIVATE
    "${SQUIRREL_ROOT}/src/squirrel"
    "${SQUIRREL_ROOT}/src/bit7z/include/bit7z"
    "${DCMTK_PREFIX}/include")

# QT_NO_KEYWORDS: Qt's `slots` macro breaks Python.h (PyType_Slot *slots)
target_compile_definitions(_squirrel PRIVATE SQUIRREL_BUILD QT_NO_KEYWORDS)

# Same library set as src/squirrel/squirrel-deps.pri (linux). The group is needed
# because the static archives depend on each other.
set(DCMTK_LIBS
    dcmdata cmr dcmdsig dcmect dcmfg dcmimage dcmimgle dcmiod dcmjpeg dcmjpls
    dcmnet dcmpmap dcmpstat dcmqrdb dcmrt dcmseg dcmsr dcmtkcharls dcmtls
    dcmtract dcmwlm dcmxml i2d ijg8 ijg12 ijg16 oficonv oflog ofstd)

target_link_directories(_squirrel PRIVATE
    "${SQUIRREL_LIB_DIR}" "${BIT7Z_LIB_DIR}" "${DCMTK_PREFIX}/lib" "${DCMTK_PREFIX}/lib64")

target_link_libraries(_squirrel PRIVATE
    Qt6::Core Qt6::Sql
    -Wl,--start-group squirrel bit7z64 ${DCMTK_LIBS} -Wl,--end-group
    dl z)

install(TARGETS _squirrel LIBRARY DESTINATION squirrel)

# Qt's SQLite driver is a runtime plugin, which auditwheel cannot discover from
# DT_NEEDED entries. Ship it inside the package; squirrel/__init__.py points
# QT_PLUGIN_PATH at it. auditwheel then grafts its Qt dependencies like any other
# extension module. Turn off when building against a Qt with the driver built in.
option(SQUIRREL_BUNDLE_QT_PLUGINS "Install Qt's qsqlite plugin into the package" ON)
if(SQUIRREL_BUNDLE_QT_PLUGINS)
    get_filename_component(QT_PREFIX "${Qt6_DIR}/../../.." ABSOLUTE)
    find_file(QSQLITE_PLUGIN libqsqlite.so PATHS "${QT_PREFIX}/plugins/sqldrivers" NO_DEFAULT_PATH REQUIRED)
    install(FILES "${QSQLITE_PLUGIN}" DESTINATION squirrel/qt_plugins/sqldrivers)
endif()

# The 7-Zip library libsquirrel loads at runtime. When SQUIRREL_7ZIP_DIR points at a
# directory holding 7z.so (see packaging/build-7zip.sh), ship it and its licenses;
# squirrel/__init__.py points SQUIRREL_7ZIP_LIB at it. Otherwise libsquirrel falls
# back to a system 7-Zip.
set(SQUIRREL_7ZIP_DIR "" CACHE PATH "directory containing 7z.so and the 7-Zip license files to bundle")
if(SQUIRREL_7ZIP_DIR)
    install(FILES "${SQUIRREL_7ZIP_DIR}/7z.so" DESTINATION squirrel/lib7z)
    file(GLOB SEVENZIP_LICENSES "${SQUIRREL_7ZIP_DIR}/7zip-*.txt")
    install(FILES ${SEVENZIP_LICENSES} DESTINATION squirrel/lib7z)
endif()
