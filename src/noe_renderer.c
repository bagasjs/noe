/**
 *
 * A simple batch rendering system
 *
 */

#include "noe.h"
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NOMATH_IMPLEMENTATION
#include "nomath.h"

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
    Matrix projection;
    Matrix modelView;
    Matrix mvp;
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

void GLCheckLastError(int exitOnError) 
{
    GLenum err = GL_NO_ERROR;
    TRACELOG(LOG_DEBUG, "CHECKING OPENGL ERROR");
    while((err = glGetError()) != GL_NO_ERROR) {
        switch(err) {
            case GL_INVALID_ENUM: 
                TRACELOG(LOG_ERROR, "OPENGL ERROR: GL_INVALID_ENUM"); 
                if(!exitOnError) { return; } else { break; }
            case GL_INVALID_VALUE: 
                TRACELOG(LOG_ERROR, "OPENGL ERROR: GL_INVALID_VALUE"); 
                if(!exitOnError) { return; } else { break; }
            case GL_INVALID_OPERATION: 
                TRACELOG(LOG_ERROR, "OPENGL ERROR: GL_INVALID_OPERATION"); 
                if(!exitOnError) { return; } else { break; }
            case GL_OUT_OF_MEMORY: 
                TRACELOG(LOG_ERROR, "OPENGL ERROR: GL_OUT_OF_MEMORY"); 
                if(!exitOnError) { return; } else { break; }
            case GL_INVALID_FRAMEBUFFER_OPERATION: 
                TRACELOG(LOG_ERROR, "OPENGL ERROR: GL_INVALID_FRAMEBUFFER_OPERATION"); 
                if(!exitOnError) { return; } else { break; }
            default: 
                TRACELOG(LOG_ERROR, "OPENGL ERROR: UNKNOWN"); 
                if(!exitOnError) { return; } else { break; } 
        }
        if(exitOnError) ExitProgram(-1);
    }
    TRACELOG(LOG_DEBUG, "THERE'S NO ERROR");
}


static _BatchRendererState RENDERER = {0};

bool loadShaderDefault(void)
{
    const char *defaultVertexShaderSource = 
        "#version 330 core\n"
        "layout (location=0) in vec3 a_Position;\n"
        "layout (location=1) in vec4 a_Color;\n"
        "layout (location=2) in vec2 a_TexCoords;\n"
        "layout (location=3) in float a_TextureIndex;\n"
        "out vec4 v_Color;\n"
        "out vec2 v_TexCoords;\n"
        "out float v_TextureIndex;\n"
        "uniform mat4 u_MVP;\n"
        "void main() {\n"
        "    gl_Position = u_MVP * vec4(a_Position, 1.0);\n"
        "    v_Color = a_Color;\n"
        "    v_TexCoords = a_TexCoords;\n"
        "    v_TextureIndex = a_TextureIndex;\n"
        "}\n";

    const char *defaultFragmentShaderSource =
        "#version 330 core\n"
        "layout (location=0) out vec4 o_FragColor;\n"
        "in vec4 v_Color;\n"
        "in vec2 v_TexCoords;\n"
        "in float v_TextureIndex;\n"
        "uniform sampler2D u_Textures[8];\n"
        "void main() {\n"
        "    switch(int(v_TextureIndex)) {\n"
        "        case 0:\n"
        "            o_FragColor = texture(u_Textures[0], v_TexCoords.xy) * v_Color;\n"
        "            break;\n"
        "        case 1:\n"
        "            o_FragColor = texture(u_Textures[1], v_TexCoords.xy) * v_Color;\n"
        "            break;\n"
        "        case 2:\n"
        "            o_FragColor = texture(u_Textures[2], v_TexCoords.xy) * v_Color;\n"
        "            break;\n"
        "        case 3:\n"
        "            o_FragColor = texture(u_Textures[3], v_TexCoords.xy) * v_Color;\n"
        "            break;\n"
        "        case 4:\n"
        "            o_FragColor = texture(u_Textures[4], v_TexCoords.xy) * v_Color;\n"
        "            break;\n"
        "        case 5:\n"
        "            o_FragColor = texture(u_Textures[5], v_TexCoords.xy) * v_Color;\n"
        "            break;\n"
        "        case 6:\n"
        "            o_FragColor = texture(u_Textures[6], v_TexCoords.xy) * v_Color;\n"
        "            break;\n"
        "        case 7:\n"
        "            o_FragColor = texture(u_Textures[7], v_TexCoords.xy) * v_Color;\n"
        "            break;\n"
        "        default:\n"
        "            o_FragColor = v_Color;\n"
        "            break;\n"
        "    }\n"
        "}\n";

    return LoadShader(&RENDERER.defaultShader, defaultVertexShaderSource, defaultFragmentShaderSource);
}

