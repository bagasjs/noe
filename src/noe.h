#ifndef NOE_H_
#define NOE_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#if defined(_WIN32)
    #define NOE_PLATFORM_WINDOWS
    #define NOE_PLATFORM_DESKTOP
#elif defined(__ANDROID__)
    #define NOE_PLATFORM_ANDROID
    #define NOE_PLATFORM_MOBILE
#elif defined(__linux__) || defined(__gnu_linux__)
    #define NOE_PLATFORM_LINUX
    #define NOE_PLATFORM_DESKTOP
#else
    #error "Unsupported platform"
#endif

#ifdef NOE_PLATFORM_LINUX
    #if !defined(NOE_LINUX_DISPLAY_X11) && !defined(NOE_LINUX_DISPLAY_WAYLAND)
        #define NOE_LINUX_DISPLAY_X11
    #endif
#endif // NOE_PLATFORM_LINUX

#ifndef MAXIMUM_KEYBOARD_KEYS
    #define MAXIMUM_KEYBOARD_KEYS 512
#endif
#ifndef MAXIMUM_MOUSE_BUTTONS
    #define MAXIMUM_MOUSE_BUTTONS 8
#endif
#ifndef MAXIMUM_KEYPRESSED_QUEUE
    #define MAXIMUM_KEYPRESSED_QUEUE 16
#endif

#ifndef SIGN
    #define SIGN(T, a) (((T)(a) > 0) - ((T)(a) < 0))
#endif 

#ifndef ABS
    #define ABS(T, a) (SIGN(T, a) * a)
#endif

#ifndef CAST
    #define CAST(T, v) ((T)(v))
#endif 

#ifndef BIT
    #define BIT(pos) (1 << (pos))
#endif

#ifndef TRACELOG
    #if !defined(NDEBUG) && !defined(NOE_BUILD_RELEASE)
        #define TRACELOG(logLevel, ...) TraceLog(logLevel, __VA_ARGS__)
    #else
        #define TRACELOG(logLevel, ...) 
    #endif
#endif
#ifndef CLITERAL
    #ifdef __cplusplus
        #define CLITERAL(T) T
    #else
        #define CLITERAL(T) (T)
    #endif
#endif

#define FLAG_SET(n, f) ((n) |= (f))
#define FLAG_CLEAR(n, f) ((n) &= ~(f))
#define FLAG_TOGGLE(n, f) ((n) ^= (f))
#define FLAG_CHECK(n, f) ((n) & (f))

/*******************************
 * Structs & Types
 *******************************/

#ifndef NOMATH_TYPES
#define NOMATH_TYPES
typedef union Vector2 {
    float elements[2];
    struct {
        union {
            float x, r, s;
        };
        union {
            float y, g, t;
        };
    };
} Vector2;

typedef union Vector3 {
    float elements[3];
    struct {
        union {
            float x, r, s;
        };
        union {
            float y, g, t;
        };
        union {
            float z, b, p;
        };
    };
} Vector3;

typedef union Vector4 {
    float elements[4];
    struct {
        union {
            float x, r, s;
        };
        union {
            float y, g, t;
        };
        union {
            float z, b, p;
        };
        union {
            float w, a, q;
        };
    };
} Vector4;

typedef union Matrix {
    float elements[4*4];
    Vector4 rows[4];
} Matrix;
#endif // NOMATH_TYPES

#ifndef NOE_SAFE_WIN32_INCLUDE
typedef struct Rectangle {
    int x, y;
    uint32_t width, height;
} Rectangle;
#endif // NOE_SAFE_WIN32_INCLUDE

typedef struct Shader {
    uint32_t ID;
    int *locs;
} Shader;

typedef struct Texture {
    uint32_t ID;
    uint32_t width, height;
    uint32_t compAmount; // RGBA = 4, RGB = 3
} Texture;

typedef struct Color {
    uint8_t r, g, b, a;
} Color;

