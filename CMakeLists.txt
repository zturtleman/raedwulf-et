cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_BUILD_TYPE Release CACHE STRING
	"Build configuration (Debug, Release, RelWithDebInfo, MinSizeRel)")

# define project and project version
project(wolfet2 C ASM)

set(wolfet2_VERSION_MAJOR  "2")
set(wolfet2_VERSION_MINOR  "60")
set(wolfet2_VERSION_PATCH  "e")

set(wolfet2_VERSION       "${wolfet2_VERSION_MAJOR}.${wolfet2_VERSION_MINOR}")
set(wolfet2_VERSION_FULL  "${wolfet2_VERSION}${wolfet2_VERSION_PATCH}")

# differentiate between windows/linux/apple for installation directories
if (WIN32 OR APPLE)
	if(WIN32) # windows (also counts for MINGW)
		SET(wolfet2_INSTALL_NAME  "")
		SET(wolfet2_SHARE_NAME    "./")
	else(WIN32) # apple
		SET(wolfet2_INSTALL_NAME  "Contents/Resources/")
		SET(wolfet2_SHARE_NAME    "wolfet2.app/")
	endif(WIN32)
else (WIN32 OR APPLE) # linux
	# TODO: this is correct for the tarball, but not for rpm/dev
	SET(wolfet2_INSTALL_NAME  "")
	SET(wolfet2_SHARE_NAME    "./")
endif (WIN32 OR APPLE)

# define general compiler configuration
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_ASM_COMPILE_OBJECT ${CMAKE_C_COMPILE_OBJECT})
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")

# Get the architecture
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i386")
if(WIN32)
	set(ARCH "x86")
else()
	set(ARCH "i386")
endif()
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

option(BUILD_MOD_PK3
	"Creates a mp_bin.pk3 file of the mod libraries" OFF)

option(USE_OPENAL
	"Use openal as the sound backend (client only)" ON)

option(USE_OPENAL_DLOPEN
	"Dynamically try to load openal at runtime (client only)" OFF)

option(USE_CURL
	"Enable auto-download support using Curl (client only)" ON)

option(USE_CODEC_VORBIS
	"Enabled vorbis OGG support (client only)" OFF)

option(USE_MUMBLE
	"Enable mumble support (client only)" ON)

option(USE_VOIP
	"Enable integrated VOIP support (client + server)" ON)

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
	"etmain/src/game/bg_misc.c"
	"etmain/src/game/bg_campaign.c"
	"etmain/src/ui/*.c")

# Load packages, set definitions
find_package(ZLIB REQUIRED)
add_definitions(-DPRODUCT_VERSION="${wolfet2_VERSION_FULL}" -DBOTLIB)

if(WIN32)
	add_definitions(-DWINVER=0x501)
	set(OS_LIBRARIES ws2_32 psapi winmm)
elseif(UNIX)
	set(OS_LIBRARIES dl m)
endif()

if(USE_CURL)
	add_definitions(-DUSE_CURL)
	find_package(CURL REQUIRED)
endif()

# Check for integrated VOIP (uses speex library)
if(USE_VOIP)
	add_definitions(-DUSE_VOIP)
	find_package(Speex REQUIRED)
	find_package(SpeexDSP REQUIRED)
endif()

if(BUILD_CLIENT)
	# GLEW
	file(GLOB_RECURSE GLEW_SRC
		"src/libs/glew/src/*.c")
	set(GLEW_INCLUDE_DIR
		"src/libs/glew/include")
	# GLFW
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildGLFW.cmake)
	
	find_package(SDL REQUIRED)
	find_package(OpenGL REQUIRED)
	find_package(JPEG REQUIRED)

	# ilm: only windows requires this
	if(WIN32)
		add_definitions(-DGLEW_STATIC)
	endif()

	if(USE_OPENAL_DLOPEN)
		add_definitions(-DUSE_OPENAL -DUSE_OPENAL_DLOPEN)

	elseif(USE_OPENAL)
		add_definitions(-DUSE_OPENAL)
		find_package(OpenAL REQUIRED)
	endif()

	if(USE_CODEC_VORBIS)
		add_definitions(-DUSE_CODEC_VORBIS)

		find_package(OggVorbis REQUIRED Ogg Vorbis Vorbisfile)
	endif()

	if(USE_MUMBLE)
		add_definitions(-DUSE_MUMBLE)
	endif()
