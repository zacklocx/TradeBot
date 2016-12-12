# - Try to find soci
# Once done this will define
#  SOCI_FOUND - System has soci
#  SOCI_INCLUDE_DIRS - The soci include directories
#  SOCI_LIBRARIES - The libraries needed to use soci
#  SOCI_DEFINITIONS - Compiler switches required for using soci

find_package(PkgConfig)
pkg_check_modules(PC_SOCI QUIET soci)
set(SOCI_DEFINITIONS ${PC_SOCI_CFLAGS_OTHER})

find_path(SOCI_INCLUDE_DIR soci.h
          HINTS ${PC_SOCI_INCLUDEDIR} ${PC_SOCI_INCLUDE_DIRS}
          PATH_SUFFIXES "soci")

find_library(SOCI_LIBRARY NAMES soci_core libsoci_core
             HINTS ${PC_SOCI_LIBDIR} ${PC_SOCI_LIBRARY_DIRS})

find_library(SOCI_SQLITE3_LIBRARY NAMES soci_sqlite3 libsoci_sqlite3
             HINTS ${PC_SOCI_LIBDIR} ${PC_SOCI_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set SOCI_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(soci DEFAULT_MSG
                                  SOCI_LIBRARY SOCI_INCLUDE_DIR)

find_package_handle_standard_args(soci_sqlite3 DEFAULT_MSG
                                  SOCI_SQLITE3_LIBRARY SOCI_INCLUDE_DIR)

mark_as_advanced(SOCI_INCLUDE_DIR SOCI_LIBRARY SOCI_SQLITE3_LIBRARY)

set(SOCI_LIBRARIES ${SOCI_LIBRARY} ${SOCI_SQLITE3_LIBRARY})
set(SOCI_INCLUDE_DIRS ${SOCI_INCLUDE_DIR})