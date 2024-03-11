#include "noe_internal.h"
#include "noe_platform_desktop.h"
#include <GLFW/glfw3.h>

static void GlfwWindowSizeCallback(GLFWwindow *window, int width, int height);
static void GlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void GlfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
static void GlfwMouseCursorPosCallback(GLFWwindow *window, double x, double y);
static void GlfwMouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);

bool _InitPlatform(_ApplicationState *app, _ApplicationConfig *config)
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
    app->platform.window.handle = glfwCreateWindow(config->window.width, config->window.height, 
            config->window.title, NULL, NULL);
    if(!app->platform.window.handle) {
        TRACELOG(LOG_FATAL, "Failed to create default window");
        return false;
    }

    glfwMakeContextCurrent(app->platform.window.handle);

    glfwSetWindowSizeCallback(app->platform.window.handle, GlfwWindowSizeCallback);
    glfwSetKeyCallback(app->platform.window.handle, GlfwKeyCallback);
    glfwSetMouseButtonCallback(app->platform.window.handle, GlfwMouseButtonCallback);
    glfwSetCursorPosCallback(app->platform.window.handle, GlfwMouseCursorPosCallback);
    glfwSetScrollCallback(app->platform.window.handle, GlfwMouseScrollCallback);
    glfwSetWindowUserPointer(app->platform.window.handle, app);

    app->platform.window.title = config->window.title;
    app->platform.window.width = config->window.width;
    app->platform.window.height = config->window.height;
    app->platform.window.resizable = config->window.resizable;
    app->platform.window.visible = config->window.visible;
    app->platform.window.fullScreen = config->window.fullScreen;
    app->platform.window.shouldClose = false;

    return true;
}

void _DeinitPlatform(_ApplicationState *app)
{
    glfwDestroyWindow(app->platform.window.handle);
    glfwTerminate();
}

void _PollPlatformEvents(_ApplicationState *app)
{
    glfwPollEvents();
    app->platform.window.shouldClose = glfwWindowShouldClose(app->platform.window.handle);
    glfwSetWindowShouldClose(app->platform.window.handle, GLFW_FALSE);
}

void _GLSwapBuffers(_ApplicationState *app)
{
    glfwSwapBuffers(app->platform.window.handle);
}

static void GlfwWindowSizeCallback(GLFWwindow *window, int width, int height)
{
    (void)window;
    (void)width;
    (void)height;
}

static void GlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if(key < 0) return;
    _ApplicationState *app= (_ApplicationState*)glfwGetWindowUserPointer(window);

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
    (void)window;
    (void)button;
    (void)action;
    (void)mods;
}

static void GlfwMouseCursorPosCallback(GLFWwindow *window, double x, double y)
{
    _ApplicationState *state = (_ApplicationState *)glfwGetWindowUserPointer(window);
    state->inputs.mouse.currentPosition.x = (float)x;
    state->inputs.mouse.currentPosition.y = (float)y;
}

static void GlfwMouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    _ApplicationState *state = (_ApplicationState *)glfwGetWindowUserPointer(window);
    state->inputs.mouse.currentWheelMove = CLITERAL(Vector2){ .x=(float)xoffset, .y=(float)yoffset };
}
