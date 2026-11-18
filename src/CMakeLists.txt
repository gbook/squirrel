# ------------------------------------------------------------------------------
# squirrel - top-level CMake project
# ------------------------------------------------------------------------------
# Builds, in one configure/build:
#   bit7z      (src/bit7z)    archive library used by libsquirrel (LZMA/7z)
#   ftxui      (src/ftxui)    terminal UI for 'squirrel explore' (CLI only)
#   squirrellib               libsquirrel static library   -> <build>/squirrel/libsquirrel.a
#   squirrel                  command line utility         -> <build>/squirrel/squirrel
#
# Typical use (Linux):
#   cmake -S src -B bin/ubuntu24 -DCMAKE_PREFIX_PATH=~/Qt/6.9.3/gcc_64
#   cmake --build bin/ubuntu24 -j 16
#
# Qt Creator: File > Open File or Project > src/CMakeLists.txt
#
# Options:
#   -DSQUIRREL_BUILD_CLI=OFF            build only libsquirrel (no FTXUI, no CLI)
#   -DBIT7Z_USE_LEGACY_IUNKNOWN=ON|OFF  must match the 7-Zip on the target system:
#                                       ON  = legacy p7zip 16.02 ABI
#                                       OFF = modern 7-Zip (23.01+), e.g. Ubuntu 24,
#                                             Debian 12/13
#   DCMTK is located with find_package(DCMTK CONFIG); on Linux it is looked for
#   in /usr/local by default, otherwise add its prefix to CMAKE_PREFIX_PATH.
# ------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16)

project(squirrel LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# libsquirrel.a is also linked into the Python extension module (a shared
# object), so everything must be position independent.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(SQUIRREL_BUILD_CLI "Build the squirrel command line utility (and FTXUI)" ON)

# ----- bit7z ------------------------------------------------------------------
# Defaults for bit7z's own options. Anything passed with -D on the command line
# takes precedence, because these only set the cache if it is still empty.
set(BIT7Z_AUTO_FORMAT ON CACHE BOOL "bit7z: detect archive format automatically")
set(BIT7Z_USE_LEGACY_IUNKNOWN ON CACHE BOOL "bit7z: legacy p7zip IUnknown ABI (OFF for modern 7-Zip)")
# bit7z turns verbose makefiles on in the cache; keep normal output.
set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "")
add_subdirectory(bit7z)
# bit7z (cmake/OutputOptions.cmake) writes its archive into its SOURCE tree
# (src/bit7z/lib/x64). build-all.sh shares the source tree across distros, so
# that would let one distro link another distro's libbit7z64.a. Keep it in this
# build directory instead.
set_target_properties(bit7z PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bit7z")

# ----- FTXUI (CLI only) -------------------------------------------------------
if(SQUIRREL_BUILD_CLI)
    set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "")
    set(FTXUI_BUILD_TESTS    OFF CACHE BOOL "")
    set(FTXUI_BUILD_DOCS     OFF CACHE BOOL "")
    set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "")
    set(FTXUI_QUIET          ON  CACHE BOOL "")
    add_subdirectory(ftxui)
endif()

# ----- squirrel library + CLI -------------------------------------------------
add_subdirectory(squirrel)
