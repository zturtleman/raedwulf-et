//========================================================================
// GLFW - An OpenGL framework
// Platform:    Cocoa/NSOpenGL
// API Version: 3.0
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2009-2010 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "internal.h"


//========================================================================
// Delegate for window related notifications
//========================================================================

@interface GLFWWindowDelegate : NSObject
{
    _GLFWwindow* window;
}

- (id)initWithGlfwWindow:(_GLFWwindow *)initWndow;

@end

@implementation GLFWWindowDelegate

- (id)initWithGlfwWindow:(_GLFWwindow *)initWindow
{
    self = [super init];
    if (self != nil)
        window = initWindow;

    return self;
}

- (BOOL)windowShouldClose:(id)sender
{
    window->closeRequested = GL_TRUE;

    return NO;
}

- (void)windowDidResize:(NSNotification *)notification
{
    [window->NSGL.context update];

    NSRect contentRect =
        [window->NS.window contentRectForFrameRect:[window->NS.window frame]];
    window->width = contentRect.size.width;
    window->height = contentRect.size.height;

    if (window->windowSizeCallback)
        window->windowSizeCallback(window, window->width, window->height);
}

- (void)windowDidMove:(NSNotification *)notification
{
    [window->NSGL.context update];

    NSRect contentRect =
        [window->NS.window contentRectForFrameRect:[window->NS.window frame]];

    CGPoint mainScreenOrigin = CGDisplayBounds(CGMainDisplayID()).origin;
    double mainScreenHeight = CGDisplayBounds(CGMainDisplayID()).size.height;
    CGPoint flippedPos = CGPointMake(contentRect.origin.x - mainScreenOrigin.x,
                                      mainScreenHeight - contentRect.origin.y -
                                          mainScreenOrigin.y - window->height);

    window->positionX = flippedPos.x;
    window->positionY = flippedPos.y;
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    window->iconified = GL_TRUE;

    if (window->windowIconifyCallback)
        window->windowIconifyCallback(window, window->iconified);
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    window->iconified = GL_FALSE;

    if (window->windowIconifyCallback)
        window->windowIconifyCallback(window, window->iconified);
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    _glfwInputWindowFocus(window, GL_TRUE);
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    _glfwInputWindowFocus(window, GL_FALSE);
}

@end

//========================================================================
// Delegate for application related notifications
//========================================================================

@interface GLFWApplicationDelegate : NSObject
@end

@implementation GLFWApplicationDelegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    _GLFWwindow* window;

    for (window = _glfwLibrary.windowListHead;  window;  window = window->next)
        window->closeRequested = GL_TRUE;

    return NSTerminateCancel;
}

@end

//========================================================================
// Keyboard symbol translation table
//========================================================================

