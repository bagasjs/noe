#ifndef NOE_PLATFORM_DESKTOP_H_
#define NOE_PLATFORM_DESKTOP_H_

#include "noe.h"

typedef struct GLFWwindow GLFWwindow;

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

#endif // NOE_PLATFORM_DESKTOP_H_
