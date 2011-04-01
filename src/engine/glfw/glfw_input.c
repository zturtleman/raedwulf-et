/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "../renderer/tr_local.h"
#include "../client/client.h"
#include "../sys/sys_local.h"

#include "glfw_local.h"

#define ARRAYLEN(x) (sizeof(x)/sizeof(x[0]))

#ifdef MACOS_X
// Mouse acceleration needs to be disabled
#define MACOS_X_ACCELERATION_HACK
// Cursor needs hack to hide
#define MACOS_X_CURSOR_HACK
#endif

#ifdef MACOS_X_ACCELERATION_HACK
#include <IOKit/IOTypes.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <IOKit/hidsystem/event_status_driver.h>
#endif

static cvar_t *in_keyboardDebug     = NULL;

static int stick = -1;

static qboolean mouseAvailable = qfalse;
static qboolean mouseActive = qfalse;
static qboolean mouseReset = qfalse;

static cvar_t *in_mouse             = NULL;
#ifdef MACOS_X_ACCELERATION_HACK
static cvar_t *in_disablemacosxmouseaccel = NULL;
static double originalMouseSpeed = -1.0;
#endif
static cvar_t *in_nograb;

static cvar_t *in_joystick          = NULL;
static cvar_t *in_joystickDebug     = NULL;
static cvar_t *in_joystickThreshold = NULL;
static cvar_t *in_joystickNo        = NULL;

#define CTRL(a) ((a)-'a'+1)

/*
===============
IN_PrintKey
===============
*/
static void IN_PrintKey(const int keysym, keyNum_t key, qboolean down)
{
	if(down)
		Com_Printf("+ ");
	else
		Com_Printf("  ");

	Com_Printf("0x%02x", keysym);

	/* Key mods not supported *yet* */

	Com_Printf(" Q:0x%02x(%s)", key, Key_KeynumToString(key));
	Com_Printf(" U:0x%02x", keysym);

	Com_Printf("\n");
}

#define MAX_CONSOLE_KEYS 16

/*
===============
IN_IsConsoleKey
===============
*/
static qboolean IN_IsConsoleKey(keyNum_t key, const unsigned char character)
{
	typedef struct consoleKey_s
	{
		enum
		{
			KEY,
			CHARACTER
		} type;

		union
		{
			keyNum_t key;
			unsigned char character;
		} u;
	} consoleKey_t;

	static consoleKey_t consoleKeys[ MAX_CONSOLE_KEYS ];
	static int numConsoleKeys = 0;
	int i;

	// Only parse the variable when it changes
	if(cl_consoleKeys->modified)
	{
		char *text_p, *token;

		cl_consoleKeys->modified = qfalse;
		text_p = cl_consoleKeys->string;
		numConsoleKeys = 0;

		while(numConsoleKeys < MAX_CONSOLE_KEYS)
		{
			consoleKey_t *c = &consoleKeys[ numConsoleKeys ];
			int charCode = 0;

			token = COM_Parse(&text_p);
			if(!token[ 0 ])
				break;

			if(strlen(token) == 4)
				charCode = Com_HexStrToInt(token);

			if(charCode > 0)
			{
				c->type = CHARACTER;
				c->u.character = (unsigned char)charCode;
			}
			else
			{
				c->type = KEY;
				c->u.key = Key_StringToKeynum(token);

				// 0 isn't a key
				if(c->u.key <= 0)
					continue;
			}

			numConsoleKeys++;
		}
	}

	// If the character is the same as the key, prefer the character
	if(key == character)
		key = 0;

	for(i = 0; i < numConsoleKeys; i++)
	{
		consoleKey_t *c = &consoleKeys[ i ];

		switch(c->type)
		{
			case KEY:
				if(key && c->u.key == key)
					return qtrue;
				break;

			case CHARACTER:
				if(c->u.character == character)
					return qtrue;
				break;
		}
	}

	return qfalse;
}

