#include "noe.h"
#include "noe_internal.h"

#include <glad/glad.h>

#define NOMATH_IMPLEMENTATION
#include "nomath.h"

#ifndef POSITION_SHADER_ATTRIBUTE_LOCATION
    #define POSITION_SHADER_ATTRIBUTE_LOCATION 0
#endif // POSITION_SHADER_ATTRIBUTE_LOCATION
#ifndef COLOR_SHADER_ATTRIBUTE_LOCATION
    #define COLOR_SHADER_ATTRIBUTE_LOCATION 1
#endif // COLOR_SHADER_ATTRIBUTE_LOCATION
#ifndef TEXCOORDS_SHADER_ATTRIBUTE_LOCATION
    #define TEXCOORDS_SHADER_ATTRIBUTE_LOCATION 2
#endif // TEXCOORDS_SHADER_ATTRIBUTE_LOCATION
#ifndef TEXTURE_INDEX_SHADER_ATTRIBUTE_LOCATION
    #define TEXTURE_INDEX_SHADER_ATTRIBUTE_LOCATION 3
#endif // TEXTURE_INDEX_SHADER_ATTRIBUTE_LOCATION
#ifndef TEXTURE_SAMPLERS_SHADER_UNIFORM_LOCATION
    #define TEXTURE_SAMPLERS_SHADER_UNIFORM_LOCATION 4
#endif // TEXTURE_SAMPLERS_SHADER_UNIFORM_LOCATION
#ifndef PROJECTION_MATRIX_SHADER_UNIFORM_LOCATION
    #define PROJECTION_MATRIX_SHADER_UNIFORM_LOCATION 5
#endif // PROJECTION_MATRIX_SHADER_UNIFORM_LOCATION
#ifndef VIEW_MATRIX_SHADER_UNIFORM_LOCATION
    #define VIEW_MATRIX_SHADER_UNIFORM_LOCATION 6
#endif // VIEW_MATRIX_SHADER_UNIFORM_LOCATION
#ifndef MODEL_MATRIX_SHADER_UNIFORM_LOCATION
    #define MODEL_MATRIX_SHADER_UNIFORM_LOCATION 7
#endif // MODEL_MATRIX_SHADER_UNIFORM_LOCATION
#ifndef POSITION_SHADER_ATTRIBUTE_NAME
    #define POSITION_SHADER_ATTRIBUTE_NAME "a_Position"
#endif // POSITION_SHADER_ATTRIBUTE_NAME
#ifndef COLOR_SHADER_ATTRIBUTE_NAME
    #define COLOR_SHADER_ATTRIBUTE_NAME "a_Color"
#endif // COLOR_SHADER_ATTRIBUTE_NAME
#ifndef TEXCOORDS_SHADER_ATTRIBUTE_NAME
    #define TEXCOORDS_SHADER_ATTRIBUTE_NAME "a_TexCoords"
#endif // TEXCOORDS_SHADER_ATTRIBUTE_NAME
#ifndef TEXTURE_INDEX_SHADER_ATTRIBUTE_NAME
    #define TEXTURE_INDEX_SHADER_ATTRIBUTE_NAME "a_TextureIndex"
#endif // TEXTURE_INDEX_SHADER_ATTRIBUTE_NAME
#ifndef TEXTURE_SAMPLERS_SHADER_UNIFORM_NAME
    #define TEXTURE_SAMPLERS_SHADER_UNIFORM_NAME "u_Textures"
#endif // TEXTURE_SAMPLERS_SHADER_UNIFORM_NAME
#ifndef PROJECTION_MATRIX_SHADER_UNIFORM_NAME
    #define PROJECTION_MATRIX_SHADER_UNIFORM_NAME "u_Projection"
#endif // PROJECTION_MATRIX_SHADER_UNIFORM_NAME
#ifndef VIEW_MATRIX_SHADER_UNIFORM_NAME
    #define VIEW_MATRIX_SHADER_UNIFORM_NAME "u_View"
#endif // VIEW_MATRIX_SHADER_UNIFORM_NAME
#ifndef MODEL_MATRIX_SHADER_UNIFORM_NAME
    #define MODEL_MATRIX_SHADER_UNIFORM_NAME "u_Model"
#endif // MODEL_MATRIX_SHADER_UNIFORM_NAME

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