// TODO: Need to find mappings for F13-F15, volume down/up/mute, and eject.
static const unsigned int MAC_TO_GLFW_KEYCODE_MAPPING[128] =
{
    /* 00 */ 'A',
    /* 01 */ 'S',
    /* 02 */ 'D',
    /* 03 */ 'F',
    /* 04 */ 'H',
    /* 05 */ 'G',
    /* 06 */ 'Z',
    /* 07 */ 'X',
    /* 08 */ 'C',
    /* 09 */ 'V',
    /* 0a */ -1,
    /* 0b */ 'B',
    /* 0c */ 'Q',
    /* 0d */ 'W',
    /* 0e */ 'E',
    /* 0f */ 'R',
    /* 10 */ 'Y',
    /* 11 */ 'T',
    /* 12 */ '1',
    /* 13 */ '2',
    /* 14 */ '3',
    /* 15 */ '4',
    /* 16 */ '6',
    /* 17 */ '5',
    /* 18 */ '=',
    /* 19 */ '9',
    /* 1a */ '7',
    /* 1b */ '-',
    /* 1c */ '8',
    /* 1d */ '0',
    /* 1e */ ']',
    /* 1f */ 'O',
    /* 20 */ 'U',
    /* 21 */ '[',
    /* 22 */ 'I',
    /* 23 */ 'P',
    /* 24 */ GLFW_KEY_ENTER,
    /* 25 */ 'L',
    /* 26 */ 'J',
    /* 27 */ '\'',
    /* 28 */ 'K',
    /* 29 */ ';',
    /* 2a */ '\\',
    /* 2b */ ',',
    /* 2c */ '/',
    /* 2d */ 'N',
    /* 2e */ 'M',
    /* 2f */ '.',
    /* 30 */ GLFW_KEY_TAB,
    /* 31 */ GLFW_KEY_SPACE,
    /* 32 */ '`',
    /* 33 */ GLFW_KEY_BACKSPACE,
    /* 34 */ -1,
    /* 35 */ GLFW_KEY_ESC,
    /* 36 */ GLFW_KEY_RSUPER,
    /* 37 */ GLFW_KEY_LSUPER,
    /* 38 */ GLFW_KEY_LSHIFT,
    /* 39 */ GLFW_KEY_CAPS_LOCK,
    /* 3a */ GLFW_KEY_LALT,
    /* 3b */ GLFW_KEY_LCTRL,
    /* 3c */ GLFW_KEY_RSHIFT,
    /* 3d */ GLFW_KEY_RALT,
    /* 3e */ GLFW_KEY_RCTRL,
    /* 3f */ -1, /*Function*/
    /* 40 */ GLFW_KEY_F17,
    /* 41 */ GLFW_KEY_KP_DECIMAL,
    /* 42 */ -1,
    /* 43 */ GLFW_KEY_KP_MULTIPLY,
    /* 44 */ -1,
    /* 45 */ GLFW_KEY_KP_ADD,
    /* 46 */ -1,
    /* 47 */ -1, /*KeypadClear*/
    /* 48 */ -1, /*VolumeUp*/
    /* 49 */ -1, /*VolumeDown*/
    /* 4a */ -1, /*Mute*/
    /* 4b */ GLFW_KEY_KP_DIVIDE,
    /* 4c */ GLFW_KEY_KP_ENTER,
    /* 4d */ -1,
    /* 4e */ GLFW_KEY_KP_SUBTRACT,
    /* 4f */ GLFW_KEY_F18,
    /* 50 */ GLFW_KEY_F19,
    /* 51 */ GLFW_KEY_KP_EQUAL,
    /* 52 */ GLFW_KEY_KP_0,
    /* 53 */ GLFW_KEY_KP_1,
    /* 54 */ GLFW_KEY_KP_2,
    /* 55 */ GLFW_KEY_KP_3,
    /* 56 */ GLFW_KEY_KP_4,
    /* 57 */ GLFW_KEY_KP_5,
    /* 58 */ GLFW_KEY_KP_6,
    /* 59 */ GLFW_KEY_KP_7,
    /* 5a */ GLFW_KEY_F20,
    /* 5b */ GLFW_KEY_KP_8,
    /* 5c */ GLFW_KEY_KP_9,
    /* 5d */ -1,
    /* 5e */ -1,
    /* 5f */ -1,
    /* 60 */ GLFW_KEY_F5,
    /* 61 */ GLFW_KEY_F6,
    /* 62 */ GLFW_KEY_F7,
    /* 63 */ GLFW_KEY_F3,
    /* 64 */ GLFW_KEY_F8,
    /* 65 */ GLFW_KEY_F9,
    /* 66 */ -1,
    /* 67 */ GLFW_KEY_F11,
    /* 68 */ -1,
    /* 69 */ GLFW_KEY_F13,
    /* 6a */ GLFW_KEY_F16,
    /* 6b */ GLFW_KEY_F14,
    /* 6c */ -1,
    /* 6d */ GLFW_KEY_F10,
    /* 6e */ -1,
    /* 6f */ GLFW_KEY_F12,
    /* 70 */ -1,
    /* 71 */ GLFW_KEY_F15,
    /* 72 */ GLFW_KEY_INSERT, /*Help*/
    /* 73 */ GLFW_KEY_HOME,
    /* 74 */ GLFW_KEY_PAGEUP,
    /* 75 */ GLFW_KEY_DEL,
    /* 76 */ GLFW_KEY_F4,
    /* 77 */ GLFW_KEY_END,
    /* 78 */ GLFW_KEY_F2,
    /* 79 */ GLFW_KEY_PAGEDOWN,
    /* 7a */ GLFW_KEY_F1,
    /* 7b */ GLFW_KEY_LEFT,
    /* 7c */ GLFW_KEY_RIGHT,
    /* 7d */ GLFW_KEY_DOWN,
    /* 7e */ GLFW_KEY_UP,
    /* 7f */ -1,
};

