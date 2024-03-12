#ifndef NOE_PLATFORM_LINUX_H_
#define NOE_PLATFORM_LINUX_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <EGL/egl.h>
#include "noe.h"

typedef struct _PlatformDisplayState {
    Display *handle;
    Screen *defaultScreen;
    int defaultScreenID;
    Window defaultRootWindow;
    int keycodes[256];
    int scancodes[KEY_LAST];

    struct {
        int event_base, error_base;
        int major, minor;
        struct {
            bool EXT_swap_control;
            bool SGI_swap_control;
            bool MESA_swap_control;

            bool ARB_multisample;
            bool ARB_framebuffer_sRGB;
            bool EXT_framebuffer_sRGB;
            bool ARB_create_context;

            bool ARB_create_context_robustness;
            bool ARB_create_context_profile;
            bool EXT_create_context_es2_profile;
            bool ARB_create_context_no_error;
            bool ARB_context_flush_control;

        } extensions;
        struct {
            PFNGLXSWAPINTERVALEXTPROC SwapIntervalEXT;
            PFNGLXSWAPINTERVALSGIPROC SwapIntervalSGI;
            PFNGLXSWAPINTERVALMESAPROC SwapIntervalMESA;
            PFNGLXCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB;
        } api;
        GLXFBConfig fbconfig;
        XVisualInfo *visual_info;
    } glx;

    struct {
    } egl;
} _PlatformDisplayState;

typedef struct _PlatformGLContext {
    bool useGLX;
    struct {
        GLXContext handle;
        GLXWindow window;
    } glx;
} _PlatformGLContext;

typedef struct _PlatformWindowState {
    Window handle;
    Colormap colormap;
    Atom wmDeleteWindow;

    const char *title;
    uint32_t width, height;

    bool visible, resizable, fullScreen;
    bool shouldClose;

    _PlatformGLContext glctx;
} _PlatformWindowState;

typedef struct _PlatformState {
    bool initialized;
    _PlatformDisplayState display;
    _PlatformWindowState window;
} _PlatformState;

#endif // NOE_PLATFORM_LINUX_H_