#define WHITE CLITERAL(Color){ .r = 0xFF, .g=0xFF, .b=0xFF, .a=0xFF }
#define BLACK CLITERAL(Color){ .r = 0x00, .g=0x00, .b=0x00, .a=0xFF }
#define RED   CLITERAL(Color){ .r = 0xFF, .g=0x00, .b=0x00, .a=0xFF }
#define GREEN CLITERAL(Color){ .r = 0x00, .g=0xFF, .b=0x00, .a=0xFF }
#define BLUE  CLITERAL(Color){ .r = 0x00, .g=0x00, .b=0xFF, .a=0xFF }

/*******************************
 * Functions
 *******************************/

/// Non application dependent functions

//
const char *StringFind(const char *haystack, const char *needle);
char *StringCopy(char *dst, const char *src, size_t length);
size_t StringLength(const char *str);
void *MemorySet(void *dst, int value, size_t length);
void *MemoryCopy(void *dst, const void *src, size_t length);
void *MemoryAlloc(size_t nBytes);
void MemoryFree(void *ptr);
void TraceLog(int logLevel, const char *fmt, ...);


/// Application configuration functions 

//
void SetupWindow(const char *title, uint32_t width, uint32_t height, uint32_t flags);
void SetupOpenGL(uint32_t versionMajor, uint32_t versionMinor, uint32_t flags);
bool InitApplication(void);
void DeinitApplication(void);

#ifndef NOE_SAFE_WIN32_INCLUDE
void SetWindowTitle(const char *title);
void SetWindowSize(uint32_t width, uint32_t height);
void SetWindowVisible(bool isVisible);
void SetWindowResizable(bool isResizable);
void SetWindowFullscreen(bool isFullscreen);
bool IsWindowVisible(void);
bool IsWindowResizable(void);
bool IsWindowFullscreen(void);
#endif // NOE_SAFE_WIN32_INCLUDE

uint64_t GetTimeMilis(void); // Get time elapsed in milisecond

/// Event handling

//
void PollInputEvents(void);
bool IsKeyPressed(int key);
bool IsKeyReleased(int key);
bool IsKeyDown(int key);
bool IsKeyUp(int key);
bool IsMouseButtonPressed(int button);
bool IsMouseButtonReleased(int button);
bool IsMouseButtonDown(int button);
bool IsMouseButtonUp(int button);
bool IsFrameResized(void);
void SetWindowShouldClose(bool shouldClose);
bool WindowShouldClose(void);

/// Textures

bool LoadTexture(Texture *result, const uint8_t *data, uint32_t width, uint32_t height, uint32_t compAmount);
bool LoadTextureFromFile(Texture *texture, const char *filePath, bool flipVerticallyOnLoad);
void UnloadTexture(Texture texture);

/// Shaders

bool LoadShader(Shader *result, const char *vertSource, const char *fragSource);
bool LoadShaderFromFile(Shader *result, const char *vertSourceFilePath, const char *fragSourceFilePath);
void UnloadShader(Shader shader);
void SetProjectionMatrixUniform(Shader shader, float *matrixData);
void SetViewMatrixUniform(Shader shader, float *matrixData);
void SetModelMatrixUniform(Shader shader, float *matrixData);
void SetShaderUniform(Shader shader, int location, int uniformType, const void *data, int count, bool transposeIfMatrix);
int GetShaderUniformLocation(Shader shader, const char *uniformName);
int GetShaderAttributeLocation(Shader shader, const char *attributeName);

///
/// Batch Renderer
///

void RenderClear(float r, float g, float b, float a);
void RenderFlush(Shader shader);
int RenderPutVertex(float x, float y, float z, float r, float g, float b, float a, float u, float v, int textureIndex);
void RenderPutElement(int vertexIndex);
int RenderEnableTexture(Texture texture);
void RenderViewport(int x, int y, uint32_t width, uint32_t height);

/// Drawing