/*
===============
IN_TranslateGLFWToQ3Key
===============
*/
static keyNum_t IN_TranslateGLFWToQ3Key(int keysym, qboolean down)
{
	keyNum_t key = 0;

	if(keysym >= GLFW_KEY_SPACE && keysym <= GLFW_KEY_WORLD_2)
	{
		// These happen to match the printable chars
		key = (int)tolower(keysym);
	}
	else
	{
		switch (keysym)
		{
			case GLFW_KEY_PAGEUP:       key = K_PGUP;          break;
			case GLFW_KEY_KP_9:         key = K_KP_PGUP;       break;
			case GLFW_KEY_PAGEDOWN:     key = K_PGDN;          break;
			case GLFW_KEY_KP_3:         key = K_KP_PGDN;       break;
			case GLFW_KEY_KP_7:         key = K_KP_HOME;       break;
			case GLFW_KEY_HOME:         key = K_HOME;          break;
			case GLFW_KEY_KP_1:         key = K_KP_END;        break;
			case GLFW_KEY_END:          key = K_END;           break;
			case GLFW_KEY_KP_4:         key = K_KP_LEFTARROW;  break;
			case GLFW_KEY_LEFT:         key = K_LEFTARROW;     break;
			case GLFW_KEY_KP_6:         key = K_KP_RIGHTARROW; break;
			case GLFW_KEY_RIGHT:        key = K_RIGHTARROW;    break;
			case GLFW_KEY_KP_2:         key = K_KP_DOWNARROW;  break;
			case GLFW_KEY_DOWN:         key = K_DOWNARROW;     break;
			case GLFW_KEY_KP_8:         key = K_KP_UPARROW;    break;
			case GLFW_KEY_UP:           key = K_UPARROW;       break;
			case GLFW_KEY_ESC:          key = K_ESCAPE;        break;
			case GLFW_KEY_KP_ENTER:     key = K_KP_ENTER;      break;
			case GLFW_KEY_ENTER:        key = K_ENTER;         break;
			case GLFW_KEY_TAB:          key = K_TAB;           break;
			case GLFW_KEY_F1:           key = K_F1;            break;
			case GLFW_KEY_F2:           key = K_F2;            break;
			case GLFW_KEY_F3:           key = K_F3;            break;
			case GLFW_KEY_F4:           key = K_F4;            break;
			case GLFW_KEY_F5:           key = K_F5;            break;
			case GLFW_KEY_F6:           key = K_F6;            break;
			case GLFW_KEY_F7:           key = K_F7;            break;
			case GLFW_KEY_F8:           key = K_F8;            break;
			case GLFW_KEY_F9:           key = K_F9;            break;
			case GLFW_KEY_F10:          key = K_F10;           break;
			case GLFW_KEY_F11:          key = K_F11;           break;
			case GLFW_KEY_F12:          key = K_F12;           break;
			case GLFW_KEY_F13:          key = K_F13;           break;
			case GLFW_KEY_F14:          key = K_F14;           break;
			case GLFW_KEY_F15:          key = K_F15;           break;

			case GLFW_KEY_BACKSPACE:    key = K_BACKSPACE;     break;
			case GLFW_KEY_KP_DECIMAL:   key = K_KP_DEL;        break;
			case GLFW_KEY_DEL:          key = K_DEL;           break;
			case GLFW_KEY_PAUSE:        key = K_PAUSE;         break;

			case GLFW_KEY_LSHIFT:
			case GLFW_KEY_RSHIFT:       key = K_SHIFT;         break;

			case GLFW_KEY_LCTRL:
			case GLFW_KEY_RCTRL:        key = K_CTRL;          break;

			//case GLFW_KEY_RMETA:
			//case GLFW_KEY_LMETA:        key = K_COMMAND;       break;

			case GLFW_KEY_RALT:
			case GLFW_KEY_LALT:         key = K_ALT;           break;

			case GLFW_KEY_LSUPER:
			case GLFW_KEY_RSUPER:       key = K_SUPER;         break;

			case GLFW_KEY_KP_5:         key = K_KP_5;          break;
			case GLFW_KEY_INSERT:       key = K_INS;           break;
			case GLFW_KEY_KP_0:         key = K_KP_INS;        break;
			case GLFW_KEY_KP_MULTIPLY:  key = K_KP_STAR;       break;
			case GLFW_KEY_KP_ADD:       key = K_KP_PLUS;       break;
			case GLFW_KEY_KP_SUBTRACT:  key = K_KP_MINUS;      break;
			case GLFW_KEY_KP_DIVIDE:    key = K_KP_SLASH;      break;

			//case GLFW_KEY_MODE:         key = K_MODE;          break;
			//case GLFW_KEY_COMPOSE:      key = K_COMPOSE;       break;
			//case GLFW_KEY_HELP:         key = K_HELP;          break;
			//case GLFW_KEY_PRINT:        key = K_PRINT;         break;
			//case GLFW_KEY_SYSREQ:       key = K_SYSREQ;        break;
			//case GLFW_KEY_BREAK:        key = K_BREAK;         break;
			case GLFW_KEY_MENU:         key = K_MENU;          break;
			//case GLFW_KEY_POWER:        key = K_POWER;         break;
			//case GLFW_KEY_EURO:         key = K_EURO;          break;
			//case GLFW_KEY_UNDO:         key = K_UNDO;          break;
			case GLFW_KEY_SCROLL_LOCK:    key = K_SCROLLOCK;     break;
			case GLFW_KEY_KP_NUM_LOCK:    key = K_KP_NUMLOCK;    break;
			case GLFW_KEY_CAPS_LOCK:      key = K_CAPSLOCK;      break;

			default:
				// TODO: What does this do again?
				if (keysym <= GLFW_KEY_WORLD_2)
					key = (keysym - 160) + K_WORLD_0;
				break;
		}
	}

	if(in_keyboardDebug->integer)
		IN_PrintKey(keysym, key, down);

	// Keys that have ASCII names but produce no character are probably
	// dead keys -- ignore them
	if (down && strlen(Key_KeynumToString(key)) == 1 &&
		keysym > GLFW_KEY_LAST)
	{
		if(in_keyboardDebug->integer)
			Com_Printf("  Ignored dead key '%c'\n", key);

		key = 0;
	}

	if (IN_IsConsoleKey(key, key))
	{
		// Console keys can't be bound or generate characters
		key = K_CONSOLE;
	}

	return key;
}

