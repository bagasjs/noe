#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <EGL/egl.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>

#include "noe_internal.h"

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

void ExitProgram(int status)
{
    DeinitApplication();
    exit(status);
}

void TraceLog(int logLevel, const char *fmt, ...)
{
    static const char *logLevelsAsText[] = {
        "[FATAL] ",
        "[ERROR] ",
        "[WARNING] ",
        "[INFO] ",
        "[DEBUG] ",
    };

    if(!(LOG_FATAL <= logLevel && logLevel <= LOG_DEBUG)) return;
    char logMessage[LOG_MESSAGE_MAXIMUM_LENGTH] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(logMessage, LOG_MESSAGE_MAXIMUM_LENGTH, fmt, ap);
    va_end(ap);
    switch(logLevel) {
        case LOG_FATAL:
            {
                fprintf(stderr, "%s %s\n", logLevelsAsText[logLevel], logMessage);
                ExitProgram(-1);
            } break;
        case LOG_ERROR:
            {
                fprintf(stderr, "%s %s\n", logLevelsAsText[logLevel], logMessage);
            } break;
        default:
            {
                printf("%s %s\n", logLevelsAsText[logLevel], logMessage);
            } break;
    }
}

void *MemoryAlloc(size_t nBytes)
{
    void *result = malloc(nBytes);
    if(!result) return NULL;
    MemorySet(result, 0, nBytes);
    return result;
}

void MemoryFree(void *ptr)
{
    if(ptr) free(ptr);
}

uint64_t GetTimeMilis(void)
{
    return 0;
}

char *LoadFileText(const char *filePath, size_t *fileSize)
{
    FILE *f = fopen(filePath, "r");
    if(!f) return NULL;

    fseek(f, 0L, SEEK_END);
    size_t filesz = ftell(f);
    fseek(f, 0L, SEEK_SET);
    char *result = MemoryAlloc(sizeof(char) * (filesz + 1));
    if(!result) {
        TraceLog(LOG_ERROR, "Failed to load file `%s` text content", filePath);
        fclose(f);
        return NULL;
    }

    size_t readLength = fread(result, sizeof(char), filesz, f);
    result[readLength] = '\0';
    if(fileSize) *fileSize = readLength;
    fclose(f);
    return result;
}

void UnloadFileText(char *text)
{
    MemoryFree(text);
}

uint8_t *LoadFileData(const char *filePath, size_t *fileSize)
{
    FILE *f = fopen(filePath, "rb");
    if(!f) return NULL;

    fseek(f, 0L, SEEK_END);
    size_t filesz = ftell(f);
    fseek(f, 0L, SEEK_SET);
    uint8_t *result = MemoryAlloc(sizeof(uint8_t) * (filesz + 1));
    if(!result) {
        TraceLog(LOG_ERROR, "Failed to load file `%s` data", filePath);
        fclose(f);
        return NULL;
    }

    size_t readLength = fread(result, sizeof(uint8_t), filesz, f);
    if(fileSize) *fileSize = readLength;
    fclose(f);
    return result;
}

void UnloadFileData(uint8_t *data)
{
    MemoryFree(data);
}


static _PlatformState PLATFORM;

static void setupX11KeyMaps(_PlatformState *platform);

void SetWindowTitle(const char *title)
{
    PLATFORM.window.title = title;
    XStoreName(PLATFORM.display.handle, PLATFORM.window.handle, title);
}

void SetWindowSize(uint32_t width, uint32_t height)
{
    PLATFORM.window.width = width;
    PLATFORM.window.height = height;
}

void SetWindowVisible(bool isVisible)
{
    PLATFORM.window.visible = isVisible;

    if(isVisible) 
        XMapWindow(PLATFORM.display.handle, PLATFORM.window.handle);
    else 
        XUnmapWindow(PLATFORM.display.handle, PLATFORM.window.handle);
}