endif()

if(BUILD_SERVER)
	add_executable(etded.${ARCH} ${OS_SRC} ${BASE_SRC} ${SHARED_SRC} ${SERVER_SRC})
	include_directories(
		${ZLIB_INCLUDE_DIRS}
		${SDL_INCLUDE_DIR}
		${CURL_INCLUDE_DIRS}
		${SPEEX_INCLUDE_DIR}
		${SPEEXDSP_INCLUDE_DIR})
	target_link_libraries(etded.${ARCH}
		${ZLIB_LIBRARIES}
		${CURL_LIBRARIES}
		${SPEEX_LIBRARIES}
		${SPEEXDSP_LIBRARY}
		${OS_LIBRARIES})
	set_target_properties(etded.${ARCH}
		PROPERTIES COMPILE_DEFINITIONS "DEDICATED")

	install(TARGETS etded.${ARCH}
	  DESTINATION ${wolfet2_SHARE_NAME}${wolfet2_INSTALL_NAME}
	)
endif()

if(BUILD_CLIENT)
	add_executable(et.${ARCH} ${OS_SRC} ${BASE_SRC} ${SHARED_SRC} ${GLFW_SRC} ${GLEW_SRC} ${CLIENT_SRC})
	include_directories(
		${ZLIB_INCLUDE_DIRS}
		${OPENGL_INCLUDE_DIR}
		${JPEG_INCLUDE_DIR}
		${OPENAL_INCLUDE_DIR}
		${CURL_INCLUDE_DIRS}
		${SPEEX_INCLUDE_DIR}
		${SPEEXDSP_INCLUDE_DIR}
		${GLEW_INCLUDE_DIR}
		${GLFW_INCLUDE_DIRS})
	target_link_libraries(et.${ARCH}
		${ZLIB_LIBRARIES}
		${SDL_LIBRARY}
		${OPENGL_gl_LIBRARY}
		${JPEG_LIBRARIES}
		${OPENAL_LIBRARY}
		${CURL_LIBRARIES}
		${SPEEX_LIBRARIES}
		${SPEEXDSP_LIBRARY}
		${OGG_LIBRARY}
		${VORBIS_LIBRARY}
		${VORBISFILE_LIBRARY}
		${OS_LIBRARIES}
		${GLFW_LIBRARIES})

	install(TARGETS et.${ARCH}
	  DESTINATION ${wolfet2_SHARE_NAME}${wolfet2_INSTALL_NAME}

	)
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

	install(TARGETS qagame.mp.${ARCH} cgame.mp.${ARCH} ui.mp.${ARCH}
	  DESTINATION ${wolfet2_SHARE_NAME}${wolfet2_INSTALL_NAME}etmain
	)

	if(BUILD_MOD_PK3)
		add_custom_target(mod_pk3 ALL DEPENDS etmain/mp_bin.pk3)
		add_custom_command(
			OUTPUT etmain/mp_bin.pk3
			COMMAND zip mp_bin.pk3 cgame.mp.${ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX} ui.mp.${ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX}
			DEPENDS cgame.mp.${ARCH} ui.mp.${ARCH}
			WORKING_DIRECTORY etmain/
		)

		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/etmain/mp_bin.pk3
			DESTINATION ${wolfet2_SHARE_NAME}${wolfet2_INSTALL_NAME}/etmain)
	endif()

	
endif()