#ifdef MACOS_X_ACCELERATION_HACK
/*
===============
IN_GetIOHandle
===============
*/
static io_connect_t IN_GetIOHandle(void) // mac os x mouse accel hack
{
	io_connect_t iohandle = MACH_PORT_NULL;
	kern_return_t status;
	io_service_t iohidsystem = MACH_PORT_NULL;
	mach_port_t masterport;

	status = IOMasterPort(MACH_PORT_NULL, &masterport);
	if(status != KERN_SUCCESS)
		return 0;

	iohidsystem = IORegistryEntryFromPath(masterport, kIOServicePlane ":/IOResources/IOHIDSystem");
	if(!iohidsystem)
		return 0;

	status = IOServiceOpen(iohidsystem, mach_task_self(), kIOHIDParamConnectType, &iohandle);
	IOObjectRelease(iohidsystem);

	return iohandle;
}
#endif

/*
===============
IN_ActivateMouse
===============
*/
static void IN_ActivateMouse(void)
{
	if (!mouseAvailable)
		return;

#ifdef MACOS_X_ACCELERATION_HACK
	if (!mouseActive) // mac os x mouse accel hack
	{
		// Save the status of mouse acceleration
		originalMouseSpeed = -1.0; // in case of error
		if(in_disablemacosxmouseaccel->integer)
		{
			io_connect_t mouseDev = IN_GetIOHandle();
			if(mouseDev != 0)
			{
				if(IOHIDGetAccelerationWithKey(mouseDev, CFSTR(kIOHIDMouseAccelerationType), &originalMouseSpeed) == kIOReturnSuccess)
				{
					Com_Printf("previous mouse acceleration: %f\n", originalMouseSpeed);
					if(IOHIDSetAccelerationWithKey(mouseDev, CFSTR(kIOHIDMouseAccelerationType), -1.0) != kIOReturnSuccess)
					{
						Com_Printf("Could not disable mouse acceleration (failed at IOHIDSetAccelerationWithKey).\n");
						Cvar_Set ("in_disablemacosxmouseaccel", 0);
					}
				}
				else
				{
					Com_Printf("Could not disable mouse acceleration (failed at IOHIDGetAccelerationWithKey).\n");
					Cvar_Set ("in_disablemacosxmouseaccel", 0);
				}
				IOServiceClose(mouseDev);
			}
			else
			{
				Com_Printf("Could not disable mouse acceleration (failed at IO_GetIOHandle).\n");
				Cvar_Set ("in_disablemacosxmouseaccel", 0);
			}
		}
	}
#endif

	if(!mouseActive)
	{
		glfwEnable(glfwWindow, GLFW_MOUSE_CURSOR);
	}

	// in_nograb makes no sense in fullscreen mode
	if (!r_fullscreen->integer)
	{
		if (in_nograb->modified || !mouseActive)
		{
			if (in_nograb->integer)
			{
				glfwEnable(glfwWindow, GLFW_MOUSE_CURSOR);
			}
			else
			{
				glfwDisable(glfwWindow, GLFW_MOUSE_CURSOR);
			}
			in_nograb->modified = qfalse;
		}
	}
	
	mouseActive = qtrue;
}