bool initBatchRenderer(const ApplicationConfig *config)
{
    gladLoadGLLoader((GLADloadproc)GLGetProcAddress);

    RENDERER.config.supportVAO = config->opengl.useCoreProfile;
    RENDERER.vertices.count = 0;
    RENDERER.elements.count = 0;
    RENDERER.activeTextureIDs.count = 0;
    RENDERER.modelView = MatrixCreate(1.0f);
    RENDERER.projection = MatrixCreate(1.0f);
    RENDERER.mvp = MatrixCreate(1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(!loadShaderDefault()) {
        TRACELOG(LOG_FATAL, "Failed to load default shader");
        return false;
    }

    if(RENDERER.config.supportVAO) {
        RENDERER.config.supportVAO = true;
        glGenVertexArrays(1, &RENDERER.vaoID);
    } else {
        RENDERER.config.supportVAO = false;
    }

    glGenBuffers(1, &RENDERER.vboID);
    glBindBuffer(GL_ARRAY_BUFFER, RENDERER.vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RENDERER.vertices.data), NULL, GL_DYNAMIC_DRAW);

    if(RENDERER.config.supportVAO) {
        glBindVertexArray(RENDERER.vaoID);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(_RenderVertex), (void*)offsetof(_RenderVertex, pos));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(_RenderVertex), (void*)offsetof(_RenderVertex, color));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(_RenderVertex), (void*)offsetof(_RenderVertex, texCoords));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(_RenderVertex), (void*)offsetof(_RenderVertex, textureIndex));
    }

    glGenBuffers(1, &RENDERER.eboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RENDERER.eboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(RENDERER.elements.data), NULL, GL_DYNAMIC_DRAW);

    return true;
}

void deinitBatchRenderer(void)
{
    UnloadShader(RENDERER.defaultShader);
    glDeleteBuffers(1, &RENDERER.eboID);
    glDeleteBuffers(1, &RENDERER.vboID);
    if(RENDERER.config.supportVAO) glDeleteVertexArrays(1, &RENDERER.vaoID);
}

void RenderPresent(void)
{
    GLSwapBuffers();
}

void RenderClear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderFlush(Shader shader)
{
    if(RENDERER.config.supportVAO) glBindVertexArray(RENDERER.vaoID);
    if(RENDERER.vertices.count > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, RENDERER.vboID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RENDERER.vertices.data[0])*RENDERER.vertices.count, 
                (void *)RENDERER.vertices.data);
    }
    if(RENDERER.elements.count > 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RENDERER.eboID);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(RENDERER.elements.data[0])*RENDERER.elements.count, 
                (void *)RENDERER.elements.data);
    }
    if(RENDERER.config.supportVAO) glBindVertexArray(0);

    glUseProgram(shader.ID);
    if(RENDERER.config.supportVAO) glBindVertexArray(RENDERER.vaoID);
    else {
        glBindBuffer(GL_ARRAY_BUFFER, RENDERER.vboID); 
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

        if(RENDERER.elements.count > 0) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RENDERER.eboID);
    }

    int textureUnits[MAXIMUM_BATCH_RENDERER_ACTIVE_TEXTURES] = {0};
    for(int i = 0; i < MAXIMUM_BATCH_RENDERER_ACTIVE_TEXTURES; ++i) {
        textureUnits[i] = i;
    }

    for(int i = 0; i < (int)RENDERER.activeTextureIDs.count; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, RENDERER.activeTextureIDs.data[i]);
    }

    glUniformMatrix4fv(shader.locs[MVP_MATRIX_SHADER_UNIFORM_LOCATION], 1, GL_FALSE, RENDERER.mvp.elements);
    glUniform1iv(shader.locs[TEXTURE_SAMPLERS_SHADER_UNIFORM_LOCATION],
            MAXIMUM_BATCH_RENDERER_ACTIVE_TEXTURES, textureUnits);

    if(RENDERER.elements.count > 0) {
        glDrawElements(GL_TRIANGLES, RENDERER.elements.count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, RENDERER.vertices.count);
    }

    if(RENDERER.config.supportVAO) glBindVertexArray(0);
    else {
        if(RENDERER.elements.count > 0) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glUseProgram(0);

    RENDERER.vertices.count = 0;
    RENDERER.elements.count = 0;
    RENDERER.activeTextureIDs.count = 0;
}

void RenderViewport(int x, int y, uint32_t width, uint32_t height)
{
    glViewport(x, y, width, height);
}

int RenderEnableTexture(Texture texture)
{
    for(int i = 0; i < MAXIMUM_BATCH_RENDERER_ACTIVE_TEXTURES; ++i) {
        if(RENDERER.activeTextureIDs.data[i] == texture.ID) 
            return i;
    }

    if(RENDERER.activeTextureIDs.count + 1 < MAXIMUM_BATCH_RENDERER_ACTIVE_TEXTURES) {
        int index = RENDERER.activeTextureIDs.count;
        RENDERER.activeTextureIDs.data[index] = texture.ID;
        RENDERER.activeTextureIDs.count += 1;
        return index;
    } else {
        return -1;
    }
}