void SetWindowResizable(bool isResizable)
{
    PLATFORM.window.resizable = isResizable;

    XSizeHints *hints = XAllocSizeHints();
    long supplied;
    XGetWMNormalHints(PLATFORM.display.handle, PLATFORM.window.handle, hints, &supplied);
    hints->flags &= ~(PMinSize | PMaxSize | PAspect);
    if(!isResizable) {
        hints->flags |= (PMinSize | PMaxSize);
        hints->min_width  = hints->max_width  = PLATFORM.window.width;
        hints->min_height = hints->max_height = PLATFORM.window.height;
    }
    XSetWMNormalHints(PLATFORM.display.handle, PLATFORM.window.handle, hints);
    XFree(hints);
    XFlush(PLATFORM.display.handle);
}

void SetWindowFullscreen(bool isFullscreen)
{
    _ApplicationState *app = _GetApplicationState("SetWindowTitle");
    if(!app) return;
    PLATFORM.window.fullScreen = isFullscreen;
}

static bool stringInExtensionString(const char* string, const char* extensions)
{
    const char* start = extensions;

    for (;;)
    {
        const char* where;
        const char* terminator;

        where = StringFind(start, string);
        if (!where)
            return false;

        terminator = where + StringLength(string);
        if (where == start || *(where - 1) == ' ') {
            if (*terminator == ' ' || *terminator == '\0')
                break;
        }

        start = terminator;
    }

    return true;
}

static int isExtensionSupportedGLX(_PlatformState *platform, const char* extension)
{
    const char* extensions = glXQueryExtensionsString(platform->display.handle, platform->display.defaultScreenID);
    if (extensions) {
        if (stringInExtensionString(extension, extensions))
            return true;
    }

    return false;
}