##
# create package target
#
# Support for:
# linux: rpm, deb and tbz2
# windows: nsis installer (win32 or mingw)
# apple: TODO (cross-compiling is too hard for apple)
##

include (InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Wolfenstein: Enemy Territory")
set(CPACK_PACKAGE_VENDOR "TODO")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.txt")
set(CPACK_RESOURCE_FILE_LICENSE    "${CMAKE_SOURCE_DIR}/COPYING.txt")

set(CPACK_PACKAGE_VERSION_MAJOR "${wolfet2_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${wolfet2_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${wolfet2_VERSION_PATCH}")

set(CPACK_SOURCE_PACKAGE_FILE_NAME "wolfet2")
set(CPACK_PACKAGE_FILE_NAME     "${CPACK_SOURCE_PACKAGE_FILE_NAME}-${wolfet2_VERSION_FULL}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "wolfet2 ${wolfet2_VERSION_MAJOR}.${wolfet2_VERSION_MINOR}")
set(CPACK_PACKAGE_EXECUTABLES   "et.${ARCH}" "packaging/wolfet.ico" "etded.${ARCH}" "packaging/wolfet.ico")
set(CPACK_PACKAGE_CONTACT       "TODO:email")

## package specifics
# windows: nsis installer
if(WIN32)
	set(CPACK_NSIS_COMPRESSOR "/FINAL /SOLID lzma")
	set(CPACK_PACKAGE_INSTALL_DIRECTORY "wolfet2")
	# There is a bug in NSI that does not handle full unix paths properly. Make
	# sure there is at least one set of four (4) backlasshes.
	set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/packaging\\\\wolfet.ico")

	set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/packaging\\\\wolfet.ico")
	set(CPACK_NSIS_MUI_UNIICON "${PROJECT_SOURCE_DIR}/packaging\\\\wolfet.ico")
	set(CPACK_NSIS_INSTALLED_ICON_NAME "et.${ARCH},0")
	set(CPACK_NSIS_DISPLAY_NAME "Wolfenstein: Enemy Territory ${wolfet2_VERSION_MAJOR}.${wolfet2_VERSION_MINOR}.${wolfet2_VERSION_PATCH}")
	set(CPACK_NSIS_HELP_LINK "TODO: website")
	set(CPACK_NSIS_URL_INFO_ABOUT "TODO: website")
	set(CPACK_NSIS_CONTACT "TODO: email")
	set(CPACK_NSIS_MODIFY_PATH OFF)
	set(CPACK_STRIP_FILES "et.${ARCH}.exe" "etded.${ARCH}")

	set(CPACK_GENERATOR "NSIS")
else(WIN32)
	set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/packaging/wolfet.ico")
	set(CPACK_STRIP_FILES "et.${ARCH}" "etded.${ARCH}")
	set(CPACK_SOURCE_STRIP_FILES "")

	# linux: rpm, deb and tbz2
	if (NOT APPLE)
		set(CPACK_GENERATOR "RPM;DEB;TBZ2")
		set(CPACK_TBZ2 "on")
		set(CPACK_DEB "on")
		set(CPACK_RPM "on")

		# debian (unset values are set to default)
		set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${ARCH}")
		set(CPACK_DEBIAN_PACKAGE_SECTION      "games")
		set(CPACK_DEBIAN_PACKAGE_PRIORITY     "optional")
		set(CPACK_DEBIAN_PACKAGE_RECOMMENDS   "")
		set(CPACK_DEBIAN_PACKAGE_SUGGESTS     "")
		set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl, libsdl, libjpeg8, libspeex, libspeexdsp")

		# rpm (unset values are set to default)
		set(CPACK_RPM_PACKAGE_RELEASE "1")
		set(CPACK_RPM_PACKAGE_REQUIRES "libcurl, libsdl, libjpeg8, libspeex, libspeexdsp")
	endif (NOT APPLE)
endif(WIN32)

include (CPack)
