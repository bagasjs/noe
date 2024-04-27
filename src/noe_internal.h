#ifndef NOE_INTERNAL_H_
#define NOE_INTERNAL_H_

#include "noe.h"

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

typedef struct _ApplicationState {
    bool initialized;
    _InputManager inputs;

    bool isFileDropped;
    char droppedFilePath[256]; // currently only supporting 1
} _ApplicationState;

_ApplicationState *_GetApplicationState(const char *functionName);


////////////////////////////////////////////////////////////// 
///
/// IMPLEMENT THESE FUNCTIONS ON THEIR RESPECTIVE PLATFORMS
/// WHEN THE PLATFORM DOESN'T SUPPORT THE FUNCTION LOG THE
/// ERROR MESSAGE
///

// All platform should implement
bool _InitPlatform(_ApplicationState *app, ApplicationConfig *config);
void _DeinitPlatform(_ApplicationState *app);
void _PollPlatformEvents(_ApplicationState *app);
void GLSwapBuffers(void);
uint64_t GetTimeMilis(void);
void ExitProgram(int status);
void TraceLog(int logLevel, const char *fmt, ...);
void *MemoryAlloc(size_t nBytes);
void MemoryFree(void *ptr);

// Any platform except the web
char *LoadFileText(const char *filePath, size_t *fileSize);
void UnloadFileText(char *text);
uint8_t *LoadFileData(const char *filePath, size_t *fileSize);
void UnloadFileData(uint8_t *data);

// Desktop only
void SetWindowTitle(const char *title);
void SetWindowSize(uint32_t width, uint32_t height);
void SetWindowVisible(bool isVisible);
void SetWindowResizable(bool isResizable);
void SetWindowFullscreen(bool isFullscreen);

#endif // NOE_INTERNAL_H_