static bool chooseFramebufferConfigGLX(_PlatformState *platform)
{
	GLint glx_attribs[] = {
		GLX_X_RENDERABLE    , True,
		GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
		GLX_RENDER_TYPE     , GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
		GLX_RED_SIZE        , 8,
		GLX_GREEN_SIZE      , 8,
		GLX_BLUE_SIZE       , 8,
		GLX_ALPHA_SIZE      , 8,
		GLX_DEPTH_SIZE      , 24,
		GLX_STENCIL_SIZE    , 8,
		GLX_DOUBLEBUFFER    , True,
		None
	};

	int fb_count;
    GLXFBConfig* fbc = glXChooseFBConfig(platform->display.handle, platform->display.defaultScreenID, glx_attribs, &fb_count);
    if (fbc == 0) {
        TRACELOG(LOG_ERROR, "Failed to retrive GLX Framebuffer");
        return false;
    }

	// Pick the FB config/visual with the most samples per pixel
	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
	for (int i = 0; i < fb_count; ++i) {
		XVisualInfo *vi = glXGetVisualFromFBConfig(platform->display.handle, fbc[i]);
		if (vi != 0) {
			int samp_buf, samples;
			glXGetFBConfigAttrib(platform->display.handle, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
			glXGetFBConfigAttrib(platform->display.handle, fbc[i], GLX_SAMPLES       , &samples);

			if(best_fbc < 0 || (samp_buf && samples > best_num_samp)) {
				best_fbc = i;
				best_num_samp = samples;
			}
			if(worst_fbc < 0 || !samp_buf || samples < worst_num_samp) {
				worst_fbc = i;
            }
			worst_num_samp = samples;
		}
		XFree(vi);
	}

    platform->display.glx.fbconfig = fbc[best_fbc];
	XFree(fbc);
    return true;
}

bool initGLX(_PlatformState *platform)
{
    if (!glXQueryExtension(platform->display.handle, &platform->display.glx.error_base, &platform->display.glx.event_base)) {
        TRACELOG(LOG_ERROR, "GLX: GLX extension not found");
        return false;
    }

    if (!glXQueryVersion(platform->display.handle, &platform->display.glx.major, &platform->display.glx.minor)) {
        TRACELOG(LOG_ERROR, "GLX: Failed to query GLX version");
        return false;
    }

    if (platform->display.glx.major == 1 && platform->display.glx.minor < 3) {
        TRACELOG(LOG_ERROR, "GLX: GLX version 1.3 is required");
        return false;
    }

    if (isExtensionSupportedGLX(platform, "GLX_EXT_swap_control")) {
        platform->display.glx.api.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)
            glXGetProcAddressARB((const GLubyte *)"glXSwapIntervalEXT");

        if (platform->display.glx.api.SwapIntervalEXT)
            platform->display.glx.extensions.EXT_swap_control = true;
    }

    if (isExtensionSupportedGLX(platform, "GLX_SGI_swap_control")) {
        platform->display.glx.api.SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
            glXGetProcAddressARB((const GLubyte *)"glXSwapIntervalSGI");

        if (platform->display.glx.api.SwapIntervalSGI)
            platform->display.glx.extensions.SGI_swap_control = true;
    }

    if (isExtensionSupportedGLX(platform, "GLX_MESA_swap_control")) {
        platform->display.glx.api.SwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)
            glXGetProcAddressARB((const GLubyte *)"glXSwapIntervalMESA");

        if (platform->display.glx.api.SwapIntervalMESA)
            platform->display.glx.extensions.MESA_swap_control = true;
    }

    if (isExtensionSupportedGLX(platform, "GLX_ARB_create_context"))
    {
        platform->display.glx.api.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");

        if (platform->display.glx.api.CreateContextAttribsARB)
            platform->display.glx.extensions.ARB_create_context = true;
    }

    if (isExtensionSupportedGLX(platform, "GLX_ARB_multisample"))
        platform->display.glx.extensions.ARB_multisample = true;
    if (isExtensionSupportedGLX(platform, "GLX_ARB_framebuffer_sRGB"))
        platform->display.glx.extensions.ARB_framebuffer_sRGB = true;
    if (isExtensionSupportedGLX(platform, "GLX_EXT_framebuffer_sRGB"))
        platform->display.glx.extensions.EXT_framebuffer_sRGB = true;
    if (isExtensionSupportedGLX(platform, "GLX_ARB_create_context_robustness"))
        platform->display.glx.extensions.ARB_create_context_robustness = true;
    if (isExtensionSupportedGLX(platform, "GLX_ARB_create_context_profile"))
        platform->display.glx.extensions.ARB_create_context_profile = true;
    if (isExtensionSupportedGLX(platform, "GLX_EXT_create_context_es2_profile"))
        platform->display.glx.extensions.EXT_create_context_es2_profile = true;
    if (isExtensionSupportedGLX(platform, "GLX_ARB_create_context_no_error"))
        platform->display.glx.extensions.ARB_create_context_no_error = true;
    if (isExtensionSupportedGLX(platform, "GLX_ARB_context_flush_control"))
        platform->display.glx.extensions.ARB_context_flush_control = true;

    if(!chooseFramebufferConfigGLX(platform)) {
        return false;
    }

	platform->display.glx.visual_info = glXGetVisualFromFBConfig(platform->display.handle, platform->display.glx.fbconfig);
	if (platform->display.glx.visual_info == 0) {
        TRACELOG(LOG_ERROR, "Could not create correct visual info");
		return false;
	}

	if (platform->display.defaultScreenID != platform->display.glx.visual_info->screen) {
        TRACELOG(LOG_ERROR, "Screen ID doesnt match visual screen ID");
		return false;
	}

    return true;
}

void makeGLContextCurrent(_PlatformDisplayState *display, _PlatformWindowState *window)
{
    if (window) {
        if (!glXMakeCurrent(display->handle, window->glctx.glx.window, window->glctx.glx.handle)) {
            TRACELOG(LOG_FATAL, "Failed to make context current (GLX)");
            return;
        }
    } else {
        if (!glXMakeCurrent(display->handle, None, NULL)) {
            TRACELOG(LOG_FATAL, "Failed to make context current (GLX)");
            return;
        }
    }
}

