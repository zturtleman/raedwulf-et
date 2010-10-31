cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_BUILD_TYPE Release CACHE STRING
	"Build configuration (Debug, Release, RelWithDebInfo, MinSizeRel)")

project(iowolfet C ASM)

set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_ASM_COMPILE_OBJECT ${CMAKE_C_COMPILE_OBJECT})

# Get the architecture
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i386")
	set(ARCH "x86")
elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "sparc" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "powerpc")
	set(ARCH ${CMAKE_SYSTEM_PROCESSOR})
else()
	set(ARCH "none")
endif()

# Get the platform
set(CONSOLE "tty")
if(WIN32)
	set(PLATFORM "win32")
	set(CONSOLE "win32")
elseif(UNIX)
	set(PLATFORM "unix")
elseif(APPLE)
	set(PLATFORM "osx")
endif()

# Options
option(BUILD_SERVER
	"Build the dedicated server executable" ON)
option(BUILD_CLIENT
	"Build the client executable" ON)
option(BUILD_MOD
	"Build the mod libraries" ON)
option(USE_CURL
	"Enable auto-download support using Curl (client only)" ON)

if(NOT BUILD_SERVER AND NOT BUILD_CLIENT)
	message(FATAL_ERROR
		"Nothing to build")
endif()

# Sources
set(OS_SRC
	"src/engine/sys/sys_main.c"
	"src/engine/sys/con_log.c"
	"src/engine/sys/sys_${PLATFORM}.c"
	"src/engine/sys/con_${CONSOLE}.c"
)

file(GLOB_RECURSE SHARED_SRC
	"src/shared/*.c")

file(GLOB_RECURSE BASE_SRC
	"src/libs/cpuinfo/*.c"
	"src/engine/qcommon/*.c"
	"src/engine/server/*.c"
	"src/engine/botlib/*.c")

file(GLOB_RECURSE SERVER_SRC
	"src/engine/null/*.c")

file(GLOB_RECURSE CLIENT_SRC
	"src/engine/glfw/*.c"
	"src/engine/sdl/sdl_snd.c"
	"src/engine/renderer/*.c"
	"src/engine/client/*.c")

file(GLOB_RECURSE GAME_SRC
	"etmain/src/botai/*.c"
	"etmain/src/game/*.c")

file(GLOB_RECURSE CGAME_SRC
	"etmain/src/game/bg_*.c"
	"etmain/src/ui/ui_shared.c"
	"etmain/src/cgame/*.c")

file(GLOB_RECURSE UI_SRC
	"etmain/src/ui/*.c")

# Load packages, set definitions
find_package(ZLIB REQUIRED)
add_definitions(-DPRODUCT_VERSION="2.60e" -DBOTLIB)

if (WIN32)
	add_definitions(-DWINVER=0x501)
	set(OS_LIBRARIES ws2_32 psapi winmm)
elseif(UNIX)
	set(OS_LIBRARIES dl m)
endif()

# GLFW
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildGLFW.cmake)

# GLEW
file(GLOB_RECURSE GLEW_SRC
	"src/libs/glew/src/*.c")
set(GLEW_INCLUDE_DIR
	"src/libs/glew/include")

if(BUILD_CLIENT)
	find_package(SDL REQUIRED)
	find_package(OpenGL REQUIRED)
	find_package(JPEG REQUIRED)

	if(USE_OPENAL_DLOPEN)
		add_definitions(-DUSE_OPENAL -DUSE_OPENAL_DLOPEN)

	elseif(USE_OPENAL)
		add_definitions(-DUSE_OPENAL)
		find_package(OpenAL REQUIRED)
	endif()
	
	if(USE_CURL)
		add_definitions(-DUSE_CURL)
		find_package(CURL REQUIRED)
	endif()
endif()

if(BUILD_SERVER)
	add_executable(etded.${ARCH} ${OS_SRC} ${BASE_SRC} ${SHARED_SRC} ${SERVER_SRC})
	include_directories(
		${ZLIB_INCLUDE_DIRS}
		${SDL_INCLUDE_DIR}
		${CURL_INCLUDE_DIRS})
	target_link_libraries(etded.${ARCH}
		${ZLIB_LIBRARIES}
		${CURL_LIBRARIES}
		${OS_LIBRARIES})
	set_target_properties(etded.${ARCH}
		PROPERTIES COMPILE_DEFINITIONS "DEDICATED")
endif()

if(BUILD_CLIENT)
	add_executable(et.${ARCH} ${OS_SRC} ${BASE_SRC} ${SHARED_SRC} ${GLEW_SRC} ${GLFW_SRC} ${CLIENT_SRC})
	include_directories(
		${GLEW_INCLUDE_DIR}
		${ZLIB_INCLUDE_DIRS}
		${OPENGL_INCLUDE_DIR}
		${JPEG_INCLUDE_DIR}
		${OPENAL_INCLUDE_DIR}
		${CURL_INCLUDE_DIRS}
		${GLFW_INCLUDE_DIRS})
	target_link_libraries(et.${ARCH}
		${ZLIB_LIBRARIES}
		${SDL_LIBRARY}
		${OPENGL_gl_LIBRARY}
		${JPEG_LIBRARIES}
		${OPENAL_LIBRARY}
		${CURL_LIBRARIES}
		${OS_LIBRARIES}
		${GLFW_LIBRARIES})
endif()

if(BUILD_MOD)
	add_library(qagame.mp.${ARCH} SHARED ${SHARED_SRC} ${GAME_SRC})
	set_target_properties(qagame.mp.${ARCH}
		PROPERTIES COMPILE_DEFINITIONS "GAMEDLL;NO_BOT_SUPPORT"
		           PREFIX ""
		           LIBRARY_OUTPUT_DIRECTORY "etmain")
	add_library(cgame.mp.${ARCH} SHARED ${SHARED_SRC} ${CGAME_SRC})
	set_target_properties(cgame.mp.${ARCH}
		PROPERTIES COMPILE_DEFINITIONS "CGAMEDLL"
		           PREFIX ""
		           LIBRARY_OUTPUT_DIRECTORY "etmain")
	add_library(ui.mp.${ARCH} SHARED ${SHARED_SRC} ${UI_SRC})
	set_target_properties(ui.mp.${ARCH}
		PROPERTIES COMPILE_DEFINITIONS "UIDLL"
		           PREFIX ""
		           LIBRARY_OUTPUT_DIRECTORY "etmain")
endif()

#set_source_files_properties(
#	src/asm/snd_mixa.s
#	src/asm/ftola.s
#	src/asm/matha.s
#	src/asm/snapvectora.s
#	PROPERTIES
#	COMPILE_FLAGS "-x assembler-with-cpp")