typedef struct _WindowState {
    const char *title;
    uint32_t width, height;

    int defaultExitButton;
    bool visible, resizable, fullScreen;
    bool shouldClose;
} _WindowState;

typedef struct _RenderVertex {
    struct { float x, y, z; } pos;
    struct { float r, g, b, a; } color;
    struct { float u, v; } texCoords;
    float textureIndex;
} _RenderVertex;

typedef struct _BatchRendererState {
    struct {
        bool supportVAO;
    } config;

    uint32_t vaoID, vboID, eboID;
    Shader defaultShader;
    struct {
        _RenderVertex data[MAXIMUM_BATCH_RENDERER_VERTICES];
        uint32_t count;
    } vertices;
    struct {
        uint32_t data[MAXIMUM_BATCH_RENDERER_ELEMENTS];
        uint32_t count;
    } elements;
    struct {
        uint32_t data[MAXIMUM_BATCH_RENDERER_ACTIVE_TEXTURES];
        uint32_t count;
    } activeTextureIDs;
} _BatchRendererState;

typedef struct _ApplicationState {
    bool initialized;
    _WindowState window;
    _InputManager inputs;
    _BatchRendererState renderer;
} _ApplicationState;

static _ApplicationConfig appConfig = {
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

// Defined in noe_platform_xxx.c
bool platformInit(const _ApplicationConfig *config);
void platformDeinit(void);
void platformPollEvent(_InputManager *inputs);
void platformSetWindowTitle(const char *title);
void platformSetWindowSize(uint32_t width, uint32_t height);
void platformSetWindowVisible(bool isVisible);
void platformSetWindowResizable(bool isResizable);
void platformSetWindowFullscreen(bool isFullscreen);
void SwapGLBuffer(void);
uint64_t GetTimeMilis(void);

bool initBatchRenderer(void)
{
    gladLoadGL();

    APP.renderer.config.supportVAO = appConfig.opengl.useCoreProfile;
    APP.renderer.vertices.count = 0;
    APP.renderer.elements.count = 0;
    APP.renderer.activeTextureIDs.count = 0;

    glGenBuffers(1, &APP.renderer.vboID);
    glBindBuffer(GL_ARRAY_BUFFER, APP.renderer.vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(APP.renderer.vertices.data), NULL, GL_DYNAMIC_DRAW);

    if(APP.renderer.config.supportVAO) {
        glBindVertexArray(APP.renderer.vaoID);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(_RenderVertex), (void*)offsetof(_RenderVertex, pos));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(_RenderVertex), (void*)offsetof(_RenderVertex, color));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(_RenderVertex), (void*)offsetof(_RenderVertex, texCoords));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(_RenderVertex), (void*)offsetof(_RenderVertex, textureIndex));
    }

    glGenBuffers(1, &APP.renderer.eboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, APP.renderer.eboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(APP.renderer.elements.data), NULL, GL_DYNAMIC_DRAW);

    return true;
}

void deinitBatchRenderer(void)
{
    if(APP.renderer.config.supportVAO) glDeleteVertexArrays(1, &APP.renderer.vaoID);
    glDeleteBuffers(1, &APP.renderer.eboID);
    glDeleteBuffers(1, &APP.renderer.vboID);
}

bool InitApplication(void)
{
    if(APP.initialized) return false;
    TRACELOG(LOG_INFO, "Initializing application");

    APP.window.title = appConfig.window.title;
    APP.window.width = appConfig.window.width;
    APP.window.height = appConfig.window.height;
    APP.window.visible = appConfig.window.visible;
    APP.window.resizable = appConfig.window.resizable;
    APP.window.fullScreen = appConfig.window.fullScreen;
    APP.window.shouldClose = false;
    APP.window.defaultExitButton = KEY_ESCAPE;
    if(!platformInit(&appConfig)) return false;

#ifndef NOE_PLATFORM_WINDOWS
    TRACELOG(LOG_INFO, "Initializing batch renderer (OpenGL)");
    if(!initBatchRenderer()) {
        TRACELOG(LOG_ERROR, "Initializing batch renderer failed (OpenGL)");
        return false;
    }
    TRACELOG(LOG_INFO, "Initializing batch renderer success (OpenGL)");
#endif

    TRACELOG(LOG_INFO, "Initializing application success");
    APP.initialized = true;
    return true;
}