/*
===============
IN_DeactivateMouse
===============
*/
static void IN_DeactivateMouse(void)
{
	if(r_fullscreen->integer || !mouseAvailable)
		return;

#ifdef MACOS_X_ACCELERATION_HACK
	if (mouseActive) // mac os x mouse accel hack
	{
		if(originalMouseSpeed != -1.0)
		{
			io_connect_t mouseDev = IN_GetIOHandle();
			if(mouseDev != 0)
			{
				Com_Printf("restoring mouse acceleration to: %f\n", originalMouseSpeed);
				if(IOHIDSetAccelerationWithKey(mouseDev, CFSTR(kIOHIDMouseAccelerationType), originalMouseSpeed) != kIOReturnSuccess)
					Com_Printf("Could not re-enable mouse acceleration (failed at IOHIDSetAccelerationWithKey).\n");
				IOServiceClose(mouseDev);
			}
			else
				Com_Printf("Could not re-enable mouse acceleration (failed at IO_GetIOHandle).\n");
		}
	}
#endif

	if(mouseActive)
	{
		glfwEnable(glfwWindow, GLFW_MOUSE_CURSOR);

		mouseReset = qtrue;
		mouseActive = qfalse;
	}
}

// We translate axes movement into keypresses
static int joy_keys[16] = {
	K_LEFTARROW, K_RIGHTARROW,
	K_UPARROW, K_DOWNARROW,
	K_JOY16, K_JOY17,
	K_JOY18, K_JOY19,
	K_JOY20, K_JOY21,
	K_JOY22, K_JOY23,

	K_JOY24, K_JOY25,
	K_JOY26, K_JOY27
};

struct
{
	qboolean buttons[16];  // !!! FIXME: these might be too many.
	unsigned int oldaxes;
} stick_state;


/*
===============
IN_InitJoystick
===============
*/
static void IN_InitJoystick(void)
{
	int i = 0;
	int total = 0;

	stick = -1;
	memset(&stick_state, '\0', sizeof (stick_state));

	if(!in_joystick->integer)
	{
		Com_DPrintf("Joystick is not active.\n");
		return;
	}

	for (i = 0; i < 16; i++)
	{
		if (glfwGetJoystickParam(GLFW_JOYSTICK_1 + i, GLFW_PRESENT))
			total++;
	}
	Com_DPrintf("%d possible joysticks\n", total);
	// joystick name not supported
	//for (i = 0; i < total; i++)
	//	Com_DPrintf("[%d] %s\n", i, SDL_JoystickName(i));

	in_joystickNo = Cvar_Get("in_joystickNo", "0", CVAR_ARCHIVE);
	if(in_joystickNo->integer < 0 || in_joystickNo->integer >= total)
		Cvar_Set("in_joystickNo", "0");

	stick = GLFW_JOYSTICK_1;

	if (glfwGetJoystickParam(stick, GLFW_PRESENT))
	{
		Com_DPrintf("No joystick opened.\n");
		return;
	}
	
	Com_DPrintf("Joystick %d opened\n", in_joystickNo->integer);
	//Com_DPrintf("Name:    %s\n", SDL_JoystickName(in_joystickNo->integer));
	Com_DPrintf("Axes:    %d\n", glfwGetJoystickParam(stick, GLFW_AXES));
	Com_DPrintf("Buttons: %d\n", glfwGetJoystickParam(stick, GLFW_BUTTONS));
}

