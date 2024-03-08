#ifndef NOE_INTERNAL_H_
#define NOE_INTERNAL_H_

#include "noe.h"


/*******************************
 * Internal Types
 *******************************/

/**
 * Internal Application Configuration 
 */
typedef struct _ApplicationConfig {
    struct {
        const char *title;
        uint32_t width, height;
        bool visible;
        bool resizable;
        bool fullScreen;
        bool decorated;
    } window;
    struct {
        // The context creation will use GLX on Linux X11 Display System or WGL on Windows 
        // otherwise It will use EGL
        bool useNative;
        bool useCoreProfile;
        bool useOpenglES;
        bool useDebugContext;
        bool forward;
        struct {
            int major;
            int minor;
        } version;
    } opengl;
} _ApplicationConfig;

/**
 * Internal Input Manager
 */
typedef struct _InputManager {
    struct {
        char currentKeyState[MAXIMUM_KEYBOARD_KEYS];
        char previousKeyState[MAXIMUM_KEYBOARD_KEYS];
        int keyPressedQueue[MAXIMUM_KEYPRESSED_QUEUE];
        int keyPressedQueueCount;
    } keyboard;

    struct {
        char currentButtonState[MAXIMUM_MOUSE_BUTTONS];
        char previousButtonState[MAXIMUM_MOUSE_BUTTONS];

        Vector2 previousPosition;
        Vector2 currentPosition;

        Vector2 currentWheelMove;
        Vector2 previousWheelMove;

        Vector2 offset;
        Vector2 scale;
    } mouse;
} _InputManager;

_InputManager *getApplicationInputManager(void);

#endif // NOE_INTERNAL_H_