_InputManager *getApplicationInputManager(void)
{
    return &APP.inputs;
}


void DeinitApplication(void)
{
    if(!APP.initialized) return;
#ifndef NOE_PLATFORM_WIN32
    deinitBatchRenderer();
#endif
    platformDeinit();
}

void SetWindowTitle(const char *title)
{
    NOE_REQUIRE_INIT_OR_RETURN_VOID("`SetWindowTitle()` requires you to call `InitApplication()`");
#ifdef NOE_PLATFORM_DESKTOP
    APP.window.title = title;
    platformSetWindowTitle(title);
#else
    TRACELOG(LOG_ERROR, "`SetWindowTitle()` only works on Desktop Platform");
#endif
}

void SetWindowSize(uint32_t width, uint32_t height)
{
#ifndef NOE_PLATFORM_DESKTOP
    TRACELOG(LOG_ERROR, "`SetWindowVisible()` only works on Desktop Platform");
#else
    NOE_REQUIRE_INIT_OR_RETURN_VOID("`SetWindowSize()` requires you to call `InitApplication()`");
    APP.window.width = width;
    APP.window.height = height;
#endif
}

void SetWindowVisible(bool isVisible)
{
#ifndef NOE_PLATFORM_DESKTOP
    TRACELOG(LOG_ERROR, "`SetWindowVisible()` only works on Desktop Platform");
#else
    NOE_REQUIRE_INIT_OR_RETURN_VOID("`SetWindowVisible()` requires you to call `InitApplication()`");
    APP.window.visible = isVisible;
    platformSetWindowVisible(isVisible);
#endif
}

void SetWindowResizable(bool isResizable)
{
#ifndef NOE_PLATFORM_DESKTOP
    TRACELOG(LOG_ERROR, "`SetWindowResizable()` only works on Desktop Platform");
#else
    NOE_REQUIRE_INIT_OR_RETURN_VOID("`SetWindowResizable()` requires you to call `InitApplication()`");
    APP.window.resizable = isResizable;
    platformSetWindowResizable(isResizable);
#endif
}

void SetWindowFullscreen(bool isFullscreen)
{
#ifndef NOE_PLATFORM_DESKTOP
    TRACELOG(LOG_ERROR, "`SetWindowFullscreen()` only works on Desktop Platform");
#else
    NOE_REQUIRE_INIT_OR_RETURN_VOID("`SetWindowFullscreen()` requires you to call `InitApplication()`");
    APP.window.fullScreen = isFullscreen;
#endif
}

bool IsWindowVisible(void)
{
#ifndef NOE_PLATFORM_DESKTOP
    TRACELOG(LOG_WARNING, "`IsWindowVisible()` is only available on Desktop platform");
    return false;
#endif
    return APP.window.visible;
}

bool IsWindowResizable(void)
{
#ifndef NOE_PLATFORM_DESKTOP
    TRACELOG(LOG_WARNING, "`IsWindowResizable()` is only available on Desktop platform");
    return false;
#endif
    return APP.window.resizable;
}

bool IsWindowFullscreen(void)
{
#ifndef NOE_PLATFORM_DESKTOP
    TRACELOG(LOG_WARNING, "`IsWindowFullscreen()` is only available on Desktop platform");
    return false;
#endif
    return APP.window.fullScreen;
}

void PollInputEvents(void)
{
    NOE_REQUIRE_INIT_OR_RETURN_VOID("`PollInputEvents()` requires you to call `InitApplication()`");
    APP.window.shouldClose = false;
    if(!APP.initialized) return;
    APP.inputs.keyboard.keyPressedQueueCount = 0;

    for (int i = 0; i < MAXIMUM_KEYBOARD_KEYS; i++)
        APP.inputs.keyboard.previousKeyState[i] = APP.inputs.keyboard.currentKeyState[i];

    for (int i = 0; i < MAXIMUM_MOUSE_BUTTONS; i++) 
        APP.inputs.mouse.previousButtonState[i] = APP.inputs.mouse.currentButtonState[i];

    // APP.inputs.mouse.previousPosition = APP.inputs.mouse.currentPosition;
    // APP.inputs.mouse.currentPosition = (Vector2){ 0.0f, 0.0f };

    // APP.inputs.mouse.previousWheelMove = APP.inputs.mouse.currentWheelMove;
    // APP.inputs.mouse.currentWheelMove = (Vector2){ 0.0f, 0.0f };

    if(IsKeyDown(KEY_ESCAPE)) SetWindowShouldClose(true);

    platformPollEvent(&APP.inputs);
}