/*
===============
IN_JoyMove
===============
*/
static void IN_JoyMove(void)
{
	unsigned char joy_buttons[ARRAYLEN(stick_state.buttons)];
	float joy_axes[16];
	int axes = 0;
	int total = 0;
	int i = 0;

	if (!stick)
		return;

	memset(joy_buttons, GLFW_PRESS, sizeof (joy_buttons));

	// TODO: Add configurability so that we know which
	// axes are balls and which buttons are hats
	
	// now query the stick buttons...
	total = glfwGetJoystickButtons(stick, joy_buttons, ARRAYLEN(joy_buttons));
	if (total > 0)
	{
		if (total > ARRAYLEN(stick_state.buttons))
			total = ARRAYLEN(stick_state.buttons);
		for (i = 0; i < total; i++)
		{
			if (joy_buttons[i] != stick_state.buttons[i])
			{
				Com_QueueEvent(0, SE_KEY, K_JOY1 + i, joy_buttons[i], 0, NULL);
				stick_state.buttons[i] = joy_buttons[i];
			}
		}
	}

	// finally, look at the axes...
	total = glfwGetJoystickPos(stick, joy_axes, ARRAYLEN(joy_axes));
	if (total > 0)
	{
		if (total > 16) total = 16;
		for (i = 0; i < total; i++)
		{
			float f = joy_axes[i];
			if(f < -in_joystickThreshold->value)
				axes |= (1 << (i * 2));
			else if(f > in_joystickThreshold->value)
				axes |= (1 << ((i * 2) + 1));
		}
	}

	/* Time to update axes state based on old vs. new. */
	if (axes != stick_state.oldaxes)
	{
		for(i = 0; i < 16; i++)
		{
			if((axes & (1 << i)) && !(stick_state.oldaxes & (1 << i)))
				Com_QueueEvent(0, SE_KEY, joy_keys[i], qtrue, 0, NULL);

			if(!(axes & (1 << i)) && (stick_state.oldaxes & (1 << i)))
				Com_QueueEvent(0, SE_KEY, joy_keys[i], qfalse, 0, NULL);
		}
	}

	/* Save for future generations. */
	stick_state.oldaxes = axes;
}

/**
 * IN_Frame
 */
void IN_Frame(void)
{
	qboolean loading;
	
	glfwPollEvents();
	IN_JoyMove();

	// If not DISCONNECTED (main menu) or ACTIVE (in game), we're loading
	loading = !!(cls.state != CA_DISCONNECTED && cls.state != CA_ACTIVE);

	if (!r_fullscreen->integer && (Key_GetCatcher() & KEYCATCH_CONSOLE))
	{
		// Console is down in windowed mode
		IN_DeactivateMouse();
	}
	else if (!r_fullscreen->integer && loading)
	{
		// Loading in windowed mode
		IN_DeactivateMouse();
	}
	/*
	else if (!glfwGetWindowParam(glfwWindow, GLFW_ACTIVE))
	{
		// Window not got focus
		IN_DeactivateMouse();
	}
	*/
	else
	{
		IN_ActivateMouse();
	}
}

static void IN_KeyCallback(GLFWwindow window, int key, int action)
{
	keyNum_t q3key;
	qboolean press = action == GLFW_PRESS;
	if ((q3key = IN_TranslateGLFWToQ3Key(key, press)))
	{
		Com_QueueEvent(0, SE_KEY, q3key, press, 0, NULL);
		// workaround backspace not a character
		if (q3key == K_BACKSPACE && press)
		{
			Com_QueueEvent(0, SE_CHAR, 'h' - 'a' + 1, 0, 0, NULL);
		}
	}
}