void _DumpVertex(int i, const _RenderVertex *v)
{
    TRACELOG(LOG_DEBUG, "Vertex[%d](.x=%.4f,.y=%.4f,.z=%.4f,.r=%.4f,.g=%.4f,.b=%.4f,.a=%.4f,.u=.%.4f,.v=%.4f,.tid=%.4f",
            i,
            v->pos.x, v->pos.y, v->pos.z,
            v->color.r, v->color.g, v->color.b, v->color.a,
            v->texCoords.u, v->texCoords.v, v->textureIndex);
}

int RenderPutVertex(float x, float y, float z, float r, float g, float b, float a, float u, float v, int textureIndex)
{
    int index = RENDERER.vertices.count;
    _RenderVertex *vertex = &RENDERER.vertices.data[index];
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
    // _DumpVertex(index, vertex);

    RENDERER.vertices.count += 1;
    return index;
}

void RenderPutElement(int vertexIndex)
{
    RENDERER.elements.data[RENDERER.elements.count] = vertexIndex;
    RENDERER.elements.count += 1;
}

bool LoadImage(Image *image, const uint8_t *data, uint32_t width, uint32_t height, uint32_t compAmount)
{
    if(!image || !data) {
        TRACELOG(LOG_INFO, "Failed to load image due to null argument");
        return false;
    }

    const size_t imageSize = sizeof(uint8_t) * width * height * compAmount;
    image->data = NOE_MALLOC(imageSize);
    MemoryCopy(image->data, data, imageSize);
    image->compAmount = compAmount;
    image->width = width;
    image->height = height;
    return true;
}

bool LoadImageFromFile(Image *image, const char *filePath)
{
    int w, h, bpp;
    size_t fileSize = 0;
    uint8_t *fileData = LoadFileData(filePath, &fileSize);
    if(!fileData) {
        TRACELOG(LOG_ERROR, "Failed to load image `%s` file data", filePath);
        return false;
    }

    const stbi_uc *data = stbi_load_from_memory(fileData, fileSize, &w, &h, &bpp, 0);
    bool result = LoadImage(image, data, w, h, bpp);
    stbi_image_free((void *)data);
    UnloadFileData(fileData);
    return result;
}

void UnloadImage(Image image)
{
    if(image.data) 
        NOE_FREE(image.data);
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

    int internalFormat = GL_RED;
    int format = GL_RGBA8;
    if(compAmount == 4) {
        internalFormat = GL_RGBA8;
        format = GL_RGBA;
    } else if(compAmount == 3) {
        internalFormat = GL_RGB8;
        format = GL_RGB;
    } else if(compAmount == 2) {
        internalFormat = GL_RG8;
        format = GL_RG;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    texture->height = height;
    texture->width = width;
    texture->compAmount = compAmount;
    TRACELOG(LOG_INFO, "Loaded texture with id %u", texture->ID);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

bool LoadTextureFromFile(Texture *texture, const char *filePath)
{
    Image image;
    if(!LoadImageFromFile(&image,filePath)) {
        TRACELOG(LOG_ERROR, "Failed to load image to create texture");
        return false;
    }
    bool result = LoadTexture(texture, image.data, image.width, image.height, image.compAmount);
    UnloadImage(image);
    return result;
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
    shader->locs = NOE_MALLOC(sizeof(int) * MAXIMUM_SHADER_LOCS);

#define GET_LOCATION_OF(func, name, mandatory) \
    do { \
        loc = func(*shader, (name)); \
        if(loc < 0) { \
            TRACELOG((mandatory) ? LOG_ERROR : LOG_WARNING, "Failed to find location of %s", name); \
            if(mandatory) { \
                NOE_FREE(shader->locs); \
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

    GET_LOCATION_OF(GetShaderUniformLocation, MVP_MATRIX_SHADER_UNIFORM_NAME, false);
    shader->locs[MVP_MATRIX_SHADER_UNIFORM_LOCATION] = loc;
#undef GET_LOCATION_OF

    glUseProgram(0);
    return true;
}

void UnloadShader(Shader shader)
{
    NOE_FREE(shader.locs);
    glDeleteProgram(shader.ID);
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

bool LoadShaderFromFile(Shader *shader, const char *vertSourceFilePath, const char *fragSourceFilePath)
{
    char *vertSource = LoadFileText(vertSourceFilePath, NULL);
    char *fragSource = LoadFileText(fragSourceFilePath, NULL);
    bool result = LoadShader(shader, vertSource, fragSource);
    UnloadFileText(vertSource);
    UnloadFileText(fragSource);
    return result;
}

int GetShaderUniformLocation(Shader shader, const char *uniformName)
{
    return glGetUniformLocation(shader.ID, uniformName);
}

int GetShaderAttributeLocation(Shader shader, const char *attributeName)
{
    return glGetAttribLocation(shader.ID, attributeName);
}

Shader GetDefaultShader(void)
{
    return RENDERER.defaultShader;
}

void SetProjectionMatrix(Matrix projection)
{
    RENDERER.projection = projection;
    RENDERER.mvp = MatrixDot(RENDERER.projection, RENDERER.modelView);
}

void SetModelViewMatrix(Matrix modelView)
{
    RENDERER.modelView = modelView;
    RENDERER.mvp = MatrixDot(RENDERER.projection, RENDERER.modelView);
}

void ClearBackground(Color color)
{
    Vector4 vc = COLOR2VECTOR4(color);
    RenderClear(vc.r, vc.g, vc.b, vc.a);
}

void DrawTriangle(Color color, float x1, float y1, float x2, float y2, float x3, float y3)
{
    Vector4 vc = COLOR2VECTOR4(color);
    RenderPutElement(RenderPutVertex(x1, y1, 0.0f, vc.r, vc.g, vc.b, vc.a, 0.0f, 0.0f, -1.0f));
    RenderPutElement(RenderPutVertex(x2, y2, 0.0f, vc.r, vc.g, vc.b, vc.a, 0.0f, 0.0f, -1.0f));
    RenderPutElement(RenderPutVertex(x3, y3, 0.0f, vc.r, vc.g, vc.b, vc.a, 0.0f, 0.0f, -1.0f));
}

void DrawRectangle(Color color, float x, float y, float w, float h)
{
    Vector4 vc = COLOR2VECTOR4(color);
    int v0 = RenderPutVertex(x,  y, 0.0f, vc.r, vc.g, vc.b, vc.a, 0.0f, 0.0f, -1.0f);
    int v1 = RenderPutVertex(x + w,  y, 0.0f, vc.r, vc.g, vc.b, vc.a, 0.0f, 0.0f, -1.0f);
    int v2 = RenderPutVertex(x + w,  y + h, 0.0f, vc.r, vc.g, vc.b, vc.a, 0.0f, 0.0f, -1.0f);
    int v3 = RenderPutVertex(x,  y + h, 0.0f, vc.r, vc.g, vc.b, vc.a, 0.0f, 0.0f, -1.0f);
    RenderPutElement(v0);
    RenderPutElement(v1);
    RenderPutElement(v2);
    RenderPutElement(v2);
    RenderPutElement(v3);
    RenderPutElement(v0);
}

void DrawTexture(Texture texture, float x, float y, float w, float h)
{
    int textureIndex = RenderEnableTexture(texture);
    int tl = RenderPutVertex(x, y, 0.0f,  
            1.0f, 1.0f, 1.0f, 1.0f, 
            0.0f, 0.0f, textureIndex);
    int tr = RenderPutVertex(x + w, y, 0.0f,  
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, textureIndex);
    int br = RenderPutVertex(x + w, y + h, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, textureIndex);
    int bl = RenderPutVertex(x, y + h, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, textureIndex);
    RenderPutElement(tl);
    RenderPutElement(tr);
    RenderPutElement(br);
    RenderPutElement(br);
    RenderPutElement(bl);
    RenderPutElement(tl);
}

void DrawTextureEx(Texture texture, Rectangle src, Rectangle dst, Color tint)
{
    int textureIndex = RenderEnableTexture(texture);
    float width  = (float)texture.width;
    float height = (float)texture.height;
    Vector4 vc = COLOR2VECTOR4(tint);
    int tl = RenderPutVertex((float)dst.x, (float)dst.y, 0.0f,  
            vc.r, vc.g, vc.b, vc.a,
            src.x/width, src.y/height, 
            textureIndex);
    int tr = RenderPutVertex((float)dst.x + (float)dst.width, (float)dst.y, 0.0f,  
            vc.r, vc.g, vc.b, vc.a,
            (src.x + src.width)/texture.width, src.y/texture.height, 
            textureIndex);
    int br = RenderPutVertex((float)dst.x + (float)dst.width, (float)dst.y + (float)dst.height, 0.0f,
            vc.r, vc.g, vc.b, vc.a,
            (src.x + src.width)/texture.width, (src.y + src.height)/texture.height, 
            textureIndex);
    int bl = RenderPutVertex((float)dst.x, (float)dst.y + (float)dst.height, 0.0f,
            vc.r, vc.g, vc.b, vc.a,
            (src.x)/texture.width, (src.y + src.height)/texture.height, 
            textureIndex);
    RenderPutElement(tl);
    RenderPutElement(tr);
    RenderPutElement(br);
    RenderPutElement(br);
    RenderPutElement(bl);
    RenderPutElement(tl);
}
