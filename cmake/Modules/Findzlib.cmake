# - Try to find zlib
# Once done this will define
#  ZLIB_FOUND - System has zlib
#  ZLIB_INCLUDE_DIRS - The zlib include directories
#  ZLIB_LIBRARIES - The libraries needed to use zlib
#  ZLIB_DEFINITIONS - Compiler switches required for using zlib

find_package(PkgConfig)
pkg_check_modules(PC_ZLIB QUIET zlib)
set(ZLIB_DEFINITIONS ${PC_ZLIB_CFLAGS_OTHER})

find_path(ZLIB_INCLUDE_DIR zlib.h
          HINTS ${PC_ZLIB_INCLUDEDIR} ${PC_ZLIB_INCLUDE_DIRS})

find_library(ZLIB_LIBRARY NAMES zlib libz
             HINTS ${PC_ZLIB_LIBDIR} ${PC_ZLIB_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ZLIB_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(zlib DEFAULT_MSG
                                  ZLIB_LIBRARY ZLIB_INCLUDE_DIR)

mark_as_advanced(ZLIB_INCLUDE_DIR ZLIB_LIBRARY)

set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})