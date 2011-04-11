# Copyright (c) 2009, Whispersoft s.r.l.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
# * Neither the name of Whispersoft s.r.l. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Finds Speex library
#
#  SPEEX_INCLUDE_DIR - where to find speex.h, etc.
#  SPEEX_LIBRARIES   - List of libraries when using Speex.
#  SPEEX_FOUND       - True if Speex found.
#

if (SPEEX_INCLUDE_DIR)
  # Already in cache, be silent
  set(SPEEX_FIND_QUIETLY TRUE)
endif (SPEEX_INCLUDE_DIR)

find_path(SPEEX_INCLUDE_DIR speex/speex.h
  /opt/local/include
  /usr/local/include
  /usr/include
)

set(SPEEX_NAMES speex)
find_library(SPEEX_LIBRARY
  NAMES ${SPEEX_NAMES}
  PATHS /usr/lib /usr/local/lib /opt/local/lib
)

if (SPEEX_INCLUDE_DIR AND SPEEX_LIBRARY)
   set(SPEEX_FOUND TRUE)
   set( SPEEX_LIBRARIES ${SPEEX_LIBRARY} )
else (SPEEX_INCLUDE_DIR AND SPEEX_LIBRARY)
   set(SPEEX_FOUND FALSE)
   set(SPEEX_LIBRARIES)
endif (SPEEX_INCLUDE_DIR AND SPEEX_LIBRARY)

if (SPEEX_FOUND)
   if (NOT SPEEX_FIND_QUIETLY)
      message(STATUS "Found Speex: ${SPEEX_LIBRARY}")
   endif (NOT SPEEX_FIND_QUIETLY)
else (SPEEX_FOUND)
   if (SPEEX_FIND_REQUIRED)
      message(STATUS "Looked for Speex libraries named ${SPEEX_NAMES}.")
      message(STATUS "Include file detected: [${SPEEX_INCLUDE_DIR}].")
      message(STATUS "Lib file detected: [${SPEEX_LIBRARY}].")
      message(FATAL_ERROR "=========> Could NOT find Speex library")
   endif (SPEEX_FIND_REQUIRED)
endif (SPEEX_FOUND)

mark_as_advanced(
  SPEEX_LIBRARY
  SPEEX_INCLUDE_DIR
  )