void SetWindowShouldClose(bool shouldClose)
{
    NOE_REQUIRE_INIT_OR_RETURN_VOID("`SetWindowShouldClose()` requires you to call `InitApplication()`");
    APP.window.shouldClose = shouldClose;
}

bool WindowShouldClose(void)
{
    NOE_REQUIRE_INIT_OR_RETURN(true, "`WindowShouldClose()` requires you to call `InitApplication()`");
    return APP.window.shouldClose;
}

bool IsKeyPressed(int key)
{
    NOE_REQUIRE_INIT_OR_RETURN(true, "`IsKeyPressed()` requires you to call `InitApplication()`");
    bool pressed = false;
    if((key > 0) && (key < MAXIMUM_KEYBOARD_KEYS)) {
        if ((APP.inputs.keyboard.previousKeyState[key] == 0) && (APP.inputs.keyboard.currentKeyState[key] == 1)) 
            pressed = true;
    }
    return pressed;
}

bool IsKeyDown(int key)
{
    NOE_REQUIRE_INIT_OR_RETURN(true, "`IsKeyDown()` requires you to call `InitApplication()`");
    bool down = false;
    if ((key > 0) && (key < MAXIMUM_KEYBOARD_KEYS)) {
        if (APP.inputs.keyboard.currentKeyState[key] == 1) down = true;
    }
    return down;
}

bool IsKeyReleased(int key)
{
    NOE_REQUIRE_INIT_OR_RETURN(true, "`IsKeyReleased()` requires you to call `InitApplication()`");
    bool released = false;
    if ((key > 0) && (key < MAXIMUM_KEYBOARD_KEYS)) {
        if ((APP.inputs.keyboard.previousKeyState[key] == 1) && (APP.inputs.keyboard.currentKeyState[key] == 0)) 
            released = true;
    }
    return released;
}

bool IsKeyUp(int key)
{
    NOE_REQUIRE_INIT_OR_RETURN(true, "`IsKeyUp()` requires you to call `InitApplication()`");
    bool up = false;
    if ((key > 0) && (key < MAXIMUM_KEYBOARD_KEYS)) {
        if (APP.inputs.keyboard.currentKeyState[key] == 0) up = true;
    }
    return up;
}

void SetupOpenGL(uint32_t versionMajor, uint32_t versionMinor, uint32_t flags)
{
    appConfig.opengl.version.major = versionMajor;
    appConfig.opengl.version.major = versionMinor;
    (void)flags;
}

void SetupWindow(const char *title, uint32_t width, uint32_t height, uint32_t flags)
{
    appConfig.window.title = title;
    appConfig.window.width = width;
    appConfig.window.height = height;
    if(flags == 0) flags = WINDOW_SETUP_VISIBLE | WINDOW_SETUP_DECORATED;
    appConfig.window.resizable = FLAG_CHECK(WINDOW_SETUP_RESIZABLE, flags) ? 1 : 0;
    appConfig.window.fullScreen = FLAG_CHECK(WINDOW_SETUP_FULLSCREEN, flags) ? 1 : 0;
    appConfig.window.visible = FLAG_CHECK(WINDOW_SETUP_VISIBLE, flags) ? 1 : 0;
    appConfig.window.decorated = FLAG_CHECK(WINDOW_SETUP_DECORATED, flags) ? 1 : 0;
}