static void IN_CharCallback(GLFWwindow window, int character)
{
	if (!IN_IsConsoleKey(0, character))
		Com_QueueEvent(0, SE_CHAR, character, 0, 0, NULL);
}

static void IN_MouseButtonCallback(GLFWwindow window, int button, int action)
{
	unsigned char b;
	switch (button)
	{
		case GLFW_MOUSE_BUTTON_LEFT:     b = K_MOUSE1; break;
		case GLFW_MOUSE_BUTTON_MIDDLE:   b = K_MOUSE3; break;
		case GLFW_MOUSE_BUTTON_RIGHT:    b = K_MOUSE2; break;
		case GLFW_MOUSE_BUTTON_4:        b = K_MOUSE4; break;
		case GLFW_MOUSE_BUTTON_5:        b = K_MOUSE5; break;
		default:  b = K_AUX1 + (button - GLFW_MOUSE_BUTTON_6) % 16; break;
	}
	Com_QueueEvent(0, SE_KEY, b,
		(action == GLFW_PRESS ? qtrue : qfalse), 0, NULL);
}

static void IN_MousePosCallback(GLFWwindow window, int x, int y)
{
	static int lastX = 0, lastY = 0;
	if (mouseActive)
	{
		if (!mouseReset)
		{
			Com_QueueEvent(0, SE_MOUSE, x - lastX, y - lastY, 0, NULL);
			Com_DPrintf("(%d, %d) ... (%d, %d)\n", x, y, x - lastX, y - lastY);
		}
		else
		{
			mouseReset = qfalse;
		}
		lastX = x;
		lastY = y;
	}
}

static void IN_ScrollCallback(GLFWwindow window, int x, int y)
{
	static int lastPos = 0;
	unsigned char b;
	if (x > lastPos)
	{
		b = K_MWHEELUP;
	}
	else if (x < lastPos)
	{
		b = K_MWHEELDOWN;
	}
	lastPos = x;
}

/**
 * IN_Init
 */
void IN_Init(void)
{
	Com_DPrintf("\n------- Input Initialization -------\n");

	// glfw callback hooks
	glfwSetKeyCallback(IN_KeyCallback);
	glfwSetCharCallback(IN_CharCallback);
	glfwSetMouseButtonCallback(IN_MouseButtonCallback);
	glfwSetMousePosCallback(IN_MousePosCallback);
	glfwSetScrollCallback(IN_ScrollCallback);
	
	in_keyboardDebug = Cvar_Get("in_keyboardDebug", "0", CVAR_ARCHIVE);

	// enable key repeating
	glfwEnable(glfwWindow, GLFW_KEY_REPEAT);
	
	// mouse variables
	in_mouse = Cvar_Get("in_mouse", "1", CVAR_ARCHIVE);
	in_nograb = Cvar_Get("in_nograb", "0", CVAR_ARCHIVE);

	in_joystick = Cvar_Get("in_joystick", "0", CVAR_ARCHIVE|CVAR_LATCH);
	in_joystickDebug = Cvar_Get("in_joystickDebug", "0", CVAR_TEMP);
	in_joystickThreshold = Cvar_Get("in_joystickThreshold", "0.15", CVAR_ARCHIVE);

#ifdef MACOS_X_ACCELERATION_HACK
	in_disablemacosxmouseaccel = Cvar_Get("in_disablemacosxmouseaccel", "0", CVAR_ARCHIVE);
#endif

	if (in_mouse->value)
	{
		mouseAvailable = qtrue;
		IN_ActivateMouse();
	}
	else
	{
		mouseAvailable = qfalse;
		glfwEnable(glfwWindow, GLFW_MOUSE_CURSOR);
		mouseReset = qtrue;
		mouseActive = qfalse;
	}

	IN_InitJoystick();
	Com_DPrintf("------------------------------------\n");
}

/*
===============
IN_Shutdown
===============
*/
void IN_Shutdown(void)
{
	IN_DeactivateMouse();
	mouseAvailable = qfalse;
}

/*
===============
IN_Restart
===============
*/
void IN_Restart(void)
{
	IN_Init();
}
