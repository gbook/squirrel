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