//========================================================================
// Converts a Mac OS X keycode to a GLFW keycode
//========================================================================

static int convertMacKeyCode(unsigned int macKeyCode)
{
    if (macKeyCode >= 128)
        return -1;

    // This treats keycodes as *positional*; that is, we'll return 'a'
    // for the key left of 's', even on an AZERTY keyboard.  The charInput
    // function should still get 'q' though.
    return MAC_TO_GLFW_KEYCODE_MAPPING[macKeyCode];
}

//========================================================================
// Content view class for the GLFW window
//========================================================================

@interface GLFWContentView : NSView
{
    _GLFWwindow* window;
}

- (id)initWithGlfwWindow:(_GLFWwindow *)initWindow;

@end

@implementation GLFWContentView

- (id)initWithGlfwWindow:(_GLFWwindow *)initWindow
{
    self = [super init];
    if (self != nil)
        window = initWindow;

    return self;
}

- (BOOL)isOpaque
{
    return YES;
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)mouseDown:(NSEvent *)event
{
    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
}

- (void)mouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)mouseUp:(NSEvent *)event
{
    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
}

- (void)mouseMoved:(NSEvent *)event
{
    if (window == _glfwLibrary.cursorLockWindow)
    {
        window->mousePosX += [event deltaX];
        window->mousePosY += [event deltaY];
    }
    else
    {
        NSPoint p = [event locationInWindow];

        // Cocoa coordinate system has origin at lower left
        window->mousePosX = p.x;
        window->mousePosY = [[window->NS.window contentView] bounds].size.height - p.y;
    }

    if (window->mousePosCallback)
        window->mousePosCallback(window, window->mousePosX, window->mousePosY);
}

- (void)rightMouseDown:(NSEvent *)event
{
    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);
}

- (void)rightMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)rightMouseUp:(NSEvent *)event
{
    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE);
}

- (void)otherMouseDown:(NSEvent *)event
{
    _glfwInputMouseClick(window, [event buttonNumber], GLFW_PRESS);
}

- (void)otherMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)otherMouseUp:(NSEvent *)event
{
    _glfwInputMouseClick(window, [event buttonNumber], GLFW_RELEASE);
}

