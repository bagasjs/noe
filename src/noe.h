#ifndef NOE_H_
#define NOE_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

//////////////////////////////////////////////////////////////
///
/// Platform context detection
///

#if defined(_MSC_VER)
    #if !defined(_WIN32)
        #error "Expecting MSVC only working in Windows Platform"
    #endif
    #define NOE_COMPILER_CL 1
    #define NOE_PLATFORM_WINDOWS 1
    #define NOE_PLATFORM_DESKTOP 1
#elif defined(__clang__)
    #define NOE_COMPILER_CLANG 1
    #if defined(_WIN32)
        #define NOE_PLATFORM_WINDOWS 1
        #define NOE_PLATFORM_DESKTOP 1
    #elif defined(__ANDROID__)
        #define NOE_PLATFORM_ANDROID 1
        #define NOE_PLATFORM_MOBILE 1
    #elif defined(__linux__) || defined(__gnu_linux__)
        #define NOE_PLATFORM_LINUX 1
        #define NOE_PLATFORM_DESKTOP 1
    #else
        #define NOE_PLATFORM_UNKNOWN 1
    #endif
#elif defined(__gcc__)
    #define NOE_COMPILER_GCC 1
    #if defined(_WIN32)
        #define NOE_PLATFORM_WINDOWS 1
        #define NOE_PLATFORM_DESKTOP 1
    #elif defined(__ANDROID__)
        #define NOE_PLATFORM_ANDROID 1
        #define NOE_PLATFORM_MOBILE 1
    #elif defined(__linux__) || defined(__gnu_linux__)
        #define NOE_PLATFORM_LINUX 1
        #define NOE_PLATFORM_DESKTOP 1
    #else
        #define NOE_PLATFORM_UNKNOWN 1
    #endif
#endif

#ifndef NOE_PLATFORM_WINDOWS
#define NOE_PLATFORM_WINDOWS 0
#endif
#ifndef NOE_PLATFORM_ANDROID
#define NOE_PLATFORM_ANDROID 0
#endif
#ifndef NOE_PLATFORM_LINUX
#define NOE_PLATFORM_LINUX 0
#endif
#ifndef NOE_PLATFORM_DESKTOP
#define NOE_PLATFORM_DESKTOP 0
#endif
#ifndef NOE_PLATFORM_MOBILE
#define NOE_PLATFORM_MOBILE 0
#endif
#ifndef NOE_COMPILER_CL
#define NOE_COMPILER_CL 0
#endif
#ifndef NOE_COMPILER_CLANG
#define NOE_COMPILER_CLANG 0
#endif
#ifndef NOE_COMPILER_GCC
#define NOE_COMPILER_GCC 0
#endif

//////////////////////////////////////////////////////////////
///
/// Macros
///

#ifdef NOE_BUILDTYPE_SHAREDLIB
    #ifdef NOE_EXPORT
        #if NOE_COMPILER_CL
            #define NAPI __declspec(dllexport)
        #else
            #define NAPI __attribute__((visibility("default")))
        #endif
    #else
        #if NOE_COMPILER_CL
            #define NAPI __declspec(dllimport)
        #else
            #define NAPI
        #endif
    #endif
#else
    #define NAPI
#endif

#ifndef STATIC_ASSERT
#define STATIC_ASSERT(COND) _Static_assert(COND, #COND)
#endif // STATIC_ASSERT

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

#ifndef TRACELOG
    #if !defined(NDEBUG) && !defined(NOE_BUILD_RELEASE)
        #define TRACELOG(logLevel, ...) TraceLog(logLevel, __VA_ARGS__)
    #else
        #define TRACELOG(logLevel, ...) 
    #endif
#endif

#define HEX2COLOR(hex) ((Color){(hex >> 24) & 0xFF, (hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF})
#define COLOR2HEX(c) ((c.r << 24) | (c.g << 16) | (c.b << 8) | c.a)
#define COLOR2VECTOR4(c) ((float)(c).r/255.0f),((float)(c).g/255.0f),((float)(c).b/255.0f),((float)(c).a/255.0f)

//////////////////////////////////////////////////////////////
///
/// Configurations
///

#ifndef LOG_MESSAGE_MAXIMUM_LENGTH
#define LOG_MESSAGE_MAXIMUM_LENGTH (32*1024)
#endif

#ifndef MAXIMUM_KEYBOARD_KEYS
#define MAXIMUM_KEYBOARD_KEYS 512
#endif

#ifndef MAXIMUM_MOUSE_BUTTONS
#define MAXIMUM_MOUSE_BUTTONS 8
#endif

#ifndef MAXIMUM_KEYPRESSED_QUEUE
#define MAXIMUM_KEYPRESSED_QUEUE 16
#endif

#ifndef MAXIMUM_SHADER_LOCS
#define MAXIMUM_SHADER_LOCS 16
#endif

#ifndef MAXIMUM_BATCH_RENDERER_VERTICES
#define MAXIMUM_BATCH_RENDERER_VERTICES (32*1024)
#endif

#ifndef MAXIMUM_BATCH_RENDERER_ELEMENTS
#define MAXIMUM_BATCH_RENDERER_ELEMENTS (64*1024)
#endif

#ifndef MAXIMUM_BATCH_RENDERER_ACTIVE_TEXTURES
#define MAXIMUM_BATCH_RENDERER_ACTIVE_TEXTURES 8
#endif

#ifndef POSITION_SHADER_ATTRIBUTE_LOCATION
#define POSITION_SHADER_ATTRIBUTE_LOCATION 0
#endif 

#ifndef COLOR_SHADER_ATTRIBUTE_LOCATION
#define COLOR_SHADER_ATTRIBUTE_LOCATION 1
#endif 

#ifndef TEXCOORDS_SHADER_ATTRIBUTE_LOCATION
#define TEXCOORDS_SHADER_ATTRIBUTE_LOCATION 2
#endif 

#ifndef TEXTURE_INDEX_SHADER_ATTRIBUTE_LOCATION
#define TEXTURE_INDEX_SHADER_ATTRIBUTE_LOCATION 3
#endif 

#ifndef TEXTURE_SAMPLERS_SHADER_UNIFORM_LOCATION
#define TEXTURE_SAMPLERS_SHADER_UNIFORM_LOCATION 4
#endif 

#ifndef PROJECTION_MATRIX_SHADER_UNIFORM_LOCATION
#define PROJECTION_MATRIX_SHADER_UNIFORM_LOCATION 5
#endif 

#ifndef VIEW_MATRIX_SHADER_UNIFORM_LOCATION
#define VIEW_MATRIX_SHADER_UNIFORM_LOCATION 6
#endif 

#ifndef MODEL_MATRIX_SHADER_UNIFORM_LOCATION
#define MODEL_MATRIX_SHADER_UNIFORM_LOCATION 7
#endif 

#ifndef POSITION_SHADER_ATTRIBUTE_NAME
#define POSITION_SHADER_ATTRIBUTE_NAME "a_Position"
#endif 

#ifndef COLOR_SHADER_ATTRIBUTE_NAME
#define COLOR_SHADER_ATTRIBUTE_NAME "a_Color"
#endif 

#ifndef TEXCOORDS_SHADER_ATTRIBUTE_NAME
#define TEXCOORDS_SHADER_ATTRIBUTE_NAME "a_TexCoords"
#endif 

#ifndef TEXTURE_INDEX_SHADER_ATTRIBUTE_NAME
#define TEXTURE_INDEX_SHADER_ATTRIBUTE_NAME "a_TextureIndex"
#endif 

#ifndef TEXTURE_SAMPLERS_SHADER_UNIFORM_NAME
#define TEXTURE_SAMPLERS_SHADER_UNIFORM_NAME "u_Textures"
#endif 

#ifndef PROJECTION_MATRIX_SHADER_UNIFORM_NAME
#define PROJECTION_MATRIX_SHADER_UNIFORM_NAME "u_Projection"
#endif 

#ifndef VIEW_MATRIX_SHADER_UNIFORM_NAME
#define VIEW_MATRIX_SHADER_UNIFORM_NAME "u_View"
#endif 

