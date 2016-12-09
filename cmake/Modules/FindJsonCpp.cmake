# - Try to find jsoncpp
# Once done this will define
#  JSONCPP_FOUND - System has jsoncpp
#  JSONCPP_INCLUDE_DIRS - The jsoncpp include directories
#  JSONCPP_LIBRARIES - The libraries needed to use jsoncpp
#  JSONCPP_DEFINITIONS - Compiler switches required for using jsoncpp

find_package(PkgConfig)
pkg_check_modules(PC_JSONCPP QUIET jsoncpp)
set(JSONCPP_DEFINITIONS ${PC_JSONCPP_CFLAGS_OTHER})

find_path(JSONCPP_INCLUDE_DIR json/json.h
          HINTS ${PC_JSONCPP_INCLUDEDIR} ${PC_JSONCPP_INCLUDE_DIRS})

find_library(JSONCPP_LIBRARY NAMES jsoncpp libjsoncpp
             HINTS ${PC_JSONCPP_LIBDIR} ${PC_JSONCPP_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set JSONCPP_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(jsoncpp DEFAULT_MSG
                                  JSONCPP_LIBRARY JSONCPP_INCLUDE_DIR)

mark_as_advanced(JSONCPP_INCLUDE_DIR JSONCPP_LIBRARY)

set(JSONCPP_LIBRARIES ${JSONCPP_LIBRARY})
set(JSONCPP_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIR})