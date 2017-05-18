# - Try to find freeglut
# Once done this will define
#  FREEGLUT_FOUND - System has freeglut
#  FREEGLUT_INCLUDE_DIRS - The freeglut include directories
#  FREEGLUT_LIBRARIES - The libraries needed to use freeglut

if(APPLE)
	find_path(FREEGLUT_INCLUDE_DIR GL/freeglut.h
	          HINTS /usr/X11R6/include)

	find_library(GL_LIBRARY NAMES gl libgl
	             HINTS /usr/X11R6/lib)

	find_library(GLU_LIBRARY NAMES glu libglu
	             HINTS /usr/X11R6/lib)

	find_library(FREEGLUT_LIBRARY NAMES glut libglut
	             HINTS /usr/X11R6/lib NO_DEFAULT_PATH)
else()
	find_path(FREEGLUT_INCLUDE_DIR GL/freeglut.h
	          HINTS /usr/include)

	find_library(GL_LIBRARY NAMES GL libGL
	             HINTS /usr/lib64)

	find_library(GLU_LIBRARY NAMES GLU libGLU
	             HINTS /usr/lib64)

	find_library(FREEGLUT_LIBRARY NAMES glut libglut
	             HINTS /usr/lib64)
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set FREEGLUT_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(gl DEFAULT_MSG
                                  GL_LIBRARY FREEGLUT_INCLUDE_DIR)

find_package_handle_standard_args(glu DEFAULT_MSG
                                  GLU_LIBRARY FREEGLUT_INCLUDE_DIR)

find_package_handle_standard_args(freeglut DEFAULT_MSG
                                  FREEGLUT_LIBRARY FREEGLUT_INCLUDE_DIR)

mark_as_advanced(FREEGLUT_INCLUDE_DIR GL_LIBRARY GLU_LIBRARY FREEGLUT_LIBRARY)

set(FREEGLUT_LIBRARIES ${GL_LIBRARY} ${GLU_LIBRARY} ${FREEGLUT_LIBRARY})
set(FREEGLUT_INCLUDE_DIRS ${FREEGLUT_INCLUDE_DIR})