#ifndef MODEL_MATRIX_SHADER_UNIFORM_NAME
#define MODEL_MATRIX_SHADER_UNIFORM_NAME "u_Model"
#endif 

//////////////////////////////////////////////////////////////
///
/// Type definitions
///

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

typedef struct Rectangle {
    float x, y;
    float width, height;
} Rectangle;

typedef struct Image {
    uint8_t *data;
    uint32_t width, height;
    uint32_t compAmount;
} Image;

typedef struct Color {
    uint8_t r, g, b, a;
} Color;

typedef struct Shader {
    uint32_t ID;
    int *locs;
} Shader;

typedef struct Texture {
    uint32_t ID;
    uint32_t width, height;
    uint32_t compAmount; // RGBA = 4, RGB = 3
} Texture;

typedef struct GlyphInfo {
    int codepoint;
    float s0, t0;
    float s1, t1;
} GlyphInfo;

typedef struct TextFont {
    Texture texture;
    Image atlas; // atlas.height is the default fontSize
    GlyphInfo *glyphs;
    int glyphsCount;
} TextFont;

#define WHITE CLITERAL(Color){ .r = 0xFF, .g=0xFF, .b=0xFF, .a=0xFF }
#define BLACK CLITERAL(Color){ .r = 0x00, .g=0x00, .b=0x00, .a=0xFF }
#define RED   CLITERAL(Color){ .r = 0xFF, .g=0x00, .b=0x00, .a=0xFF }
#define GREEN CLITERAL(Color){ .r = 0x00, .g=0xFF, .b=0x00, .a=0xFF }
#define BLUE  CLITERAL(Color){ .r = 0x00, .g=0x00, .b=0xFF, .a=0xFF }

//////////////////////////////////////////////////////////////
///
/// Functions
///

/// Utility functions

NAPI const char *StringFind(const char *haystack, const char *needle);
NAPI char *StringCopy(char *dst, const char *src, size_t length);
NAPI size_t StringLength(const char *str);
NAPI void *MemorySet(void *dst, int value, size_t length);
NAPI void *MemoryCopy(void *dst, const void *src, size_t length);
NAPI const char *GetFileExtension(const char *filePath);
NAPI const char *GetFileName(const char *filePath);

/// Platform core functions (required in every platform except file IO in web)

NAPI void *MemoryAlloc(size_t nBytes); 
NAPI void MemoryFree(void *ptr); 
NAPI void ExitProgram(int status);
NAPI uint64_t GetUnixTimestamp(void); 
NAPI void TraceLog(int logLevel, const char *fmt, ...);
NAPI char *LoadFileText(const char *filePath, size_t *fileSize); // not implemented on web
NAPI void UnloadFileText(char *text); // not implemented on web
NAPI uint8_t *LoadFileData(const char *filePath, size_t *fileSize); // not implemented on web
NAPI void UnloadFileData(uint8_t *data); // not implemented on web

/// Application initialization & deinitialization

NAPI void SetupWindow(const char *title, uint32_t width, uint32_t height, uint32_t flags); // desktop only
NAPI void SetupOpenGL(uint32_t versionMajor, uint32_t versionMinor, uint32_t flags);
NAPI bool InitApplication(void);
NAPI void DeinitApplication(void);

/// Event handling

NAPI void PollInputEvents(void);
NAPI bool WindowShouldClose(void); // desktop only
NAPI bool IsKeyPressed(int key);
NAPI bool IsKeyReleased(int key);
NAPI bool IsKeyDown(int key);
NAPI bool IsKeyUp(int key);
NAPI bool IsMouseButtonPressed(int button);
NAPI bool IsMouseButtonReleased(int button);
NAPI bool IsMouseButtonDown(int button);
NAPI bool IsMouseButtonUp(int button);
NAPI bool IsFrameResized(void);
NAPI Vector2 GetCursorPos(void);

/// Image

NAPI bool LoadImageFromFile(Image *image, const char *filePath);
NAPI bool LoadImage(Image *image, const uint8_t *data, uint32_t width, uint32_t height, uint32_t compAmount);
NAPI void UnloadImage(Image image);

/// Font