- (void)keyDown:(NSEvent *)event
{
    NSUInteger i, length;
    NSString* characters;
    int code = convertMacKeyCode([event keyCode]);

    if (code != -1)
    {
        _glfwInputKey(window, code, GLFW_PRESS);

        if ([event modifierFlags] & NSCommandKeyMask)
        {
            if (!window->sysKeysDisabled)
                [super keyDown:event];
        }
        else
        {
            characters = [event characters];
            length = [characters length];

            for (i = 0;  i < length;  i++)
                _glfwInputChar(window, [characters characterAtIndex:i]);
        }
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    int mode;
    unsigned int newModifierFlags =
        [event modifierFlags] | NSDeviceIndependentModifierFlagsMask;

    if (newModifierFlags > window->NS.modifierFlags)
        mode = GLFW_PRESS;
    else
        mode = GLFW_RELEASE;

    window->NS.modifierFlags = newModifierFlags;
    _glfwInputKey(window, MAC_TO_GLFW_KEYCODE_MAPPING[[event keyCode]], mode);
}

- (void)keyUp:(NSEvent *)event
{
    int code = convertMacKeyCode([event keyCode]);
    if (code != -1)
        _glfwInputKey(window, code, GLFW_RELEASE);
}

- (void)scrollWheel:(NSEvent *)event
{
    double deltaX = window->NS.fracScrollX + [event deltaX];
    double deltaY = window->NS.fracScrollY + [event deltaY];

    if ((int) deltaX || (int) deltaY)
        _glfwInputScroll(window, (int) deltaX, (int) deltaY);

    window->NS.fracScrollX = (int) (deltaX - floor(deltaX));
    window->NS.fracScrollY = (int) (deltaY - floor(deltaY));
}

@end


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Here is where the window is created, and the OpenGL rendering context is
// created
//========================================================================

int _glfwPlatformOpenWindow(_GLFWwindow* window,
                            const _GLFWwndconfig *wndconfig,
                            const _GLFWfbconfig *fbconfig)
{
    // Fail if OpenGL 3.0 or above was requested
    if (wndconfig->glMajor > 2)
    {
        _glfwSetError(GLFW_VERSION_UNAVAILABLE);
        return GL_FALSE;
    }

    // We can only have one application delegate, but we only allocate it the
    // first time we create a window to keep all window code in this file
    if (_glfwLibrary.NS.delegate == nil)
    {
        _glfwLibrary.NS.delegate = [[GLFWApplicationDelegate alloc] init];
        if (_glfwLibrary.NS.delegate == nil)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR);
            return GL_FALSE;
        }

        [NSApp setDelegate:_glfwLibrary.NS.delegate];
    }

    window->NS.delegate = [[GLFWWindowDelegate alloc] initWithGlfwWindow:window];
    if (window->NS.delegate == nil)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR);
        return GL_FALSE;
    }

    // Mac OS X needs non-zero color size, so set resonable values
    int colorBits = fbconfig->redBits + fbconfig->greenBits + fbconfig->blueBits;
    if (colorBits == 0)
        colorBits = 24;
    else if (colorBits < 15)
        colorBits = 15;

    // Ignored hints:
    // OpenGLMajor, OpenGLMinor, OpenGLForward:
    //     pending Mac OS X support for OpenGL 3.x
    // OpenGLDebug
    //     pending it meaning anything on Mac OS X

    // Don't use accumulation buffer support; it's not accelerated
    // Aux buffers probably aren't accelerated either

    CFDictionaryRef fullscreenMode = NULL;
    if (wndconfig->mode == GLFW_FULLSCREEN)
    {
        // I think it's safe to pass 0 to the refresh rate for this function
        // rather than conditionalizing the code to call the version which
        // doesn't specify refresh...
        fullscreenMode =
            CGDisplayBestModeForParametersAndRefreshRateWithProperty(
                CGMainDisplayID(),
                colorBits + fbconfig->alphaBits,
                window->width, window->height,
                wndconfig->refreshRate,
                // Controversial, see macosx_fullscreen.m for discussion
                kCGDisplayModeIsSafeForHardware,
                NULL);

        window->width =
            [[(id)fullscreenMode objectForKey:(id)kCGDisplayWidth] intValue];
        window->height =
            [[(id)fullscreenMode objectForKey:(id)kCGDisplayHeight] intValue];
    }

    unsigned int styleMask = 0;

    if (wndconfig->mode == GLFW_WINDOWED)
    {
        styleMask = NSTitledWindowMask | NSClosableWindowMask |
                    NSMiniaturizableWindowMask;

        if (!wndconfig->windowNoResize)
            styleMask |= NSResizableWindowMask;
    }
    else
        styleMask = NSBorderlessWindowMask;

    window->NS.window = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(0, 0, window->width, window->height)
                  styleMask:styleMask
                    backing:NSBackingStoreBuffered
                      defer:NO];

    [window->NS.window setTitle:[NSString stringWithCString:wndconfig->title
                                                   encoding:NSISOLatin1StringEncoding]];

    [window->NS.window setContentView:[[GLFWContentView alloc] initWithGlfwWindow:window]];
    [window->NS.window setDelegate:window->NS.delegate];
    [window->NS.window setAcceptsMouseMovedEvents:YES];
    [window->NS.window center];

    if (wndconfig->mode == GLFW_FULLSCREEN)
    {
        CGCaptureAllDisplays();
        CGDisplaySwitchToMode(CGMainDisplayID(), fullscreenMode);
    }

    unsigned int attribute_count = 0;

