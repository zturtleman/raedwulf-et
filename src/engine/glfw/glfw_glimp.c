/*
===========================================================================
Copyright (C) 2010 Tai Chi Minh Ralph Eastwood
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of the Open Territory source code.

Open Territory source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Open Territory source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Open Territory source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../renderer/tr_local.h"
#include "../client/client.h"
#include "../sys/sys_local.h"

#include "glfw_local.h"

typedef enum
{
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
} rserr_t;

cvar_t *r_allowSoftwareGL; // Don't abort out if a hardware visual can't be obtained

/* video modes list */
#define MAX_VIDEO_MODES 128
GLFWvidmode videoModes[MAX_VIDEO_MODES];
GLFWwindow glfwWindow;

/**
 * GLimp_Shutdown
 */
void GLimp_Shutdown(void)
{
	float oldDisplayAspect = displayAspect;
	glfwTerminate();

	Com_Memset(&glConfig, 0, sizeof(glConfig));
	displayAspect = oldDisplayAspect;
	Com_Memset(&glState, 0, sizeof(glState));
}

/**
 * GLimp_Shutdown
 */
void GLimp_LogComment(char *comment)
{
}

/**
 * GLimp_DetectAvailableModes
 */
static void GLimp_DetectAvailableModes(void)
{
	int numModes;
	char buf[MAX_STRING_CHARS];
	int i;
	int lastWidth = 0, lastHeight = 0;
	int len = MAX_STRING_CHARS - 1;
	char *p = buf;
	
	numModes = glfwGetVideoModes(videoModes, MAX_VIDEO_MODES);
	
	if (!numModes)
	{
		ri.Printf(PRINT_WARNING, "Can't get list of available modes\n");
		return;
	}
	
	for (i = 0; i < numModes; i++)
	{
		if (lastWidth != videoModes[i].width ||
			lastHeight != videoModes[i].height)
		{
			int modeslen;
			Com_sprintf(p, len, "%ux%u ", videoModes[i].width, 
				videoModes[i].height);
			modeslen = strlen(p);
			len -= modeslen;
			p += modeslen;
		}
	}
	
	ri.Printf(PRINT_DEVELOPER, "Available modes: '%s'\n", buf);
	ri.Cvar_Set("r_availableModes", buf);
}


/**
 * GLimp_SetMode
 */
