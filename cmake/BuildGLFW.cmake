# Build dependencies
set(GLFW_SOURCE_DIR "src/libs/glfw")
set(GLFW_BINARY_DIR "src/libs/glfw")

# Build GLFW 3.x
set(GLFW_VERSION_MAJOR "3")
set(GLFW_VERSION_MINOR "0")
set(GLFW_VERSION_PATCH "0")
set(GLFW_VERSION_EXTRA "")
set(GLFW_VERSION "${GLFW_VERSION_MAJOR}.${GLFW_VERSION_MINOR}")
set(GLFW_VERSION_FULL  
    "${GLFW_VERSION}.${GLFW_VERSION_PATCH}${GLFW_VERSION_EXTRA}")

include(CheckFunctionExists)
include(CheckSymbolExists)

find_package(OpenGL REQUIRED)

file(GLOB common_SOURCES
	"${GLFW_SOURCE_DIR}/src/*.c")

#--------------------------------------------------------------------
# Set up GLFW for Win32 and WGL on Windows
#--------------------------------------------------------------------
if (WIN32)
	message(STATUS "Building GLFW for WGL on a Win32 system") 

	# Set up library and include paths
	set(CMAKE_REQUIRED_LIBRARIES ${OPENGL_gl_LIBRARY})
	list(APPEND GLFW_INCLUDE_DIR ${OPENGL_INCLUDE_DIR})
	list(APPEND GLFW_LIBRARIES ${OPENGL_gl_LIBRARY})

	# Select platform specific code
	set(GLFW_PLATFORM_DIR "${GLFW_SOURCE_DIR}/src/win32")
endif (WIN32)

#--------------------------------------------------------------------
# Set up GLFW for Xlib and GLX on Unix-like systems with X Windows
#--------------------------------------------------------------------
if (UNIX AND NOT APPLE AND NOT CYGWIN)
	message(STATUS "Building GLFW for X11 and GLX on a Unix-like system")
	
	# Set up library and include paths
	set(CMAKE_REQUIRED_LIBRARIES ${X11_X11_LIB} ${OPENGL_gl_LIBRARY})
	list(APPEND GLFW_INCLUDE_DIR ${X11_X11_INCLUDE_PATH})
	list(APPEND GLFW_LIBRARIES ${X11_X11_LIB})
	list(APPEND GLFW_INCLUDE_DIR ${OPENGL_INCLUDE_DIR})
	list(APPEND GLFW_LIBRARIES ${OPENGL_gl_LIBRARY})

	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckX11Extensions.cmake)

	# Check for XRandR (modern resolution switching extension)
	CHECK_X11_XRANDR()
	if (X11_XRANDR_FOUND)
		set(_GLFW_HAS_XRANDR 1) 
		list(APPEND GLFW_INCLUDE_DIR ${X11_XRANDR_INCLUDE_DIR})
		list(APPEND GLFW_LIBRARIES ${X11_XRANDR_LIBRARIES})
	endif(X11_XRANDR_FOUND)

	# Check for Xf86VidMode (fallback legacy resolution switching extension)
	CHECK_X11_XF86VIDMODE()
	if (X11_XF86VIDMODE_FOUND)
		set(_GLFW_HAS_XF86VIDMODE 1)
		list(APPEND GLFW_INCLUDE_DIR ${X11_XF86VIDMODE_INCLUDE_DIR})
		list(APPEND GLFW_LIBRARIES ${X11_XF86VIDMODE_LIBRARIES})
	endif(X11_XF86VIDMODE_FOUND) 

	CHECK_FUNCTION_EXISTS(glXGetProcAddress _GLFW_HAS_GLXGETPROCADDRESS)

	if (NOT _GLFW_HAS_GLXGETPROCADDRESS)
		CHECK_FUNCTION_EXISTS(glXGetProcAddressARB _GLFW_HAS_GLXGETPROCADDRESSARB)
	endif (NOT _GLFW_HAS_GLXGETPROCADDRESS)

	if (NOT _GLFW_HAS_GLXGETPROCADDRESS AND NOT _GLFW_HAS_GLXGETPROCADDRESSARB)
		CHECK_FUNCTION_EXISTS(glXGetProcAddressEXT _GLFW_HAS_GLXGETPROCADDRESSEXT)
	endif (NOT _GLFW_HAS_GLXGETPROCADDRESS AND NOT _GLFW_HAS_GLXGETPROCADDRESSARB)

	if (NOT _GLFW_HAS_GLXGETPROCADDRESS AND
	    NOT _GLFW_HAS_GLXGETPROCADDRESSARB AND
	    NOT _GLFW_HAS_GLXGETPROCADDRESSEXT)
	    message(WARNING "No glXGetProcAddressXXX variant found")
	endif (NOT _GLFW_HAS_GLXGETPROCADDRESS AND
	       NOT _GLFW_HAS_GLXGETPROCADDRESSARB AND
	       NOT _GLFW_HAS_GLXGETPROCADDRESSEXT)

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		set(_GLFW_USE_LINUX_JOYSTICKS 1)
	endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

	# Select platform specific code
	set(GLFW_PLATFORM_DIR "${GLFW_SOURCE_DIR}/src/x11")
endif(UNIX AND NOT APPLE AND NOT CYGWIN)

#--------------------------------------------------------------------
# Set up GLFW for Cocoa and NSOpenGL on Mac OS X
#--------------------------------------------------------------------
if (UNIX AND APPLE)
	message(STATUS "Building GLFW for Cocoa and NSOpenGL on Mac OS X")
	
	# Universal build, decent set of warning flags...
	set(CMAKE_OSX_ARCHITECTURES ppc;i386;ppc64;x86_64)
	set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.5.sdk)
	set(CMAKE_C_FLAGS "-mmacosx-version-min=10.5 -Wall -Wextra -Wno-unused-parameter -Werror") 

	# Set up library and include paths
	find_library(COCOA_FRAMEWORK Cocoa)
	list(APPEND GLFW_LIBRARIES ${COCOA_FRAMEWORK})
	list(APPEND GLFW_LIBRARIES ${OPENGL_gl_LIBRARY})

	# Select platform specific code
	set(GLFW_PLATFORM_DIR "${GLFW_SOURCE_DIR}/src/cocoa")
endif(UNIX AND APPLE)

file(GLOB platform_SOURCES
	"${GLFW_PLATFORM_DIR}/*.c")
	
#--------------------------------------------------------------------
# Create shared configuration header
#--------------------------------------------------------------------
configure_file(${GLFW_SOURCE_DIR}/src/config.h.in
               ${GLFW_BINARY_DIR}/src/config.h @ONLY)

set(GLFW_SRC ${common_SOURCES} ${platform_SOURCES})
set(GLFW_INCLUDE_DIRS "${GLFW_SOURCE_DIR}/include"
                      "${GLFW_SOURCE_DIR}/src"
                      "${CMAKE_CURRENT_BINARY_DIR}/${GLFW_BINARY_DIR}/src"
                      "${GLFW_PLATFORM_DIR}")