#define ADD_ATTR(x) attributes[attribute_count++] = x
#define ADD_ATTR2(x, y) { ADD_ATTR(x); ADD_ATTR(y); }

    // Arbitrary array size here
    NSOpenGLPixelFormatAttribute attributes[24];

    ADD_ATTR(NSOpenGLPFADoubleBuffer);

    if (wndconfig->mode == GLFW_FULLSCREEN)
    {
        ADD_ATTR(NSOpenGLPFAFullScreen);
        ADD_ATTR(NSOpenGLPFANoRecovery);
        ADD_ATTR2(NSOpenGLPFAScreenMask,
                  CGDisplayIDToOpenGLDisplayMask(CGMainDisplayID()));
    }

    ADD_ATTR2(NSOpenGLPFAColorSize, colorBits);

    if (fbconfig->alphaBits > 0)
        ADD_ATTR2(NSOpenGLPFAAlphaSize, fbconfig->alphaBits);

    if (fbconfig->depthBits > 0)
        ADD_ATTR2(NSOpenGLPFADepthSize, fbconfig->depthBits);

    if (fbconfig->stencilBits > 0)
        ADD_ATTR2(NSOpenGLPFAStencilSize, fbconfig->stencilBits);

    int accumBits = fbconfig->accumRedBits + fbconfig->accumGreenBits +
                    fbconfig->accumBlueBits + fbconfig->accumAlphaBits;

    if (accumBits > 0)
        ADD_ATTR2(NSOpenGLPFAAccumSize, accumBits);

    if (fbconfig->auxBuffers > 0)
        ADD_ATTR2(NSOpenGLPFAAuxBuffers, fbconfig->auxBuffers);

    if (fbconfig->stereo)
        ADD_ATTR(NSOpenGLPFAStereo );

    if (fbconfig->samples > 0)
    {
        ADD_ATTR2(NSOpenGLPFASampleBuffers, 1);
        ADD_ATTR2(NSOpenGLPFASamples, fbconfig->samples);
    }

    ADD_ATTR(0);

#undef ADD_ATTR
#undef ADD_ATTR2

    window->NSGL.pixelFormat =
        [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    if (window->NSGL.pixelFormat == nil)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR);
        return GL_FALSE;
    }

    NSOpenGLContext* share = NULL;

    if (wndconfig->share)
        share = wndconfig->share->NSGL.context;

    window->NSGL.context =
        [[NSOpenGLContext alloc] initWithFormat:window->NSGL.pixelFormat
                                   shareContext:share];
    if (window->NSGL.context == nil)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR);
        return GL_FALSE;
    }

    [window->NS.window makeKeyAndOrderFront:nil];
    [window->NSGL.context setView:[window->NS.window contentView]];

    if (wndconfig->mode == GLFW_FULLSCREEN)
    {
        // TODO: Make this work on pre-Leopard systems
        [[window->NS.window contentView] enterFullScreenMode:[NSScreen mainScreen]
                                                 withOptions:nil];
    }

    glfwMakeWindowCurrent(window);

    NSPoint point = [[NSCursor currentCursor] hotSpot];
    window->mousePosX = point.x;
    window->mousePosY = point.y;

    window->windowNoResize = wndconfig->windowNoResize;

    return GL_TRUE;
}