#ifndef NOE_SAFE_WIN32_INCLUDE
void ClearBackground(Color color);
void BeginDrawing(void);
void EndDrawing(void);
void DrawRectangle(Color color, int x, int y, uint32_t w, uint32_t h);
void DrawTexture(Texture texture, int x, int y, uint32_t w, uint32_t h);
void DrawTextureEx(Texture texture, Rectangle src, Rectangle dst);
void DrawTriangle(Color color, int x1, int y1, int x2, int y2, int x3, int y3);
void DrawCircle(Color color, int cx, int cy, uint32_t r);
#endif // NOE_SAFE_WIN32_INCLUDE


/// OpenGL

//
void SwapBufferGL(void);
void *GetProcGL(const char *procName);


/*******************************
 * Enumerations
 *******************************/
typedef enum NoeLogLevel {
    LOG_INFO = 0,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL,
} NoeLogLevel;

typedef enum NoeWindowSetupFlags {
    WINDOW_SETUP_DEFAULT = 0,
    WINDOW_SETUP_VISIBLE = BIT(1),
    WINDOW_SETUP_RESIZABLE = BIT(2),
    WINDOW_SETUP_FULLSCREEN = BIT(3),
    WINDOW_SETUP_DECORATED = BIT(4),
} NoeWindowSetupFlags;

typedef enum NoeOpenGLSetupFlags {
    OPENGL_SETUP_DEFAULT = 0,
    OPENGL_SETUP_NATIVE_CONTEXT = BIT(1),
    OPENGL_SETUP_CORE_PROFILE = BIT(2),
    OPENGL_SETUP_OPENGLES = BIT(3),
    OPENGL_SETUP_DEBUG_CONTEXT = BIT(4),
    OPENGL_SETUP_FORWARD = BIT(5),
} NoeOpenGLSetupFlags;

typedef enum NoeShaderUniformType {
    INVALID_SHADER_UNIFORM = 0,
    SHADER_UNIFORM_FLOAT, SHADER_UNIFORM_VEC2, SHADER_UNIFORM_VEC3, SHADER_UNIFORM_VEC4,
    SHADER_UNIFORM_UINT, SHADER_UNIFORM_UVEC2, SHADER_UNIFORM_UVEC3, SHADER_UNIFORM_UVEC4,
    SHADER_UNIFORM_INT, SHADER_UNIFORM_IVEC2, SHADER_UNIFORM_IVEC3, SHADER_UNIFORM_IVEC4,
    SHADER_UNIFORM_MAT3, SHADER_UNIFORM_MAT4, SHADER_UNIFORM_SAMPLER,
} NoeShaderUniformType;