static int GLimp_SetMode(int mode, qboolean fullscreen)
{
	const char* glstring;
	int rbits, gbits, bbits, abits, cbits, dbits, sbits;
	int width, height;
	int once;
	static GLFWvidmode dvidmode;
	static int have_dvidmode;

	ri.Printf(PRINT_DEVELOPER, "Initializing OpenGL display\n");

	/* first time, resolve desktop resolution */
	if (!have_dvidmode)
	{
		glfwGetDesktopMode(&dvidmode);
		have_dvidmode = qtrue;
	}
	/* default values are desktop values or cvar */
	if (!R_GetModeInfo(&width, &height, &glConfig.windowAspect, mode))
	{
		ri.Printf(PRINT_ALL, " invalid mode\n");
		return RSERR_INVALID_MODE;
	}
    
	//width = r_width->integer ? r_width->integer : dvidmode.Width;
	//height = r_height->integer ? r_height->integer : dvidmode.Height;
	cbits = r_colorbits->integer ? r_colorbits->integer :
		dvidmode.redBits + dvidmode.blueBits + dvidmode.greenBits;
	ri.Printf(PRINT_DEVELOPER, "Attempting window size: %dx%d %d\n", 
		width, height, cbits);

	displayAspect = (float)width / (float)height;
	ri.Printf(PRINT_DEVELOPER, "Estimated display aspect: %.3f\n", displayAspect);
	
	glConfig.isFullscreen = fullscreen;

	dbits = r_depthbits->integer;
	sbits = r_stencilbits->integer;
	
    // Do the bit calculation
	if (cbits < 1 || cbits < 32 || !(cbits % 8))
		cbits = 32;
	if (dbits < 1 || dbits < 24 || !(dbits % 8))
		dbits = 24;
	if (sbits < 1 || sbits < 16 || !(sbits % 8))
		sbits = 8;
	
	abits = 0;
	switch (cbits)
	{
		case 16:
			rbits = 5;
			gbits = 6;
			bbits = 5;
			break;
		case 32:
			abits = 8;
		case 24:
		default:
			rbits = 8;
			gbits = 8;
			bbits = 8;
			break;
	}

	// TODO: glfw already fallsback, do we need a fallback at all!
	once = 0;
	do
	{
		glfwOpenWindowHint(GLFW_ACCUM_RED_BITS, rbits);
		glfwOpenWindowHint(GLFW_ACCUM_GREEN_BITS, gbits);
		glfwOpenWindowHint(GLFW_ACCUM_BLUE_BITS, bbits);
		glfwOpenWindowHint(GLFW_ACCUM_ALPHA_BITS, abits);
		glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	
		ri.Printf(PRINT_DEVELOPER, "Using %d/%d/%d color, "
			"%d depth, %d stencil bits\n",
			rbits, gbits, bbits, dbits, sbits);

		glfwWindow = glfwOpenWindow(width, height,
			fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOWED,
			CLIENT_WINDOW_TITLE, NULL);
		
		if (glfwWindow)
		{
			glfwSwapInterval(r_swapInterval->integer);
			glfwDisable(glfwWindow, GLFW_MOUSE_CURSOR);
		}
		else
		{
			if (once)
				break;
		
			/* fallback onto desktop resolution */
			width = dvidmode.width;
			height = dvidmode.height;
			rbits = dvidmode.redBits;
			gbits = dvidmode.greenBits;
			bbits = dvidmode.blueBits;
			abits = 0;
			sbits = 8;
			dbits = 8;
			cbits = rbits + gbits + bbits + abits;
			once = 1;
		}
	}
	while (!glfwWindow);
			
	/* set the cvars to the actual values */
	if (glfwWindow)
	{
		rbits = glfwGetWindowParam(glfwWindow, GLFW_RED_BITS);
		gbits = glfwGetWindowParam(glfwWindow, GLFW_GREEN_BITS);
		bbits = glfwGetWindowParam(glfwWindow, GLFW_BLUE_BITS);
		abits = glfwGetWindowParam(glfwWindow, GLFW_ALPHA_BITS);
		dbits = glfwGetWindowParam(glfwWindow, GLFW_DEPTH_BITS);
		sbits = glfwGetWindowParam(glfwWindow, GLFW_STENCIL_BITS);
		cbits = rbits + gbits + bbits + abits;
		// This probably is not set only when fullscreen
		if (!fullscreen)
		{
			Cvar_SetValue("r_width", width);
			Cvar_SetValue("r_height", height);
		}
		Cvar_SetValue("r_colorbits", cbits);
		Cvar_SetValue("r_depthbits", dbits);
		Cvar_SetValue("r_stencilbits", sbits);
	}
	
	displayAspect = (float)width / (float)height;
	ri.Printf(PRINT_DEVELOPER, "Actual display aspect: %.3f\n", displayAspect);
		
	glConfig.displayFrequency = glfwGetWindowParam(glfwWindow, GLFW_REFRESH_RATE);
	glConfig.vidHeight = height;
	glConfig.vidWidth = width;
	glConfig.colorBits = cbits;
	glConfig.depthBits = dbits;
	glConfig.stencilBits = sbits;

	GLimp_DetectAvailableModes();

	if (!glfwWindow)
	{
		ri.Printf(PRINT_ALL, "Couldn't get a visual\n");
		return RSERR_INVALID_MODE;
	}

	glstring = (char *)glGetString(GL_RENDERER);
	ri.Printf(PRINT_DEVELOPER, "GL_RENDERER: %s\n", glstring);

	return RSERR_OK;
}

/**
 * GLimp_StartDriverAndSetMode
 */