//========================================================================
// Make the OpenGL context associated with the specified window current
//========================================================================

void _glfwPlatformMakeWindowCurrent(_GLFWwindow* window)
{
    if (window)
        [window->NSGL.context makeCurrentContext];
    else
        [NSOpenGLContext clearCurrentContext];
}


//========================================================================
// Properly kill the window / video display
//========================================================================

void _glfwPlatformCloseWindow(_GLFWwindow* window)
{
    [window->NS.window orderOut:nil];

    if (window->mode == GLFW_FULLSCREEN)
    {
        [[window->NS.window contentView] exitFullScreenModeWithOptions:nil];

        CGDisplaySwitchToMode(CGMainDisplayID(),
                              (CFDictionaryRef) _glfwLibrary.NS.desktopMode);
        CGReleaseAllDisplays();
    }

    [window->NSGL.pixelFormat release];
    window->NSGL.pixelFormat = nil;

    [NSOpenGLContext clearCurrentContext];
    [window->NSGL.context release];
    window->NSGL.context = nil;

    [window->NS.window setDelegate:nil];
    [window->NS.delegate release];
    window->NS.delegate = nil;

    [window->NS.window close];
    window->NS.window = nil;

    // TODO: Probably more cleanup
}

//========================================================================
// Set the window title
//========================================================================

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char *title)
{
    [window->NS.window setTitle:[NSString stringWithCString:title
                       encoding:NSISOLatin1StringEncoding]];
}

//========================================================================
// Set the window size
//========================================================================

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    [window->NS.window setContentSize:NSMakeSize(width, height)];
}

//========================================================================
// Set the window position
//========================================================================

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int x, int y)
{
    NSRect contentRect =
        [window->NS.window contentRectForFrameRect:[window->NS.window frame]];

    // We assume here that the client code wants to position the window within the
    // screen the window currently occupies
    NSRect screenRect = [[window->NS.window screen] visibleFrame];
    contentRect.origin = NSMakePoint(screenRect.origin.x + x,
                                     screenRect.origin.y + screenRect.size.height -
                                         y - contentRect.size.height);

    [window->NS.window setFrame:[window->NS.window frameRectForContentRect:contentRect]
                        display:YES];
}

//========================================================================
// Iconify the window
//========================================================================

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
    [window->NS.window miniaturize:nil];
}

//========================================================================
// Restore (un-iconify) the window
//========================================================================

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
    [window->NS.window deminiaturize:nil];
}

//========================================================================
// Swap buffers
//========================================================================

void _glfwPlatformSwapBuffers(void)
{
    _GLFWwindow* window = _glfwLibrary.currentWindow;

    // ARP appears to be unnecessary, but this is future-proof
    [window->NSGL.context flushBuffer];
}

//========================================================================
// Set double buffering swap interval
//========================================================================

void _glfwPlatformSwapInterval(int interval)
{
    _GLFWwindow* window = _glfwLibrary.currentWindow;

    GLint sync = interval;
    [window->NSGL.context setValues:&sync forParameter:NSOpenGLCPSwapInterval];
}

//========================================================================
// Write back window parameters into GLFW window structure
//========================================================================

