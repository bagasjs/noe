#include "noe.h"
#include "noe_internal.h"
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>

typedef struct _PlatformState {
    bool initialized;
    struct {
        GLFWwindow *handle;

        const char *title;
        uint32_t width, height;

        bool visible, resizable, fullScreen;
        bool shouldClose;
    } window;
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
    StringFormatV(logMessage, LOG_MESSAGE_MAXIMUM_LENGTH, fmt, ap);
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
    char *result = NOE_MALLOC(sizeof(char) * (filesz + 1));
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
    NOE_FREE(text);
}

uint8_t *LoadFileData(const char *filePath, size_t *fileSize)
{
    FILE *f = fopen(filePath, "rb");
    if(!f) return NULL;

    fseek(f, 0L, SEEK_END);
    size_t filesz = ftell(f);
    fseek(f, 0L, SEEK_SET);
    uint8_t *result = NOE_MALLOC(sizeof(uint8_t) * (filesz + 1));
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
    NOE_FREE(data);
}

static _PlatformState PLATFORM = {0};

static void GlfwFileDropCallback(GLFWwindow *window, int count, const char **paths);
static void GlfwWindowSizeCallback(GLFWwindow *window, int width, int height);
static void GlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void GlfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
static void GlfwMouseCursorPosCallback(GLFWwindow *window, double x, double y);
static void GlfwMouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);

bool _InitPlatform(_ApplicationState *app, ApplicationConfig *config)
{
    TRACELOG(LOG_INFO, "Initializing platform (desktop)");
    if(!glfwInit()) {
        TRACELOG(LOG_FATAL, "Initializing platform failed (desktop)");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config->opengl.version.major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config->opengl.version.minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, config->opengl.useCoreProfile ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, config->window.resizable);
    glfwWindowHint(GLFW_VISIBLE, config->window.visible);
    PLATFORM.window.handle = glfwCreateWindow(config->window.width, config->window.height, 
            config->window.title, NULL, NULL);
    if(!PLATFORM.window.handle) {
        TRACELOG(LOG_FATAL, "Failed to create default window");
        return false;
    }

    glfwSetWindowSizeCallback(PLATFORM.window.handle, GlfwWindowSizeCallback);
    glfwSetKeyCallback(PLATFORM.window.handle, GlfwKeyCallback);
    glfwSetMouseButtonCallback(PLATFORM.window.handle, GlfwMouseButtonCallback);
    glfwSetCursorPosCallback(PLATFORM.window.handle, GlfwMouseCursorPosCallback);
    glfwSetScrollCallback(PLATFORM.window.handle, GlfwMouseScrollCallback);
    glfwSetDropCallback(PLATFORM.window.handle, GlfwFileDropCallback);
    glfwSetWindowUserPointer(PLATFORM.window.handle, app);

    glfwMakeContextCurrent(PLATFORM.window.handle);


    PLATFORM.window.title = config->window.title;
    PLATFORM.window.width = config->window.width;
    PLATFORM.window.height = config->window.height;
    PLATFORM.window.resizable = config->window.resizable;
    PLATFORM.window.visible = config->window.visible;
    PLATFORM.window.fullScreen = config->window.fullScreen;
    PLATFORM.window.shouldClose = false;

    return true;
}

void _DeinitPlatform(_ApplicationState *app)
{
    (void)app;
    TRACELOG(LOG_INFO, "Destroying window (desktop)");
    glfwDestroyWindow(PLATFORM.window.handle);
    TRACELOG(LOG_INFO, "Deinitializing platform (desktop)");
    glfwTerminate();
}

void SetWindowShouldClose(bool shouldClose)
{
    PLATFORM.window.shouldClose = shouldClose;
    glfwSetWindowShouldClose(PLATFORM.window.handle, PLATFORM.window.shouldClose);
}

bool WindowShouldClose(void)
{
    return PLATFORM.window.shouldClose;
}

void _PollPlatformEvents(_ApplicationState *app)
{
    (void)app;
    glfwPollEvents();
    PLATFORM.window.shouldClose = glfwWindowShouldClose(PLATFORM.window.handle);
    glfwSetWindowShouldClose(PLATFORM.window.handle, GLFW_FALSE);
}

void GLSwapBuffers(void)
{
    glfwSwapBuffers(PLATFORM.window.handle);
}

void *GLGetProcAddress(const char *procName)
{
    return (void *)glfwGetProcAddress(procName);
}

bool IsWindowVisible(void)
{
    return PLATFORM.window.visible;
}

bool IsWindowResizable(void)
{
    return PLATFORM.window.resizable;
}

bool IsWindowFullscreen(void)
{
    return PLATFORM.window.fullScreen;
}

static void GlfwWindowSizeCallback(GLFWwindow *window, int width, int height)
{
    (void)window;
    (void)width;
    (void)height;
}

static void GlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;
    if(key < 0) return;
    _ApplicationState *app = (_ApplicationState*)glfwGetWindowUserPointer(window);

    if (action == GLFW_RELEASE) app->inputs.keyboard.currentKeyState[key] = 0;
    else if(action == GLFW_PRESS) app->inputs.keyboard.currentKeyState[key] = 1;

    // Check if there is space available in the key queue
    if ((app->inputs.keyboard.keyPressedQueueCount < MAXIMUM_KEYPRESSED_QUEUE) && (action == GLFW_PRESS)) {
        app->inputs.keyboard.keyPressedQueue[app->inputs.keyboard.keyPressedQueueCount] = key;
        app->inputs.keyboard.keyPressedQueueCount++;
    }
}

static void GlfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    (void)mods;

    _ApplicationState *app= (_ApplicationState*)glfwGetWindowUserPointer(window);
    if (action == GLFW_RELEASE) app->inputs.mouse.currentButtonState[button] = 0;
    else if(action == GLFW_PRESS) app->inputs.mouse.currentButtonState[button] = 1;
}

static void GlfwMouseCursorPosCallback(GLFWwindow *window, double x, double y)
{
    _ApplicationState *state = (_ApplicationState *)glfwGetWindowUserPointer(window);
    state->inputs.mouse.currentPosition.x = (float)x;
    state->inputs.mouse.currentPosition.y = (float)y;
}

static void GlfwFileDropCallback(GLFWwindow *window, int count, const char **paths)
{
    if(count > 0) {
        _ApplicationState *state = (_ApplicationState *)glfwGetWindowUserPointer(window);
        state->isFileDropped = true;
        StringCopy(state->droppedFilePath, paths[0], sizeof(char)*256);
    }
}

static void GlfwMouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    _ApplicationState *state = (_ApplicationState *)glfwGetWindowUserPointer(window);
    state->inputs.mouse.currentWheelMove = CLITERAL(Vector2){ .x=(float)xoffset, .y=(float)yoffset };
}