static qboolean GLimp_StartDriverAndSetMode(int mode, qboolean fullscreen)
{
	rserr_t err;
	static int initialised = qfalse;
	
	if (initialised)
	{
		glfwTerminate();
	}
	
	if (glfwInit())
	{
		int major, minor, rev;
		initialised = qtrue;
		glfwGetVersion(&major, &minor, &rev);
		ri.Printf(PRINT_DEVELOPER, "GLFW version %d.%d.%d\n",
			major, minor, rev);
		// TODO: Get video driver name and put into r_glfwDriver
	}
	else
	{
		ri.Printf(PRINT_ALL, "glfwInit() FAILED\n");
	}

	if (r_fullscreen->integer && Cvar_VariableIntegerValue("in_nograb"))
	{
		ri.Printf(PRINT_ALL, "Fullscreen not allowed with in_nograb 1\n");
		ri.Cvar_Set("r_fullscreen", "0");
	}

	err = GLimp_SetMode(mode, fullscreen);

	switch (err)
	{
		case RSERR_INVALID_FULLSCREEN:
			ri.Printf(PRINT_ALL, "...WARNING: fullscreen unavailable in this mode\n");
			return qfalse;
		case RSERR_INVALID_MODE:
			ri.Printf(PRINT_ALL, "...WARNING: could not set the given mode (%d)\n", mode);
			return qfalse;
		default:
			break;
	}

	return qtrue;
}

/**
 * GLimp_InitExtensions
 */
static void GLimp_InitExtensions(void)
{
	GLenum err;
	
	if (!r_allowExtensions->integer)
	{
		ri.Printf(PRINT_ALL, "* IGNORING OPENGL EXTENSIONS *\n");
		return;
	}

	ri.Printf(PRINT_DEVELOPER, "Initializing OpenGL extensions using GLEW\n");
	
	/* Initialise GLEW */
	err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		ri.Error(ERR_FATAL, (const char *)glewGetErrorString(err));
	}	
	
	// GL_EXT_texture_compression_s3tc
	glConfig.textureCompression = TC_NONE;
	if (r_ext_compressed_textures->value)
	{
		if (GLEW_ARB_texture_compression || GLEW_EXT_texture_compression_s3tc)
		{
			glConfig.textureCompression = TC_EXT_COMP_S3TC;
			ri.Printf(PRINT_DEVELOPER, "...using GL_ARB_texture_compression\n");
		}
		else if (GLEW_S3_s3tc)
		{
			glConfig.textureCompression = TC_S3TC;
			ri.Printf(PRINT_DEVELOPER, "...using GL_S3_s3tc\n");
		}
	}

	// GL_EXT_texture_env_add
	glConfig.textureEnvAddAvailable = qfalse;
	if (r_ext_texture_env_add->integer)
	{
		if (GLEW_EXT_texture_env_add)
		{
			glConfig.textureEnvAddAvailable = qtrue;
			ri.Printf(PRINT_DEVELOPER, "...using GL_EXT_texture_env_add\n");
		}
	}

	// GL_ARB_multitexture
	if (r_ext_multitexture->value)
	{
		if (GLEW_ARB_multitexture)
		{
			GLint glint = 0;
			glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &glint);
			glConfig.maxActiveTextures = (int) glint;
			if (glConfig.maxActiveTextures > 1)
				ri.Printf(PRINT_DEVELOPER, "...using GL_ARB_multitexture\n");
			// TODO: make sure glConfig.maxActiveTextures is checked in renderer
		}
	}

	// GL_EXT_compiled_vertex_array
	if (r_ext_compiled_vertex_array->value)
	{
		if (GLEW_EXT_compiled_vertex_array)
		{
			ri.Printf(PRINT_DEVELOPER, "...using GL_EXT_compiled_vertex_array\n");
		}
	}

	glConfig.anisotropicAvailable = qfalse;
	if (r_ext_texture_filter_anisotropic->integer)
	{
		if (GLEW_EXT_texture_filter_anisotropic)
		{
			glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, (GLint *)&glConfig.maxAnisotropy);
			if (glConfig.maxAnisotropy <= 0) 
			{
				ri.Printf(PRINT_DEVELOPER, "...GL_EXT_texture_filter_anisotropic not properly supported!\n");
				glConfig.maxAnisotropy = 0;
			}
			else
			{
				ri.Printf(PRINT_DEVELOPER, "...using GL_EXT_texture_filter_anisotropic (max: %i)\n", glConfig.maxAnisotropy);
				glConfig.anisotropicAvailable = qtrue;
			}
		}
	}
}