void RenderClear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderFlush(Shader shader)
{
    if(APP.renderer.config.supportVAO) glBindVertexArray(APP.renderer.vaoID);
    if(APP.renderer.vertices.count > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, APP.renderer.vboID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(APP.renderer.vertices.data[0])*APP.renderer.vertices.count, 
                (void *)APP.renderer.vertices.data);
    }
    if(APP.renderer.elements.count > 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, APP.renderer.eboID);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(APP.renderer.elements.data[0])*APP.renderer.elements.count, 
                (void *)APP.renderer.elements.data);
    }
    if(APP.renderer.config.supportVAO) glBindVertexArray(0);

    glUseProgram(shader.ID);
    if(APP.renderer.config.supportVAO) glBindVertexArray(APP.renderer.vaoID);
    else {
        glBindBuffer(GL_ARRAY_BUFFER, APP.renderer.vboID); 
        glEnableVertexAttribArray(shader.locs[POSITION_SHADER_ATTRIBUTE_LOCATION]);
        glVertexAttribPointer(shader.locs[POSITION_SHADER_ATTRIBUTE_LOCATION], 
                3, GL_FLOAT, GL_FALSE, sizeof(_RenderVertex), (void*)offsetof(_RenderVertex, pos));
        glEnableVertexAttribArray(shader.locs[COLOR_SHADER_ATTRIBUTE_LOCATION]);
        glVertexAttribPointer(shader.locs[COLOR_SHADER_ATTRIBUTE_LOCATION], 
                4, GL_FLOAT, GL_FALSE, sizeof(_RenderVertex), (void*)offsetof(_RenderVertex, color));
        glEnableVertexAttribArray(shader.locs[TEXCOORDS_SHADER_ATTRIBUTE_LOCATION]);
        glVertexAttribPointer(shader.locs[TEXCOORDS_SHADER_ATTRIBUTE_LOCATION], 
                2, GL_FLOAT, GL_FALSE, sizeof(_RenderVertex), (void*)offsetof(_RenderVertex, texCoords));
        glEnableVertexAttribArray(shader.locs[TEXTURE_INDEX_SHADER_ATTRIBUTE_LOCATION]);
        glVertexAttribPointer(shader.locs[TEXTURE_INDEX_SHADER_ATTRIBUTE_LOCATION], 
                1, GL_FLOAT, GL_FALSE, sizeof(_RenderVertex), (void*)offsetof(_RenderVertex, textureIndex));

        if(APP.renderer.elements.count > 0) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, APP.renderer.eboID);
    }

    int textureUnits[MAXIMUM_BATCH_RENDERER_ACTIVE_TEXTURES] = {0};
    for(int i = 0; i < MAXIMUM_BATCH_RENDERER_ACTIVE_TEXTURES; ++i) {
        textureUnits[i] = i;
    }

    for(int i = 0; i < (int)APP.renderer.activeTextureIDs.count; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, APP.renderer.activeTextureIDs.data[i]);
    }

    glUniform1iv(shader.locs[TEXTURE_SAMPLERS_SHADER_UNIFORM_LOCATION],
            MAXIMUM_BATCH_RENDERER_ACTIVE_TEXTURES, textureUnits);

    if(APP.renderer.elements.count > 0) {
        glDrawElements(GL_TRIANGLES, APP.renderer.elements.count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, APP.renderer.vertices.count);
    }

    if(APP.renderer.config.supportVAO) glBindVertexArray(0);
    else {
        if(APP.renderer.elements.count > 0) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glUseProgram(0);

    APP.renderer.vertices.count = 0;
    APP.renderer.elements.count = 0;
    APP.renderer.activeTextureIDs.count = 0;
}

void RenderViewport(int x, int y, uint32_t width, uint32_t height)
{
    glViewport(x, y, width, height);
}

int RenderEnableTexture(Texture texture)
{
    int index = APP.renderer.activeTextureIDs.count;
    APP.renderer.activeTextureIDs.data[index] = texture.ID;
    APP.renderer.activeTextureIDs.count += 1;
    return index;
}

int RenderPutVertex(float x, float y, float z, float r, float g, float b, float a, float u, float v, int textureIndex)
{
    int index = APP.renderer.vertices.count;
    _RenderVertex *vertex = &APP.renderer.vertices.data[index];
    vertex->pos.x = x;
    vertex->pos.y = y;
    vertex->pos.z = z;
    vertex->color.r = r;
    vertex->color.g = g;
    vertex->color.b = b;
    vertex->color.a = a;
    vertex->texCoords.u = u;
    vertex->texCoords.v = v;
    vertex->textureIndex= (float)textureIndex;

    APP.renderer.vertices.count += 1;
    return index;
}

void RenderPutElement(int vertexIndex)
{
    APP.renderer.elements.data[APP.renderer.elements.count] = vertexIndex;
    APP.renderer.elements.count += 1;
}

