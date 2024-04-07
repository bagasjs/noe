#include "noe.h"

#include <stdio.h>
#include "noe_internal.h"

static ApplicationConfig CONFIG = {
    .window.width = 800,
    .window.height = 600,
    .window.title = "Noe Window",
    .window.visible = true,
    .window.resizable = false,
    .window.fullScreen = false,
    .window.decorated = true,

    .opengl.version.major = 3,
    .opengl.version.minor = 3,
    .opengl.useCoreProfile = true,
    .opengl.useDebugContext = false,
    .opengl.useOpenglES = false,
    .opengl.useNative = true,
};

static _ApplicationState APP = {0};

#define NOE_REQUIRE_INIT_OR_RETURN_VOID(...)    \
    do {                                        \
        if(!APP.initialized) {                  \
            TRACELOG(LOG_ERROR, __VA_ARGS__);   \
            return;                             \
        }                                       \
    } while(0)

#define NOE_REQUIRE_INIT_OR_RETURN(retval, ...) \
    do {                                        \
        if(!APP.initialized) {                  \
            TRACELOG(LOG_ERROR, __VA_ARGS__);   \
            return (retval);                    \
        }                                       \
    } while(0)

bool initBatchRenderer(const ApplicationConfig *config);
void deinitBatchRenderer(void);

bool InitApplication(void)
{
    if(APP.initialized) return false;
    TRACELOG(LOG_INFO, "Initializing application");

    if(!_InitPlatform(&APP, &CONFIG)) return false;

    TRACELOG(LOG_INFO, "Initializing batch renderer (OpenGL)");
    if(!initBatchRenderer(&CONFIG)) {
        TRACELOG(LOG_FATAL, "Initializing batch renderer failed (OpenGL)");
        return false;
    }
    TRACELOG(LOG_INFO, "Initializing batch renderer success (OpenGL)");

    TRACELOG(LOG_INFO, "Initializing application success");
    APP.initialized = true;
    return true;
}

void DeinitApplication(void)
{
    if(!APP.initialized) return;
#ifndef NOE_PLATFORM_WIN32
    deinitBatchRenderer();
#endif
    _DeinitPlatform(&APP);
}

_ApplicationState *_GetApplicationState(const char *functionName)
{
    NOE_REQUIRE_INIT_OR_RETURN(NULL, "`%s()` requires you to call `InitApplication()`", functionName);
    return &APP;
}

void PollInputEvents(void)
{
    NOE_REQUIRE_INIT_OR_RETURN_VOID("`PollInputEvents()` requires you to call `InitApplication()`");
    if(!APP.initialized) return;
    APP.inputs.keyboard.keyPressedQueueCount = 0;

    for (int i = 0; i < MAXIMUM_KEYBOARD_KEYS; i++)
        APP.inputs.keyboard.previousKeyState[i] = APP.inputs.keyboard.currentKeyState[i];

    for (int i = 0; i < MAXIMUM_MOUSE_BUTTONS; i++) 
        APP.inputs.mouse.previousButtonState[i] = APP.inputs.mouse.currentButtonState[i];

    APP.inputs.mouse.previousWheelMove = APP.inputs.mouse.currentWheelMove;
    APP.inputs.mouse.currentWheelMove = CLITERAL(Vector2){ .x=0.0f, .y=0.0f };

    APP.inputs.mouse.previousPosition = APP.inputs.mouse.currentPosition;

    _PollPlatformEvents(&APP);
}

Vector2 GetCursorPosition(void)
{
    Vector2 result = (Vector2){.x=0.0f, .y=0.0f};
    NOE_REQUIRE_INIT_OR_RETURN(result, "`GetCursorPos()` requires you to call `InitApplication()`");
    result.x = APP.inputs.mouse.currentPosition.x;
    result.y = APP.inputs.mouse.currentPosition.y;
    return result;
}

bool IsFileDropped(void)
{
    return APP.isFileDropped;
}

const char *GetDroppedFile(void)
{
    if(!APP.isFileDropped) return NULL;
    APP.isFileDropped = false;
    return APP.droppedFilePath;
}

bool IsMouseButtonPressed(int button)
{
    NOE_REQUIRE_INIT_OR_RETURN(false, "`IsMouseButtonPressed()` requires you to call `InitApplication()`");
    bool pressed = false;

    if ((APP.inputs.mouse.currentButtonState[button] == 1) && (APP.inputs.mouse.previousButtonState[button] == 0)) 
        pressed = true;

    return pressed;
}

bool IsMouseButtonDown(int button)
{
    NOE_REQUIRE_INIT_OR_RETURN(false, "`IsMouseButtonDown()` requires you to call `InitApplication()`");
    bool down = false;

    if (APP.inputs.mouse.currentButtonState[button] == 1) 
        down = true;

    return down;
}

bool IsMouseButtonReleased(int button)
{
    NOE_REQUIRE_INIT_OR_RETURN(false, "`IsMouseButtonReleased()` requires you to call `InitApplication()`");
    bool released = false;

    if ((APP.inputs.mouse.currentButtonState[button] == 0) && (APP.inputs.mouse.previousButtonState[button] == 1)) 
        released = true;

    return released;
}

