# Locate OpenAL
# This module defines
# GLEW_LIBRARY
# GLEW_FOUND, if false, do not try to link to OpenAL 
# GLEW_INCLUDE_DIR, where to find the headers
#
# $GLEWDIR is an environment variable that would
# correspond to the ./configure --prefix=$GLEWDIR
# used in building OpenAL.
#
# Created by Tai Chi Minh Ralph Eastwood. This was influenced by the FindOpenAL.cmake module.

FIND_PATH(GLEW_INCLUDE_DIR glew.h
  HINTS
  $ENV{GLEWDIR}
  PATH_SUFFIXES GL include/GL include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

FIND_LIBRARY(GLEW_LIBRARY 
  NAMES glew
  HINTS
  $ENV{GLEWDIR}
  PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)


# handle the QUIETLY and REQUIRED arguments and set GLEW_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenAL  DEFAULT_MSG  GLEW_LIBRARY GLEW_INCLUDE_DIR)

MARK_AS_ADVANCED(GLEW_LIBRARY GLEW_INCLUDE_DIR)