bool LoadTexture(Texture *texture, const uint8_t *data, uint32_t width, uint32_t height, uint32_t compAmount)
{
    if(!texture) return false;
    if(!data) return false;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texture->ID);
    glBindTexture(GL_TEXTURE_2D, texture->ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, 
            compAmount== 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    texture->height = height;
    texture->width = width;
    texture->compAmount = compAmount;
    TRACELOG(LOG_INFO, "Loaded texture with id %u", texture->ID);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void UnloadTexture(Texture texture)
{
    glDeleteTextures(1, &texture.ID);
}

bool LoadShader(Shader *shader, const char *vertSource, const char *fragSource)
{
    if(!shader) return false;
    if(!vertSource) return false;
    if(!fragSource) return false;
    uint32_t vertModule, fragModule;
    int success;

    vertModule = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertModule, 1, (const char **)&vertSource, NULL);
    glCompileShader(vertModule);
    glGetShaderiv(vertModule, GL_COMPILE_STATUS, &success);
    if(!success) {
        char info_log[512];
        glGetShaderInfoLog(vertModule, sizeof(info_log), NULL, info_log);
        TRACELOG(LOG_ERROR, "Vertex shader compilation error \"%s\"", info_log);
        return false;
    }

    fragModule = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragModule, 1, (const char **)&fragSource, NULL);
    glCompileShader(fragModule);
    glGetShaderiv(fragModule, GL_COMPILE_STATUS, &success);
    if(!success) {
        char info_log[512];
        glGetShaderInfoLog(fragModule, sizeof(info_log), NULL, info_log);
        glDeleteShader(vertModule);
        TRACELOG(LOG_ERROR, "Fragment shader compilation error \"%s\"", info_log);
        return false;
    }

    shader->ID = glCreateProgram();
    glAttachShader(shader->ID, vertModule);
    glAttachShader(shader->ID, fragModule);
    glLinkProgram(shader->ID);
    glGetProgramiv(shader->ID, GL_LINK_STATUS, &success);
    if(!success) {
        char info_log[512];
        glGetProgramInfoLog(shader->ID,  sizeof(info_log), NULL, info_log);
        glDeleteShader(vertModule);
        glDeleteShader(fragModule);
        TRACELOG(LOG_ERROR, "Shader Linking Error: %s\n", info_log);
        return false;
    }
    glDeleteShader(vertModule);
    glDeleteShader(fragModule);

    glUseProgram(shader->ID);
    int loc = -1;
    shader->locs = MemoryAlloc(sizeof(int) * MAXIMUM_SHADER_LOCS);

#define GET_LOCATION_OF(func, name, mandatory) \
    do { \
        loc = func(*shader, (name)); \
        if(loc < 0) { \
            TRACELOG((mandatory) ? LOG_ERROR : LOG_WARNING, "Failed to find location of %s", name); \
            if(mandatory) { \
                MemoryFree(shader->locs); \
                return false; \
            } \
        } \
    } while(0);

    GET_LOCATION_OF(GetShaderAttributeLocation, POSITION_SHADER_ATTRIBUTE_NAME, true);
    shader->locs[POSITION_SHADER_ATTRIBUTE_LOCATION] = loc;

    GET_LOCATION_OF(GetShaderAttributeLocation, COLOR_SHADER_ATTRIBUTE_NAME, true);
    shader->locs[COLOR_SHADER_ATTRIBUTE_LOCATION] = loc;

    GET_LOCATION_OF(GetShaderAttributeLocation, TEXCOORDS_SHADER_ATTRIBUTE_NAME, true);
    shader->locs[TEXCOORDS_SHADER_ATTRIBUTE_LOCATION] = loc;

    GET_LOCATION_OF(GetShaderAttributeLocation, TEXTURE_INDEX_SHADER_ATTRIBUTE_NAME, true);
    shader->locs[TEXTURE_INDEX_SHADER_ATTRIBUTE_LOCATION] = loc;

    GET_LOCATION_OF(GetShaderUniformLocation, TEXTURE_SAMPLERS_SHADER_UNIFORM_NAME, true);
    shader->locs[TEXTURE_SAMPLERS_SHADER_UNIFORM_LOCATION] = loc;

    GET_LOCATION_OF(GetShaderUniformLocation, PROJECTION_MATRIX_SHADER_UNIFORM_NAME, false);
    shader->locs[PROJECTION_MATRIX_SHADER_UNIFORM_LOCATION] = loc;

    GET_LOCATION_OF(GetShaderUniformLocation, VIEW_MATRIX_SHADER_UNIFORM_NAME, false);
    shader->locs[VIEW_MATRIX_SHADER_UNIFORM_LOCATION] = loc;

    GET_LOCATION_OF(GetShaderUniformLocation, MODEL_MATRIX_SHADER_UNIFORM_NAME, false);
    shader->locs[MODEL_MATRIX_SHADER_UNIFORM_LOCATION] = loc;