bool IsMouseButtonUp(int button)
{
    NOE_REQUIRE_INIT_OR_RETURN(false, "`IsMouseButtonUp()` requires you to call `InitApplication()`");
    bool up = false;

    if (APP.inputs.mouse.currentButtonState[button] == 0) 
        up = true;

    return up;
}

bool IsKeyPressed(int key)
{
    NOE_REQUIRE_INIT_OR_RETURN(false, "`IsKeyPressed()` requires you to call `InitApplication()`");
    bool pressed = false;
    if((key > 0) && (key < MAXIMUM_KEYBOARD_KEYS)) {
        if ((APP.inputs.keyboard.previousKeyState[key] == 0) && (APP.inputs.keyboard.currentKeyState[key] == 1)) 
            pressed = true;
    }
    return pressed;
}

bool IsKeyDown(int key)
{
    NOE_REQUIRE_INIT_OR_RETURN(false, "`IsKeyDown()` requires you to call `InitApplication()`");
    bool down = false;
    if ((key > 0) && (key < MAXIMUM_KEYBOARD_KEYS)) {
        if (APP.inputs.keyboard.currentKeyState[key] == 1) down = true;
    }
    return down;
}

bool IsKeyReleased(int key)
{
    NOE_REQUIRE_INIT_OR_RETURN(false, "`IsKeyReleased()` requires you to call `InitApplication()`");
    bool released = false;
    if ((key > 0) && (key < MAXIMUM_KEYBOARD_KEYS)) {
        if ((APP.inputs.keyboard.previousKeyState[key] == 1) && (APP.inputs.keyboard.currentKeyState[key] == 0)) 
            released = true;
    }
    return released;
}

bool IsKeyUp(int key)
{
    NOE_REQUIRE_INIT_OR_RETURN(false, "`IsKeyUp()` requires you to call `InitApplication()`");
    bool up = false;
    if ((key > 0) && (key < MAXIMUM_KEYBOARD_KEYS)) {
        if (APP.inputs.keyboard.currentKeyState[key] == 0) up = true;
    }
    return up;
}

void SetupOpenGL(uint32_t versionMajor, uint32_t versionMinor, uint32_t flags)
{
    CONFIG.opengl.version.major = versionMajor;
    CONFIG.opengl.version.major = versionMinor;
    (void)flags;
}

void SetupWindow(const char *title, uint32_t width, uint32_t height, uint32_t flags)
{
    CONFIG.window.title = title;
    CONFIG.window.width = width;
    CONFIG.window.height = height;
    if(flags == 0) flags = WINDOW_SETUP_VISIBLE | WINDOW_SETUP_DECORATED;
    CONFIG.window.resizable = FLAG_CHECK(WINDOW_SETUP_RESIZABLE, flags) ? 1 : 0;
    CONFIG.window.fullScreen = FLAG_CHECK(WINDOW_SETUP_FULLSCREEN, flags) ? 1 : 0;
    CONFIG.window.visible = FLAG_CHECK(WINDOW_SETUP_VISIBLE, flags) ? 1 : 0;
    CONFIG.window.decorated = FLAG_CHECK(WINDOW_SETUP_DECORATED, flags) ? 1 : 0;
}

void *MemorySet(void *dst, int value, size_t length)
{
    for(size_t i = 0; i < (length); ++i) 
        CAST(int8_t *, dst)[i] = CAST(int8_t, value);
    return dst;
}

void *MemoryCopy(void *dst, const void *src, size_t length)
{
    for(size_t i = 0; i < (length); ++i) 
        CAST(uint8_t *, dst)[i] = CAST(const uint8_t *, src)[i];
    return dst;
}

int StringFormatV(char *dst, size_t dstSize, const char *fmt, va_list ap)
{
    return vsnprintf(dst, dstSize, fmt, ap);
}

int StringFormat(char *dst, size_t dstSize, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int writtenLength = StringFormatV(dst, dstSize, fmt, ap); 
    va_end(ap);
    return writtenLength;
}

char *StringCopy(char *dst, const char *src, size_t length)
{
    for(size_t i = 0; i < length; ++i)
        dst[i] = src[i];
    return dst;
}

size_t StringLength(const char *str)
{
    size_t i = 0;
    while(str[i++] != '\0');
    return i;
}

const char *StringFind(const char *haystack, const char *needle)
{
    if (!*needle) return (char *)haystack;

    while (*haystack) {
        const char *p1 = haystack;
        const char *p2 = needle;
        while (*p1 && *p2 && *p1 == *p2) {
            p1++;
            p2++;
        }
        if (!*p2) {
            return haystack;
        }
        haystack++;
    }

    return NULL; 
}

const char *GetFileExtension(const char *filePath)
{
    return filePath;
}

const char *GetFileName(const char *filePath)
{
    return filePath;
}

bool CheckColissionPointRec(Vector2 point, Rectangle rect)
{
    return (point.x >= rect.x && point.x <= rect.x + rect.width &&
            point.y >= rect.y && point.y <= rect.y + rect.height);
}