bool initGLContextGLX(_PlatformState *platform, const ApplicationConfig *config)
{
    GLXContext share = NULL;
    GLXContext result_context = NULL;

    if(config->opengl.useOpenglES) {
        if (!platform->display.glx.extensions.ARB_create_context ||
                !platform->display.glx.extensions.ARB_create_context_profile ||
                !platform->display.glx.extensions.EXT_create_context_es2_profile) {
            TRACELOG(LOG_ERROR, "GLX: OpenGL ES requested but GLX_EXT_create_context_es2_profile is unavailable");
            return false;
        }
    }

    if (config->opengl.forward) {
        if (!platform->display.glx.extensions.ARB_create_context) {
            TRACELOG(LOG_ERROR, "GLX: Forward compatibility requested but GLX_ARB_create_context_profile is unavailable");
            return false;
        }
    }

    if(platform->display.glx.extensions.ARB_create_context) {
        int attribs[40];
#define SET_ATTRIB(a, v) \
        do { \
            if(((size_t)index + 1) < (sizeof(attribs) / sizeof(attribs[0]))) { \
                attribs[index++] = a; \
                attribs[index++] = v; \
            } else { \
                TRACELOG(LOG_FATAL, "Failed to opengl context attribute at %s:%d", __FILE__, __LINE__); \
                return false; \
            } \
        } while(0)

        int index = 0, mask = 0, flags = 0;
        if(config->opengl.useOpenglES) {
            mask |= GLX_CONTEXT_ES2_PROFILE_BIT_EXT;
        } else {
            if (config->opengl.useCoreProfile) mask |= GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
            else mask |= GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
        }

        if (config->opengl.useDebugContext) flags |= GLX_CONTEXT_DEBUG_BIT_ARB;
        if (config->opengl.version.major != 1 || config->opengl.version.minor != 0)
        {
            SET_ATTRIB(GLX_CONTEXT_MAJOR_VERSION_ARB, config->opengl.version.major);
            SET_ATTRIB(GLX_CONTEXT_MINOR_VERSION_ARB, config->opengl.version.minor);
        }

        if (mask) SET_ATTRIB(GLX_CONTEXT_PROFILE_MASK_ARB, mask);
        if (flags) SET_ATTRIB(GLX_CONTEXT_FLAGS_ARB, flags);

        SET_ATTRIB(None, None);
        result_context = platform->display.glx.api.CreateContextAttribsARB(platform->display.handle,
                platform->display.glx.fbconfig,
                share,
                True,
                attribs);

        if (!result_context) {
            TRACELOG(LOG_ERROR, "Failed to create GLX context with ARB");
        }
    } else {
        result_context = glXCreateNewContext(platform->display.handle, 
                platform->display.glx.fbconfig, GLX_RGBA_TYPE, share, True);
    }

    platform->window.glctx.glx.handle = result_context;

    platform->window.glctx.glx.window = glXCreateWindow(platform->display.handle, 
            platform->display.glx.fbconfig, platform->window.handle, NULL);
    if(!platform->window.glctx.glx.window) {
        glXDestroyContext(platform->display.handle, platform->window.glctx.glx.handle);
        return false;
    }

    return true;
}

