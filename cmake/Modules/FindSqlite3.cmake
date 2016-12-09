# - Try to find sqlite3
# Once done this will define
#  SQLITE3_FOUND - System has sqlite3
#  SQLITE3_INCLUDE_DIRS - The sqlite3 include directories
#  SQLITE3_LIBRARIES - The libraries needed to use sqlite3
#  SQLITE3_DEFINITIONS - Compiler switches required for using sqlite3

find_package(PkgConfig)
pkg_check_modules(PC_SQLITE3 QUIET sqlite3)
set(SQLITE3_DEFINITIONS ${PC_SQLITE3_CFLAGS_OTHER})

find_path(SQLITE3_INCLUDE_DIR sqlite3.h
          HINTS ${PC_SQLITE3_INCLUDEDIR} ${PC_SQLITE3_INCLUDE_DIRS})

find_library(SQLITE3_LIBRARY NAMES sqlite3 libsqlite3
             HINTS ${PC_SQLITE3_LIBDIR} ${PC_SQLITE3_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set SQLITE3_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(sqlite3 DEFAULT_MSG
                                  SQLITE3_LIBRARY SQLITE3_INCLUDE_DIR)

mark_as_advanced(SQLITE3_INCLUDE_DIR SQLITE3_LIBRARY)

set(SQLITE3_LIBRARIES ${SQLITE3_LIBRARY})
set(SQLITE3_INCLUDE_DIRS ${SQLITE3_INCLUDE_DIR})