typedef enum NoeKeyCode {
    KEY_INVALID            = 0,

    /* Printable keys */
    KEY_SPACE              = 32,
    KEY_APOSTROPHE         = 39,  /* ' */
    KEY_COMMA              = 44,  /* , */
    KEY_MINUS              = 45,  /* - */
    KEY_PERIOD             = 46,  /* . */
    KEY_SLASH              = 47,  /* / */
    KEY_0                  = 48,
    KEY_1                  = 49,
    KEY_2                  = 50,
    KEY_3                  = 51,
    KEY_4                  = 52,
    KEY_5                  = 53,
    KEY_6                  = 54,
    KEY_7                  = 55,
    KEY_8                  = 56,
    KEY_9                  = 57,
    KEY_SEMICOLON          = 59,  /* ; */
    KEY_EQUAL              = 61,  /* = */
    KEY_A                  = 65,
    KEY_B                  = 66,
    KEY_C                  = 67,
    KEY_D                  = 68,
    KEY_E                  = 69,
    KEY_F                  = 70,
    KEY_G                  = 71,
    KEY_H                  = 72,
    KEY_I                  = 73,
    KEY_J                  = 74,
    KEY_K                  = 75,
    KEY_L                  = 76,
    KEY_M                  = 77,
    KEY_N                  = 78,
    KEY_O                  = 79,
    KEY_P                  = 80,
    KEY_Q                  = 81,
    KEY_R                  = 82,
    KEY_S                  = 83,
    KEY_T                  = 84,
    KEY_U                  = 85,
    KEY_V                  = 86,
    KEY_W                  = 87,
    KEY_X                  = 88,
    KEY_Y                  = 89,
    KEY_Z                  = 90,
    KEY_LEFT_BRACKET       = 91,  /* [ */
    KEY_BACKSLASH          = 92,  /* \ */
    KEY_RIGHT_BRACKET      = 93,  /* ] */
    KEY_GRAVE_ACCENT       = 96,  /* ` */
    KEY_WORLD_1            = 161, /* non-US #1 */
    KEY_WORLD_2            = 162, /* non-US #2 */

    /* Function keys */
    KEY_ESCAPE             = 256,
    KEY_ENTER              = 257,
    KEY_TAB                = 258,
    KEY_BACKSPACE          = 259,
    KEY_INSERT             = 260,
    KEY_DELETE             = 261,
    KEY_RIGHT              = 262,
    KEY_LEFT               = 263,
    KEY_DOWN               = 264,
    KEY_UP                 = 265,
    KEY_PAGE_UP            = 266,
    KEY_PAGE_DOWN          = 267,
    KEY_HOME               = 268,
    KEY_END                = 269,
    KEY_CAPS_LOCK          = 280,
    KEY_SCROLL_LOCK        = 281,
    KEY_NUM_LOCK           = 282,
    KEY_PRINT_SCREEN       = 283,
    KEY_PAUSE              = 284,
    KEY_F1                 = 290,
    KEY_F2                 = 291,
    KEY_F3                 = 292,
    KEY_F4                 = 293,
    KEY_F5                 = 294,
    KEY_F6                 = 295,
    KEY_F7                 = 296,
    KEY_F8                 = 297,
    KEY_F9                 = 298,
    KEY_F10                = 299,
    KEY_F11                = 300,
    KEY_F12                = 301,
    KEY_F13                = 302,
    KEY_F14                = 303,
    KEY_F15                = 304,
    KEY_F16                = 305,
    KEY_F17                = 306,
    KEY_F18                = 307,
    KEY_F19                = 308,
    KEY_F20                = 309,
    KEY_F21                = 310,
    KEY_F22                = 311,
    KEY_F23                = 312,
    KEY_F24                = 313,
    KEY_F25                = 314,
    KEY_KP_0               = 320,
    KEY_KP_1               = 321,
    KEY_KP_2               = 322,
    KEY_KP_3               = 323,
    KEY_KP_4               = 324,
    KEY_KP_5               = 325,
    KEY_KP_6               = 326,
    KEY_KP_7               = 327,
    KEY_KP_8               = 328,
    KEY_KP_9               = 329,
    KEY_KP_DECIMAL         = 330,
    KEY_KP_DIVIDE          = 331,
    KEY_KP_MULTIPLY        = 332,
    KEY_KP_SUBTRACT        = 333,
    KEY_KP_ADD             = 334,
    KEY_KP_ENTER           = 335,
    KEY_KP_EQUAL           = 336,
    KEY_LEFT_SHIFT         = 340,
    KEY_LEFT_CONTROL       = 341,
    KEY_LEFT_ALT           = 342,
    KEY_LEFT_SUPER         = 343,
    KEY_RIGHT_SHIFT        = 344,
    KEY_RIGHT_CONTROL      = 345,
    KEY_RIGHT_ALT          = 346,
    KEY_RIGHT_SUPER        = 347,
    KEY_MENU               = 348,
    KEY_LAST = KEY_MENU,
} NoeKeyCode;

typedef enum NoeKeyMods {
    KEY_MOD_SHIFT      = 0x0001,
    KEY_MOD_CONTROL    = 0x0002,
    KEY_MOD_ALT        = 0x0004,
    KEY_MOD_SUPER      = 0x0008,
    KEY_MOD_CAPSLOCK   = 0x0010,
    KEY_MOD_NUMLOCK    = 0x0020,
} NoeKeyMods;

#endif // NOE_H_