NAPI bool LoadFont(TextFont *font, const uint8_t *fontBuffer, int fontSize, int codepointAmount, int *codepoints);
NAPI void UnloadFont(TextFont font);
NAPI Vector2 GetTextDimension(TextFont font, const char *text, int fontSize);
void DrawTextEx(TextFont font, const char *text, int fontSize, Vector2 pos, Color color);

/// Textures

NAPI bool LoadTextureFromFile(Texture *texture, const char *filePath);
NAPI bool LoadTextureFromImage(Texture *result, Image image);
NAPI bool LoadTexture(Texture *result, const uint8_t *data, uint32_t width, uint32_t height, uint32_t compAmount);
NAPI void UnloadTexture(Texture texture);

/// Shaders

NAPI bool LoadShader(Shader *result, const char *vertSource, const char *fragSource);
NAPI bool LoadShaderFromFile(Shader *result, const char *vertSourceFilePath, const char *fragSourceFilePath);
NAPI void UnloadShader(Shader shader);
NAPI void SetProjectionMatrixUniform(Shader shader, float *matrixData);
NAPI void SetViewMatrixUniform(Shader shader, float *matrixData);
NAPI void SetModelMatrixUniform(Shader shader, float *matrixData);
NAPI void SetShaderUniform(Shader shader, int location, int uniformType, const void *data, int count, bool transposeIfMatrix);
NAPI int GetShaderUniformLocation(Shader shader, const char *uniformName);
NAPI int GetShaderAttributeLocation(Shader shader, const char *attributeName);

/// Batch Renderer

NAPI void RenderClear(float r, float g, float b, float a);
NAPI void RenderPresent(void);
NAPI void RenderFlush(Shader shader);
NAPI int  RenderPutVertex(float x, float y, float z, float r, float g, float b, float a, float u, float v, int textureIndex);
NAPI void RenderPutElement(int vertexIndex);
NAPI int  RenderEnableTexture(Texture texture);
NAPI void RenderViewport(int x, int y, uint32_t width, uint32_t height);

/// Drawing

NAPI void ClearBackground(Color color);
NAPI void BeginDrawing(void);
NAPI void EndDrawing(void);
NAPI void DrawRectangle(Color color, float x, float y, float w, float h);
NAPI void DrawTexture(Texture texture, float x, float y, float w, float h);
NAPI void DrawTextureEx(Texture texture, Rectangle src, Rectangle dst, Color tint);
NAPI void DrawTriangle(Color color, float x1, float y1, float x2, float y2, float x3, float y3);
NAPI void DrawCircle(Color color, int cx, int cy, uint32_t r);

/// OpenGL

NAPI void GLSwapBuffers(void);
NAPI void GLGetProc(const char *procName);

/// Desktop platform only

NAPI void SetWindowTitle(const char *title);
NAPI void SetWindowSize(uint32_t width, uint32_t height);
NAPI void SetWindowVisible(bool isVisible);
NAPI void SetWindowResizable(bool isResizable);
NAPI void SetWindowFullscreen(bool isFullscreen);
NAPI bool IsWindowVisible(void);
NAPI bool IsWindowResizable(void);
NAPI bool IsWindowFullscreen(void);
NAPI void SetWindowShouldClose(bool shouldClose);

/*******************************
 * Enumerations
 *******************************/
typedef enum NoeLogLevel {
    LOG_FATAL = 0,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
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

typedef enum {
    MOUSE_BUTTON_LEFT    = 0,       // Mouse button left
    MOUSE_BUTTON_RIGHT   = 1,       // Mouse button right
    MOUSE_BUTTON_MIDDLE  = 2,       // Mouse button middle (pressed wheel)
    MOUSE_BUTTON_SIDE    = 3,       // Mouse button side (advanced mouse device)
    MOUSE_BUTTON_EXTRA   = 4,       // Mouse button extra (advanced mouse device)
    MOUSE_BUTTON_FORWARD = 5,       // Mouse button forward (advanced mouse device)
    MOUSE_BUTTON_BACK    = 6,       // Mouse button back (advanced mouse device)
} NoeMouseButton;

#endif // NOE_H_