#undef GET_LOCATION_OF

    glUseProgram(0);
    return true;
}

void UnloadShader(Shader shader)
{
    MemoryFree(shader.locs);
    glDeleteProgram(shader.ID);
}

int GetShaderUniformLocation(Shader shader, const char *uniformName)
{
    return glGetUniformLocation(shader.ID, uniformName);
}

int GetShaderAttributeLocation(Shader shader, const char *attributeName)
{
    return glGetAttribLocation(shader.ID, attributeName);
}

void SetProjectionMatrixUniform(Shader shader, float *matrixData)
{
    SetShaderUniform(shader, shader.locs[PROJECTION_MATRIX_SHADER_UNIFORM_LOCATION],
            SHADER_UNIFORM_MAT4, (void *)matrixData, 1, false);
}

void SetViewMatrixUniform(Shader shader, float *matrixData)
{
    SetShaderUniform(shader, shader.locs[VIEW_MATRIX_SHADER_UNIFORM_LOCATION],
            SHADER_UNIFORM_MAT4, (void *)matrixData, 1, false);
}

void SetModelMatrixUniform(Shader shader, float *matrixData)
{
    SetShaderUniform(shader, shader.locs[MODEL_MATRIX_SHADER_UNIFORM_LOCATION],
            SHADER_UNIFORM_MAT4, (void *)matrixData, 1, false);
}

void SetShaderUniform(Shader shader, int location, int uniformType, const void *data, int count, bool transposeIfMatrix)
{
    glUseProgram(shader.ID);
    switch(uniformType) {
        case SHADER_UNIFORM_FLOAT:
            glUniform1fv(location, count, (const float *)data);
            break;
        case SHADER_UNIFORM_VEC2:
            glUniform2fv(location, count, (const float *)data);
            break;
        case SHADER_UNIFORM_VEC3:
            glUniform3fv(location, count, (const float *)data);
            break;
        case SHADER_UNIFORM_VEC4:
            glUniform4fv(location, count, (const float *)data);
            break;
        case SHADER_UNIFORM_UINT:
            glUniform1uiv(location, count, (const uint32_t *)data);
            break;
        case SHADER_UNIFORM_UVEC2:
            glUniform2uiv(location, count, (const uint32_t *)data);
            break;
        case SHADER_UNIFORM_UVEC3:
            glUniform3uiv(location, count, (const uint32_t *)data);
            break;
        case SHADER_UNIFORM_UVEC4:
            glUniform4uiv(location, count, (const uint32_t *)data);
            break;
        case SHADER_UNIFORM_INT:
            glUniform1iv(location, count, (const int *)data);
            break;
        case SHADER_UNIFORM_IVEC2:
            glUniform2iv(location, count, (const int *)data);
            break;
        case SHADER_UNIFORM_IVEC3:
            glUniform3iv(location, count, (const int *)data);
            break;
        case SHADER_UNIFORM_IVEC4:
            glUniform4iv(location, count, (const int *)data);
            break;
        case SHADER_UNIFORM_MAT3:
            glUniformMatrix3fv(location, count, transposeIfMatrix ? GL_TRUE : GL_FALSE, (const float *)data);
            break;
        case SHADER_UNIFORM_MAT4:
            glUniformMatrix4fv(location, count, transposeIfMatrix ? GL_TRUE : GL_FALSE, (const float *)data);
            break;
        case SHADER_UNIFORM_SAMPLER:
            glUniform1iv(location, count, data);
            break;
        default:
            break;
    }
    glUseProgram(0);
}