bool _InitPlatform(_ApplicationState *app, ApplicationConfig *config)
{
    (void)app;
    TRACELOG(LOG_INFO, "Initializing platform (Linux)");
    if(PLATFORM.initialized) {
        TRACELOG(LOG_ERROR, "Initializing platform failed: You have initialize the platform");
        return false;
    }

    TRACELOG(LOG_INFO, "Initializing display system (X11)");
    Display *dpy = XOpenDisplay(NULL);

    if(!dpy) {
        TRACELOG(LOG_FATAL, "Failed to initialize native display system (X11)");
        return false;
    }
    
    PLATFORM.display.defaultScreen = XDefaultScreenOfDisplay(dpy);
    PLATFORM.display.defaultScreenID = XDefaultScreen(dpy);
    PLATFORM.display.defaultRootWindow = XRootWindow(dpy, PLATFORM.display.defaultScreenID);
    PLATFORM.display.handle = dpy;
    setupX11KeyMaps(&PLATFORM);

    if(config->opengl.useNative) {
        if(!initGLX(&PLATFORM)) {
            XCloseDisplay(PLATFORM.display.handle);
            TRACELOG(LOG_INFO, "Failed to initialize GLX");
            return false;
        }
    } else {
        // EGL initialization
    }

    TRACELOG(LOG_INFO, "Creating window (X11)");
    Window handle;
    Colormap colormap = XCreateColormap(PLATFORM.display.handle, PLATFORM.display.defaultRootWindow, 
            XDefaultVisualOfScreen(PLATFORM.display.defaultScreen), AllocNone);
    Atom wmDeleteWindow = XInternAtom(PLATFORM.display.handle, "WM_DELETE_WINDOW", False);

    XSetWindowAttributes swa;
    swa.override_redirect = True;
    swa.border_pixel = None;
    swa.background_pixel = XBlackPixel(PLATFORM.display.handle, PLATFORM.display.defaultScreenID);
    swa.colormap = colormap;
    swa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                    PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                    ExposureMask | FocusChangeMask | VisibilityChangeMask |
                    EnterWindowMask | LeaveWindowMask | PropertyChangeMask;

    handle = XCreateWindow(PLATFORM.display.handle, PLATFORM.display.defaultRootWindow,
            0, 0, config->window.width, config->window.height, 0,
            CopyFromParent, InputOutput,
            CopyFromParent, 
            CWBackPixel | CWColormap | CWBorderPixel | CWEventMask,
            &swa);

    if(!handle) {
        TRACELOG(LOG_FATAL, "Failed to create native window (X11)");
        return false;
    }

    XSetWMProtocols(PLATFORM.display.handle, handle, &wmDeleteWindow, 1);

    PLATFORM.window.handle = handle;
    PLATFORM.window.colormap = colormap;
    PLATFORM.window.wmDeleteWindow = wmDeleteWindow;
    PLATFORM.window.glctx.useGLX = config->opengl.useNative;
    PLATFORM.window.title = config->window.title;
    PLATFORM.window.width = config->window.width;
    PLATFORM.window.height = config->window.height;
    PLATFORM.window.visible = config->window.visible;
    PLATFORM.window.resizable = config->window.resizable;
    PLATFORM.window.fullScreen = config->window.fullScreen;
    PLATFORM.window.shouldClose = false;

    // First setup
    XStoreName(PLATFORM.display.handle, PLATFORM.window.handle, config->window.title);
    if(config->window.visible) 
        XMapWindow(PLATFORM.display.handle, PLATFORM.window.handle);
    else 
        XUnmapWindow(PLATFORM.display.handle, PLATFORM.window.handle);
    XSizeHints *hints = XAllocSizeHints();
    long supplied;
    XGetWMNormalHints(PLATFORM.display.handle, PLATFORM.window.handle, hints, &supplied);
    hints->flags &= ~(PMinSize | PMaxSize | PAspect);
    if(!config->window.resizable) {
        hints->flags |= (PMinSize | PMaxSize);
        hints->min_width  = hints->max_width  = PLATFORM.window.width;
        hints->min_height = hints->max_height = PLATFORM.window.height;
    }
    XSetWMNormalHints(PLATFORM.display.handle, PLATFORM.window.handle, hints);
    XFree(hints);
    XFlush(PLATFORM.display.handle);

    // HACK: Wait and skip the next event to make sure the WM receive the notification
    while(!XPending(PLATFORM.display.handle));
    XEvent event = {0};
    XNextEvent(PLATFORM.display.handle, &event);

    TRACELOG(LOG_INFO, "Initializing display system success (X11)");

    if(config->opengl.useNative) {
        if(!initGLContextGLX(&PLATFORM, config)) {
            XCloseDisplay(PLATFORM.display.handle);
            TRACELOG(LOG_INFO, "Failed to initialize GLX");
            return false;
        }
    } else {
        // EGL initialization
    }

    makeGLContextCurrent(&PLATFORM.display, &PLATFORM.window);

    PLATFORM.initialized = true;
    TRACELOG(LOG_INFO, "Initializing platform success (Linux)");

    return true;
}

