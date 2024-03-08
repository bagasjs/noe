#define NOE_SAFE_WIN32_INCLUDE
#include "noe.h"
#include "noe_internal.h"

#include "windows.h"

typedef struct _PlatformWindowState {
    HWND hWnd;
} _PlatformWindowState;

typedef struct _PlatformDisplayState {
    HINSTANCE hInstance;
} _PlatformDisplayState;

typedef struct _PlatformState {
    bool initialized;
    _PlatformDisplayState display;
    _PlatformWindowState window;
} _PlatformState;

static _PlatformState PLATFORM = {0};

LRESULT CALLBACK win32WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void platformSetWindowTitle(const char *title)
{
    (void)title;
}

void platformSetWindowSize(uint32_t width, uint32_t height)
{
    (void)width;
    (void)height;
}

void platformSetWindowVisible(bool isVisible)
{
    int visibilityFlag = isVisible ? SW_SHOWNA : SW_HIDE;
    ShowWindow(PLATFORM.window.hWnd, visibilityFlag);
}

void platformSetWindowResizable(bool isResizable)
{
    (void)isResizable;
}
void platformSetWindowFullscreen(bool isFullscreen)
{
    (void)isFullscreen;
}

bool platformInit(const _ApplicationConfig *config)
{
    TRACELOG(LOG_INFO, "Initializing platform (Win32)");
    if(PLATFORM.initialized) {
        TRACELOG(LOG_ERROR, "Initializing platform failed: You have initialize the platform");
        return false;
    }

    PLATFORM.display.hInstance = GetModuleHandleA(NULL);
    if(!PLATFORM.display.hInstance) {
        TRACELOG(LOG_FATAL, "Failed to get the hInstance of this application (Win32)");
        return false;
    }

    WNDCLASSA wc;
    MemorySet(&wc, 0, sizeof(WNDCLASSA));
    wc.lpszClassName = "NoeEngineWindow";
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = win32WindowProc;
    wc.hInstance = PLATFORM.display.hInstance;
    wc.hIcon = LoadIcon(PLATFORM.display.hInstance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    
    ATOM windowClass = RegisterClassA(&wc);
    if(!windowClass) {
        TRACELOG(LOG_FATAL, "Failed to register window class (Win32)");
        return false;
    }

    RECT wr;
    wr.right = config->window.width;
    wr.bottom = config->window.height;
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    HWND handle = CreateWindowExA(
            WS_EX_APPWINDOW,
            MAKEINTATOM(windowClass),
            config->window.title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            wr.right - wr.left,
            wr.bottom - wr.top,
            NULL,
            NULL,
            PLATFORM.display.hInstance,
            NULL);
    if(!handle) {
        TRACELOG(LOG_FATAL, "Failed to create window (Win32)");
        return false;
    }
        
    PLATFORM.window.hWnd = handle;

    platformSetWindowVisible(true);

    PLATFORM.initialized = true;
    TRACELOG(LOG_INFO, "Initializing platform success (Win32)");
    return true;
}

void platformDeinit(void)
{
    TRACELOG(LOG_INFO, "Deinitializing platform (Win32)");
    DestroyWindow(PLATFORM.window.hWnd);
}

void platformPollEvent(_InputManager *inputs)
{
    MSG msg;

    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if(msg.message == WM_QUIT) {
            SetWindowShouldClose(true);
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void SwapBufferGL(void)
{
}

int win32GetKeyMods(void)
{
    int mods = 0;

    if (GetKeyState(VK_SHIFT) & 0x8000)
        mods |= KEY_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        mods |= KEY_MOD_CONTROL;
    if (GetKeyState(VK_MENU) & 0x8000)
        mods |= KEY_MOD_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
        mods |= KEY_MOD_SUPER;
    if (GetKeyState(VK_CAPITAL) & 1)
        mods |= KEY_MOD_CAPSLOCK;
    if (GetKeyState(VK_NUMLOCK) & 1)
        mods |= KEY_MOD_NUMLOCK;

    return mods;
}

int win32Scancode2Keycode(int scancode)
{
    switch(scancode) {
        case 0x00B: return KEY_0;
        case 0x002: return KEY_1;
        case 0x003: return KEY_2;
        case 0x004: return KEY_3;
        case 0x005: return KEY_4;
        case 0x006: return KEY_5;
        case 0x007: return KEY_6;
        case 0x008: return KEY_7;
        case 0x009: return KEY_8;
        case 0x00A: return KEY_9;
        case 0x01E: return KEY_A;
        case 0x030: return KEY_B;
        case 0x02E: return KEY_C;
        case 0x020: return KEY_D;
        case 0x012: return KEY_E;
        case 0x021: return KEY_F;
        case 0x022: return KEY_G;
        case 0x023: return KEY_H;
        case 0x017: return KEY_I;
        case 0x024: return KEY_J;
        case 0x025: return KEY_K;
        case 0x026: return KEY_L;
        case 0x032: return KEY_M;
        case 0x031: return KEY_N;
        case 0x018: return KEY_O;
        case 0x019: return KEY_P;
        case 0x010: return KEY_Q;
        case 0x013: return KEY_R;
        case 0x01F: return KEY_S;
        case 0x014: return KEY_T;
        case 0x016: return KEY_U;
        case 0x02F: return KEY_V;
        case 0x011: return KEY_W;
        case 0x02D: return KEY_X;
        case 0x015: return KEY_Y;
        case 0x02C: return KEY_Z;
        case 0x028: return KEY_APOSTROPHE;
        case 0x02B: return KEY_BACKSLASH;
        case 0x033: return KEY_COMMA;
        case 0x00D: return KEY_EQUAL;
        case 0x029: return KEY_GRAVE_ACCENT;
        case 0x01A: return KEY_LEFT_BRACKET;
        case 0x00C: return KEY_MINUS;
        case 0x034: return KEY_PERIOD;
        case 0x01B: return KEY_RIGHT_BRACKET;
        case 0x027: return KEY_SEMICOLON;
        case 0x035: return KEY_SLASH;
        case 0x056: return KEY_WORLD_2;
        case 0x00E: return KEY_BACKSPACE;
        case 0x153: return KEY_DELETE;
        case 0x14F: return KEY_END;
        case 0x01C: return KEY_ENTER;
        case 0x001: return KEY_ESCAPE;
        case 0x147: return KEY_HOME;
        case 0x152: return KEY_INSERT;
        case 0x15D: return KEY_MENU;
        case 0x151: return KEY_PAGE_DOWN;
        case 0x149: return KEY_PAGE_UP;
        case 0x045: return KEY_PAUSE;
        case 0x039: return KEY_SPACE;
        case 0x00F: return KEY_TAB;
        case 0x03A: return KEY_CAPS_LOCK;
        case 0x145: return KEY_NUM_LOCK;
        case 0x046: return KEY_SCROLL_LOCK;
        case 0x03B: return KEY_F1;
        case 0x03C: return KEY_F2;
        case 0x03D: return KEY_F3;
        case 0x03E: return KEY_F4;
        case 0x03F: return KEY_F5;
        case 0x040: return KEY_F6;
        case 0x041: return KEY_F7;
        case 0x042: return KEY_F8;
        case 0x043: return KEY_F9;
        case 0x044: return KEY_F10;
        case 0x057: return KEY_F11;
        case 0x058: return KEY_F12;
        case 0x064: return KEY_F13;
        case 0x065: return KEY_F14;
        case 0x066: return KEY_F15;
        case 0x067: return KEY_F16;
        case 0x068: return KEY_F17;
        case 0x069: return KEY_F18;
        case 0x06A: return KEY_F19;
        case 0x06B: return KEY_F20;
        case 0x06C: return KEY_F21;
        case 0x06D: return KEY_F22;
        case 0x06E: return KEY_F23;
        case 0x076: return KEY_F24;
        case 0x038: return KEY_LEFT_ALT;
        case 0x01D: return KEY_LEFT_CONTROL;
        case 0x02A: return KEY_LEFT_SHIFT;
        case 0x15B: return KEY_LEFT_SUPER;
        case 0x137: return KEY_PRINT_SCREEN;
        case 0x138: return KEY_RIGHT_ALT;
        case 0x11D: return KEY_RIGHT_CONTROL;
        case 0x036: return KEY_RIGHT_SHIFT;
        case 0x15C: return KEY_RIGHT_SUPER;
        case 0x150: return KEY_DOWN;
        case 0x14B: return KEY_LEFT;
        case 0x14D: return KEY_RIGHT;
        case 0x148: return KEY_UP;
        case 0x052: return KEY_KP_0;
        case 0x04F: return KEY_KP_1;
        case 0x050: return KEY_KP_2;
        case 0x051: return KEY_KP_3;
        case 0x04B: return KEY_KP_4;
        case 0x04C: return KEY_KP_5;
        case 0x04D: return KEY_KP_6;
        case 0x047: return KEY_KP_7;
        case 0x048: return KEY_KP_8;
        case 0x049: return KEY_KP_9;
        case 0x04E: return KEY_KP_ADD;
        case 0x053: return KEY_KP_DECIMAL;
        case 0x135: return KEY_KP_DIVIDE;
        case 0x11C: return KEY_KP_ENTER;
        case 0x059: return KEY_KP_EQUAL;
        case 0x037: return KEY_KP_MULTIPLY;
        case 0x04A: return KEY_KP_SUBTRACT;
        default: return KEY_INVALID;
    }
    return KEY_INVALID;
}

LRESULT CALLBACK win32WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    _InputManager *inputs = getApplicationInputManager();
    if(!inputs) return DefWindowProc(hWnd, uMsg, wParam, lParam);

    switch(uMsg){
        case WM_QUIT:
            {
                PostQuitMessage(0);
                return 0;
            } break;
        case WM_SIZE:
            {
                RECT r;
                GetClientRect(hWnd, &r);
                int window_width = r.right - r.left;
                int window_height= r.bottom - r.top;
            } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            {
                const int released = 0;
                const int pressed = 1;
                int key, scancode;
                const int action = (HIWORD(lParam) & KF_UP) ? released : pressed;
                const int mods = win32GetKeyMods();

                scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
                if(!scancode) {
                    scancode = MapVirtualKeyA((UINT)wParam, MAPVK_VK_TO_VSC);
                }

                // HACK: Alt+PrtSc has a different scancode than just PrtSc
                if (scancode == 0x54)
                    scancode = 0x137;

                // HACK: Ctrl+Pause has a different scancode than just Pause
                if (scancode == 0x146)
                    scancode = 0x45;

                // HACK: CJK IME sets the extended bit for right Shift
                if (scancode == 0x136)
                    scancode = 0x36;

                key = win32Scancode2Keycode(scancode);

                // The Ctrl keys require special handling
                if (wParam == VK_CONTROL) {
                    if (HIWORD(lParam) & KF_EXTENDED) {
                        // Right side keys have the extended key bit set
                        key = KEY_RIGHT_CONTROL;
                    } else {
                        // NOTE: Alt Gr sends Left Ctrl followed by Right Alt
                        // HACK: We only want one event for Alt Gr, so if we detect
                        //       this sequence we discard this Left Ctrl message now
                        //       and later report Right Alt normally
                        MSG next;
                        const DWORD time = GetMessageTime();

                        if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE))
                        {
                            if (next.message == WM_KEYDOWN ||
                                    next.message == WM_SYSKEYDOWN ||
                                    next.message == WM_KEYUP ||
                                    next.message == WM_SYSKEYUP)
                            {
                                if (next.wParam == VK_MENU &&
                                        (HIWORD(next.lParam) & KF_EXTENDED) &&
                                        next.time == time)
                                {
                                    // Next message is Right Alt down so discard this
                                    break;
                                }
                            }
                        }

                        // This is a regular Left Ctrl message
                        key = KEY_LEFT_CONTROL;
                    }
                } else if (wParam == VK_PROCESSKEY) {
                    // IME notifies that keys have been filtered by setting the
                    // virtual key-code to VK_PROCESSKEY
                    break;
                }

                if (action == pressed && wParam == VK_SHIFT) {
                    // HACK: Release both Shift keys on Shift up event, as when both
                    //       are pressed the first release does not emit any event
                    // NOTE: The other half of this is in _glfwPollEventsWin32

                    // _zInputKey(eq, ZKEY_LEFT_SHIFT, scancode, action, mods);
                    // _zInputKey(eq, ZKEY_RIGHT_SHIFT, scancode, action, mods);
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
                } else if (wParam == VK_SNAPSHOT) {
                    // HACK: Key down is not reported for the Print Screen key
                    // _zInputKey(eq, key, scancode, ZEVENT_KEY_PRESSED, mods);
                    // _zInputKey(eq, key, scancode, ZEVENT_KEY_RELEASED, mods);
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
                } else {
                    if (action == pressed) inputs->keyboard.currentKeyState[key] = 1;
                    else if(action == released) inputs->keyboard.currentKeyState[key] = 0;

                    // Check if there is space available in the key queue
                    if ((inputs->keyboard.keyPressedQueueCount < MAXIMUM_KEYPRESSED_QUEUE) && (action == pressed)) {
                        inputs->keyboard.keyPressedQueue[inputs->keyboard.keyPressedQueueCount] = key;
                        inputs->keyboard.keyPressedQueueCount++;
                    }
                }
            } break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#define LOG_MESSAGE_MAXIMUM_LENGTH (32*1024)
static const char *logLevelsAsText[] = {
    "[INFO] ",
    "[WARNING] ",
    "[ERROR] ",
    "[FATAL] ",
};

void platformConsoleWrite(const char *message, BYTE colour) { 
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(consoleHandle, &csbi);

    // INFO,WARN,ERROR,FATAL
    static BYTE levels[] = { 2, 6, 4, 64 };
    SetConsoleTextAttribute(consoleHandle, levels[colour]);
    OutputDebugStringA(message);
    DWORD length = strlen(message);
    DWORD numberWritten = 0;
    WriteConsoleA(consoleHandle, message, (DWORD)length, &numberWritten, 0);
    SetConsoleTextAttribute(consoleHandle, csbi.wAttributes);
}

void platformConsoleWriteError(const char *message, BYTE colour) {
    HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi); 

    // INFO,WARN,ERROR,FATAL
    static BYTE levels[] = { 2, 6, 4, 64 };
    SetConsoleTextAttribute(consoleHandle, levels[colour]);
    OutputDebugStringA(message);
    DWORD length = strlen(message);
    DWORD numberWritten = 0;
    WriteConsoleA(consoleHandle, message, (DWORD)length, &numberWritten, 0);

    SetConsoleTextAttribute(consoleHandle, csbi.wAttributes);
}

void TraceLog(int logLevel, const char *fmt, ...)
{
    if(!(LOG_INFO <= logLevel && logLevel <= LOG_FATAL)) return;
    char logMessage[LOG_MESSAGE_MAXIMUM_LENGTH];
    char result[LOG_MESSAGE_MAXIMUM_LENGTH];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(logMessage, LOG_MESSAGE_MAXIMUM_LENGTH, fmt, ap);
    snprintf(result, LOG_MESSAGE_MAXIMUM_LENGTH, "%s %s\n", logLevelsAsText[logLevel], logMessage);
    va_end(ap);
    if(logLevel == LOG_FATAL || logLevel == LOG_ERROR) platformConsoleWriteError(result, logLevel);
    else platformConsoleWrite(result, logLevel);
}

#define INV_HANDLE(x)       (((x) == NULL) || ((x) == INVALID_HANDLE_VALUE))
void *MemoryAlloc(size_t nBytes)
{
    void *result = VirtualAllocEx(
        GetCurrentProcess(),      /* Allocate in current process address space */
        NULL,                     /* Unknown position */
        nBytes,               /* Bytes to allocate */
        MEM_COMMIT | MEM_RESERVE, /* Reserve and commit allocated page */
        PAGE_READWRITE            /* Permissions ( Read/Write )*/
    );
    if(INV_HANDLE(result)) {
        return NULL;
    }

    return result;
}

void MemoryFree(void *ptr)
{
    if (INV_HANDLE(ptr))
        return;

    VirtualFreeEx(
        GetCurrentProcess(),        /* Deallocate from current process address space */
        (LPVOID)ptr,                /* Address to deallocate */
        0,                          /* Bytes to deallocate ( Unknown, deallocate entire page ) */
        MEM_RELEASE                 /* Release the page ( And implicitly decommit it ) */
    );
}

uint64_t GetTimeMilis(void)
{
    return 0;
}