void _glfwPlatformRefreshWindowParams(void)
{
    GLint value;
    _GLFWwindow* window = _glfwLibrary.currentWindow;

    // Since GLFW doesn't understand screens, we use virtual screen zero

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAAccelerated
                       forVirtualScreen:0];
    window->accelerated = value;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAAlphaSize
                       forVirtualScreen:0];
    window->alphaBits = value;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAColorSize
                       forVirtualScreen:0];

    // It seems that the color size includes the size of the alpha channel so
    // we subtract it before splitting
    _glfwSplitBPP(value - window->alphaBits,
                  &window->redBits,
                  &window->greenBits,
                  &window->blueBits);

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFADepthSize
                       forVirtualScreen:0];
    window->depthBits = value;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAStencilSize
                       forVirtualScreen:0];
    window->stencilBits = value;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAAccumSize
                       forVirtualScreen:0];

    _glfwSplitBPP(value,
                  &window->accumRedBits,
                  &window->accumGreenBits,
                  &window->accumBlueBits);

    // TODO: Figure out what to set this value to
    window->accumAlphaBits = 0;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAAuxBuffers
                       forVirtualScreen:0];
    window->auxBuffers = value;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAStereo
                       forVirtualScreen:0];
    window->stereo = value;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFASamples
                       forVirtualScreen:0];
    window->samples = value;

    // These are forced to false as long as Mac OS X lacks support for OpenGL 3.0+
    window->glForward = GL_FALSE;
    window->glDebug = GL_FALSE;
    window->glProfile = 0;
}

//========================================================================
// Poll for new window and input events
//========================================================================

void _glfwPlatformPollEvents(void)
{
    NSEvent* event;

    do
    {
        event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                   untilDate:[NSDate distantPast]
                                      inMode:NSDefaultRunLoopMode
                                     dequeue:YES];

        if (event)
            [NSApp sendEvent:event];
    }
    while (event);

    [_glfwLibrary.NS.autoreleasePool drain];
    _glfwLibrary.NS.autoreleasePool = [[NSAutoreleasePool alloc] init];
}

//========================================================================
// Wait for new window and input events
//========================================================================

void _glfwPlatformWaitEvents( void )
{
    // I wanted to pass NO to dequeue:, and rely on PollEvents to
    // dequeue and send.  For reasons not at all clear to me, passing
    // NO to dequeue: causes this method never to return.
    NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                        untilDate:[NSDate distantFuture]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    [NSApp sendEvent:event];

    _glfwPlatformPollEvents();
}

//========================================================================
// Hide mouse cursor (lock it)
//========================================================================

void _glfwPlatformHideMouseCursor(_GLFWwindow* window)
{
    [NSCursor hide];
    CGAssociateMouseAndMouseCursorPosition(false);
}

//========================================================================
// Show mouse cursor (unlock it)
//========================================================================

void _glfwPlatformShowMouseCursor(_GLFWwindow* window)
{
    [NSCursor unhide];
    CGAssociateMouseAndMouseCursorPosition(true);
}

//========================================================================
// Set physical mouse cursor position
//========================================================================

void _glfwPlatformSetMouseCursorPos(_GLFWwindow* window, int x, int y)
{
    // The library seems to assume that after calling this the mouse won't move,
    // but obviously it will, and escape the app's window, and activate other apps,
    // and other badness in pain.  I think the API's just silly, but maybe I'm
    // misunderstanding it...

    // Also, (x, y) are window coords...

    // Also, it doesn't seem possible to write this robustly without
    // calculating the maximum y coordinate of all screens, since Cocoa's
    // "global coordinates" are upside down from CG's...

    // Without this (once per app run, but it's convenient to do it here)
    // events will be suppressed for a default of 0.25 seconds after we
    // move the cursor.
    CGSetLocalEventsSuppressionInterval(0.0);

    NSPoint localPoint = NSMakePoint(x, y);
    NSPoint globalPoint = [window->NS.window convertBaseToScreen:localPoint];
    CGPoint mainScreenOrigin = CGDisplayBounds(CGMainDisplayID()).origin;
    double mainScreenHeight = CGDisplayBounds(CGMainDisplayID()).size.height;
    CGPoint targetPoint = CGPointMake(globalPoint.x - mainScreenOrigin.x,
                                      mainScreenHeight - globalPoint.y -
                                          mainScreenOrigin.y);
    CGDisplayMoveCursorToPoint(CGMainDisplayID(), targetPoint);
}