void _DeinitPlatform(_ApplicationState *app)
{
    (void)app;
    if(!PLATFORM.initialized) return;

    if(PLATFORM.window.glctx.useGLX) {
        glXDestroyWindow(PLATFORM.display.handle, PLATFORM.window.glctx.glx.window);
        glXDestroyContext(PLATFORM.display.handle, PLATFORM.window.glctx.glx.handle);
    } else {
    }

    XFreeColormap(PLATFORM.display.handle, PLATFORM.window.colormap);
    XDestroyWindow(PLATFORM.display.handle, PLATFORM.window.handle);
    TRACELOG(LOG_INFO, "Destroying native window (X11)");

    XCloseDisplay(PLATFORM.display.handle);
    TRACELOG(LOG_INFO, "Deinitializing platform (Linux)");
}


void GLSwapBuffers(void)
{
    if(PLATFORM.window.glctx.useGLX)
        glXSwapBuffers(PLATFORM.display.handle, PLATFORM.window.glctx.glx.window);
}

static int translateX11Key(_PlatformState *platform, int scancode) {
    if (scancode < 0 || scancode > 255)
        return KEY_INVALID;

    return platform->display.keycodes[scancode];
}

static int translateX11KeyState(int state) {
    int mods = 0;

    if (state & ShiftMask)
        mods |= KEY_MOD_SHIFT;
    if (state & ControlMask)
        mods |= KEY_MOD_CONTROL;
    if (state & Mod1Mask)
        mods |= KEY_MOD_ALT;
    if (state & Mod4Mask)
        mods |= KEY_MOD_SUPER;
    if (state & LockMask)
        mods |= KEY_MOD_CAPSLOCK;
    if (state & Mod2Mask)
        mods |= KEY_MOD_NUMLOCK;

    return mods;
}

void SetWindowShouldClose(bool shouldClose)
{
    PLATFORM.window.shouldClose = shouldClose;
}

bool WindowShouldClose(void)
{
    return PLATFORM.window.shouldClose;
}

void _PollPlatformEvents(_ApplicationState *app)
{
    XEvent event = {0};
    while(XPending(PLATFORM.display.handle) > 0) {
        XNextEvent(PLATFORM.display.handle, &event);
        int scancode = 0;

        // HACK: Save scancode as some IMs clear the field in XFilterEvent
        if (event.type == KeyPress || event.type == KeyRelease) scancode = event.xkey.keycode;

        bool filtered = False;
        filtered = XFilterEvent(&event, None);
        (void)filtered;

        switch(event.type) {
            case ClientMessage:
                {
                    if((Atom)event.xclient.data.l[0] == PLATFORM.window.wmDeleteWindow) 
                        SetWindowShouldClose(true);
                } break;
            case KeyPress:
            case KeyRelease:
                {
                    const int mods = translateX11KeyState(event.xkey.state);
                    (void)mods;
                    const int keycode = translateX11Key(&PLATFORM, scancode);
                    if(keycode < 0) return;
                    if (event.type == KeyPress) app->inputs.keyboard.currentKeyState[keycode] = 1;
                    else if(event.type == KeyRelease) app->inputs.keyboard.currentKeyState[keycode] = 0;

                    // Check if there is space available in the key queue
                    if ((app->inputs.keyboard.keyPressedQueueCount < MAXIMUM_KEYPRESSED_QUEUE) && (event.type == KeyPress)) {
                        app->inputs.keyboard.keyPressedQueue[app->inputs.keyboard.keyPressedQueueCount] = keycode;
                        app->inputs.keyboard.keyPressedQueueCount++;
                    }
                } break;
            case ButtonPress:
            case ButtonRelease:
                {
                } break;
            default:
                break;
        }
    }
}

