###############################################################################
# CMake module to search for SOCI library
#
# WARNING: This module is experimental work in progress.
#
# This module defines:
#  SOCI_INCLUDE_DIRS        = include dirs to be used when using the soci library
#  SOCI_LIBRARIES           = full path to the soci library
#  SOCI_LIBRARIES_DIR       = full path to directory containing soci library
#  SOCI_VERSION             = the soci version found (not yet. soci does not provide that info.)
#  SOCI_FOUND               = true if soci was found
#
# This module respects:
#  LIB_SUFFIX         = (64|32|"") Specifies the suffix for the lib directory
#
# For each component you specify in find_package(), the following variables are set.
#
#  SOCI_${COMPONENT}_PLUGIN = full path to the soci plugin
#  SOCI_${COMPONENT}_FOUND
#
# Copyright (c) 2011 Michael Jansen <info@michael-jansen.biz>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
###############################################################################
#
### Global Configuration Section
#
SET(_SOCI_ALL_PLUGINS    mysql odbc postgresql sqlite3)
SET(_SOCI_REQUIRED_VARS  SOCI_INCLUDE_DIR SOCI_LIBRARIES)

#
### FIRST STEP: Find the soci headers.
#
FIND_PATH(
    SOCI_INCLUDE_DIR soci.h
    PATHS "/usr/local"
    PATH_SUFFIXES "soci"
    DOC "soci (http://soci.sourceforge.net) include directory")
MARK_AS_ADVANCED(SOCI_INCLUDE_DIR)

SET(SOCI_INCLUDE_DIRS ${SOCI_INCLUDE_DIR})

#
### SECOND STEP: Find the soci core library. Respect LIB_SUFFIX
#
if(DEFINED LIB_SUFFIX)
    set(LIB_SUFFIXES lib${LIB_SUFFIX})
else()
    set(LIB_SUFFIXES lib lib32 lib64)
endif()

FIND_LIBRARY(
    SOCI_LIBRARIES
    NAMES soci_core
    HINTS ${SOCI_INCLUDE_DIR}/..
    PATH_SUFFIXES ${LIB_SUFFIXES})
MARK_AS_ADVANCED(SOCI_LIBRARIES)

GET_FILENAME_COMPONENT(SOCI_LIBRARIES_DIR ${SOCI_LIBRARIES} PATH)
MARK_AS_ADVANCED(SOCI_LIBRARIES_DIR)

#
### ADHERE TO STANDARDS
#
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(soci DEFAULT_MSG ${_SOCI_REQUIRED_VARS})

#
### THIRD STEP: Find all installed plugins if the library was found
#
IF(SOCI_INCLUDE_DIR AND SOCI_LIBRARIES)

    FOREACH(plugin IN LISTS _SOCI_ALL_PLUGINS)

        FIND_LIBRARY(
            SOCI_${plugin}_PLUGIN
            NAMES soci_${plugin}
            HINTS ${SOCI_INCLUDE_DIR}/..
            PATH_SUFFIXES ${LIB_SUFFIXES})
        MARK_AS_ADVANCED(SOCI_${plugin}_PLUGIN)

        IF(SOCI_${plugin}_PLUGIN)
            SET(SOCI_${plugin}_FOUND True)
            SET(SOCI_LIBRARIES ${SOCI_LIBRARIES} ${SOCI_${plugin}_PLUGIN})
            message("--   Found soci ${plugin} plugin")
        ELSE()
            SET(SOCI_${plugin}_FOUND False)
        ENDIF()

    ENDFOREACH()
ENDIF()