/**
 * GLimp_Init
 * 
 * This routine is responsible for initialising the OS specific portions of OpenGL
 */
void GLimp_Init(void)
{
	r_allowSoftwareGL = ri.Cvar_Get("r_allowSoftwareGL", "0", CVAR_LATCH);
	// TODO: glfw driver...?
	//r_glfwDriver = ri.Cvar_Get("r_glfwDriver", "", CVAR_ROM);

	Sys_GLimpInit();

	// create the window and set up the context
	if(!GLimp_StartDriverAndSetMode(r_mode->integer, r_fullscreen->integer))
	{
		ri.Error(ERR_FATAL, "GLimp_Init() - could not load OpenGL subsystem\n");
	}

	// This values force the UI to disable driver selection
	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;
	glConfig.deviceSupportsGamma = qfalse;
	// glConfig.deviceSupportsGamma = !!(SDL_SetGamma(1.0f, 1.0f, 1.0f) >= 0);
	// TODO: modify GLFW to support gamma
	glConfig.deviceSupportsGamma = qfalse;

	// get our config strings
	Q_strncpyz(glConfig.vendor_string, (char *) glGetString (GL_VENDOR), sizeof(glConfig.vendor_string));
	Q_strncpyz(glConfig.renderer_string, (char *) glGetString (GL_RENDERER), sizeof(glConfig.renderer_string));
	if (*glConfig.renderer_string && glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] == '\n')
		glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] = 0;
	Q_strncpyz(glConfig.version_string, (char *) glGetString (GL_VERSION), sizeof(glConfig.version_string));
	Q_strncpyz(glConfig.extensions_string, (char *) glGetString (GL_EXTENSIONS), sizeof(glConfig.extensions_string));

	// initialize extensions
	GLimp_InitExtensions();

	// Create available modes cvar
	ri.Cvar_Get("r_availableModes", "", CVAR_ROM);

	// This depends on SDL_INIT_VIDEO, hence having it here
	IN_Init();
}


/**
 * GLimp_EndFrame
 * 
 * Responsible for doing a swapbuffers
 */
void GLimp_EndFrame(void)
{
	qboolean fullscreen = qfalse;
	
	// don't flip if drawing to front buffer
	if (Q_stricmp(r_drawBuffer->string, "GL_FRONT") != 0)
		glfwSwapBuffers();

	// check if the fullscreen setting has changed */
	if (r_fullscreen->integer != glConfig.isFullscreen)
	{
		/*
		glfwSetWindowMode(r_fullscreen->integer ?
				GLFW_FULLSCREEN :
				GLFW_WINDOW);
		*/
		glfwCloseWindow(glfwWindow);
		GLimp_Init();
		CL_Vid_Restart_f();
		glfwRestoreWindow(glfwWindow);
	}
}

/**
 * SMP stubs
 */
void GLimp_RenderThreadWrapper(void *arg)
{
}

qboolean GLimp_SpawnRenderThread(void (*function)(void))
{
	ri.Printf(PRINT_WARNING, "ERROR: SMP support was disabled at compile time\n");
	return qfalse;
}

void *GLimp_RendererSleep(void)
{
	return NULL;
}

void GLimp_FrontEndSleep(void)
{
}

void GLimp_WakeRenderer(void *data)
{
}

/**
 * GLimp_SetGamma
 */
void GLimp_SetGamma (unsigned char red[256], unsigned char green[256], unsigned char blue[256])
{
	// TODO: glfw gamma code will be put in soon
}