static int translateKeySyms(const KeySym* keysyms, int width)
{
    if (width > 1) {
        switch (keysyms[1]) {
            case XK_KP_0:           return KEY_KP_0;
            case XK_KP_1:           return KEY_KP_1;
            case XK_KP_2:           return KEY_KP_2;
            case XK_KP_3:           return KEY_KP_3;
            case XK_KP_4:           return KEY_KP_4;
            case XK_KP_5:           return KEY_KP_5;
            case XK_KP_6:           return KEY_KP_6;
            case XK_KP_7:           return KEY_KP_7;
            case XK_KP_8:           return KEY_KP_8;
            case XK_KP_9:           return KEY_KP_9;
            case XK_KP_Separator:
            case XK_KP_Decimal:     return KEY_KP_DECIMAL;
            case XK_KP_Equal:       return KEY_KP_EQUAL;
            case XK_KP_Enter:       return KEY_KP_ENTER;
            default:                break;
        }
    }

    switch (keysyms[0]) {
        case XK_Escape:         return KEY_ESCAPE;
        case XK_Tab:            return KEY_TAB;
        case XK_Shift_L:        return KEY_LEFT_SHIFT;
        case XK_Shift_R:        return KEY_RIGHT_SHIFT;
        case XK_Control_L:      return KEY_LEFT_CONTROL;
        case XK_Control_R:      return KEY_RIGHT_CONTROL;
        case XK_Meta_L:
        case XK_Alt_L:          return KEY_LEFT_ALT;
        case XK_Mode_switch: // Mapped to Alt_R on many keyboards
        case XK_ISO_Level3_Shift: // AltGr on at least some machines
        case XK_Meta_R:
        case XK_Alt_R:          return KEY_RIGHT_ALT;
        case XK_Super_L:        return KEY_LEFT_SUPER;
        case XK_Super_R:        return KEY_RIGHT_SUPER;
        case XK_Menu:           return KEY_MENU;
        case XK_Num_Lock:       return KEY_NUM_LOCK;
        case XK_Caps_Lock:      return KEY_CAPS_LOCK;
        case XK_Print:          return KEY_PRINT_SCREEN;
        case XK_Scroll_Lock:    return KEY_SCROLL_LOCK;
        case XK_Pause:          return KEY_PAUSE;
        case XK_Delete:         return KEY_DELETE;
        case XK_BackSpace:      return KEY_BACKSPACE;
        case XK_Return:         return KEY_ENTER;
        case XK_Home:           return KEY_HOME;
        case XK_End:            return KEY_END;
        case XK_Page_Up:        return KEY_PAGE_UP;
        case XK_Page_Down:      return KEY_PAGE_DOWN;
        case XK_Insert:         return KEY_INSERT;
        case XK_Left:           return KEY_LEFT;
        case XK_Right:          return KEY_RIGHT;
        case XK_Down:           return KEY_DOWN;
        case XK_Up:             return KEY_UP;
        case XK_F1:             return KEY_F1;
        case XK_F2:             return KEY_F2;
        case XK_F3:             return KEY_F3;
        case XK_F4:             return KEY_F4;
        case XK_F5:             return KEY_F5;
        case XK_F6:             return KEY_F6;
        case XK_F7:             return KEY_F7;
        case XK_F8:             return KEY_F8;
        case XK_F9:             return KEY_F9;
        case XK_F10:            return KEY_F10;
        case XK_F11:            return KEY_F11;
        case XK_F12:            return KEY_F12;
        case XK_F13:            return KEY_F13;
        case XK_F14:            return KEY_F14;
        case XK_F15:            return KEY_F15;
        case XK_F16:            return KEY_F16;
        case XK_F17:            return KEY_F17;
        case XK_F18:            return KEY_F18;
        case XK_F19:            return KEY_F19;
        case XK_F20:            return KEY_F20;
        case XK_F21:            return KEY_F21;
        case XK_F22:            return KEY_F22;
        case XK_F23:            return KEY_F23;
        case XK_F24:            return KEY_F24;
        case XK_F25:            return KEY_F25;

        // Numeric keypad
        case XK_KP_Divide:      return KEY_KP_DIVIDE;
        case XK_KP_Multiply:    return KEY_KP_MULTIPLY;
        case XK_KP_Subtract:    return KEY_KP_SUBTRACT;
        case XK_KP_Add:         return KEY_KP_ADD;

        // These should have been detected in secondary keysym test above!
        case XK_KP_Insert:      return KEY_KP_0;
        case XK_KP_End:         return KEY_KP_1;
        case XK_KP_Down:        return KEY_KP_2;
        case XK_KP_Page_Down:   return KEY_KP_3;
        case XK_KP_Left:        return KEY_KP_4;
        case XK_KP_Right:       return KEY_KP_6;
        case XK_KP_Home:        return KEY_KP_7;
        case XK_KP_Up:          return KEY_KP_8;
        case XK_KP_Page_Up:     return KEY_KP_9;
        case XK_KP_Delete:      return KEY_KP_DECIMAL;
        case XK_KP_Equal:       return KEY_KP_EQUAL;
        case XK_KP_Enter:       return KEY_KP_ENTER;

        // Last resort: Check for printable keys (should not happen if the XKB
        // extension is available). This will give a layout dependent mapping
        // (which is wrong, and we may miss some keys, especially on non-US
        // keyboards), but it's better than nothing...
        case XK_a:              return KEY_A;
        case XK_b:              return KEY_B;
        case XK_c:              return KEY_C;
        case XK_d:              return KEY_D;
        case XK_e:              return KEY_E;
        case XK_f:              return KEY_F;
        case XK_g:              return KEY_G;
        case XK_h:              return KEY_H;
        case XK_i:              return KEY_I;
        case XK_j:              return KEY_J;
        case XK_k:              return KEY_K;
        case XK_l:              return KEY_L;
        case XK_m:              return KEY_M;
        case XK_n:              return KEY_N;
        case XK_o:              return KEY_O;
        case XK_p:              return KEY_P;
        case XK_q:              return KEY_Q;
        case XK_r:              return KEY_R;
        case XK_s:              return KEY_S;
        case XK_t:              return KEY_T;
        case XK_u:              return KEY_U;
        case XK_v:              return KEY_V;
        case XK_w:              return KEY_W;
        case XK_x:              return KEY_X;
        case XK_y:              return KEY_Y;
        case XK_z:              return KEY_Z;
        case XK_1:              return KEY_1;
        case XK_2:              return KEY_2;
        case XK_3:              return KEY_3;
        case XK_4:              return KEY_4;
        case XK_5:              return KEY_5;
        case XK_6:              return KEY_6;
        case XK_7:              return KEY_7;
        case XK_8:              return KEY_8;
        case XK_9:              return KEY_9;
        case XK_0:              return KEY_0;
        case XK_space:          return KEY_SPACE;
        case XK_minus:          return KEY_MINUS;
        case XK_equal:          return KEY_EQUAL;
        case XK_bracketleft:    return KEY_LEFT_BRACKET;
        case XK_bracketright:   return KEY_RIGHT_BRACKET;
        case XK_backslash:      return KEY_BACKSLASH;
        case XK_semicolon:      return KEY_SEMICOLON;
        case XK_apostrophe:     return KEY_APOSTROPHE;
        case XK_grave:          return KEY_GRAVE_ACCENT;
        case XK_comma:          return KEY_COMMA;
        case XK_period:         return KEY_PERIOD;
        case XK_slash:          return KEY_SLASH;
        case XK_less:           return KEY_WORLD_1; // At least in some layouts...
        default:                break;
    }

    // No matching translation was found
    return KEY_INVALID;
}

static void setupX11KeyMaps(_PlatformState *platform)
{
    int scancodeMin, scancodeMax;

    MemorySet(platform->display.keycodes, -1, sizeof(platform->display.keycodes));
    MemorySet(platform->display.scancodes, -1, sizeof(platform->display.scancodes));

    XDisplayKeycodes(platform->display.handle, &scancodeMin, &scancodeMax);

    int width;
    KeySym* keysyms = XGetKeyboardMapping(platform->display.handle,
                                          scancodeMin,
                                          scancodeMax - scancodeMin + 1,
                                          &width);

    for (int scancode = scancodeMin;  scancode <= scancodeMax;  scancode++) {
        // Translate the un-translated key codes using traditional X11 KeySym
        // lookups
        if (platform->display.keycodes[scancode] < 0) {
            const size_t base = (scancode - scancodeMin) * width;
            platform->display.keycodes[scancode] = translateKeySyms(&keysyms[base], width);
        }

        // Store the reverse translation for faster key name lookup
        if (platform->display.keycodes[scancode] > 0)
            platform->display.scancodes[platform->display.keycodes[scancode]] = scancode;
    }

    XFree(keysyms);
}
