#include "nogl.h"
#include <glad/glad.h>

struct NgVertexBuffer {
    GLuint vboID, vaoID;
    GLuint vertexStride;
};

struct NgIndexBuffer {
    GLuint iboID;
};

struct NgShader {
    GLuint ID;
    GLboolean ready;
    GLuint modules[NG_COUNT_SHADER_MODULE_TYPE];
};

typedef struct NgShaderModuleTypeInfo {
    GLint glRepr;
} NgShaderModuleTypeInfo;

static NgShaderModuleTypeInfo shaderModuleTypeInfos[NG_COUNT_SHADER_MODULE_TYPE] = {
    [NG_INVALID_SHADER_MODULE_TYPE] = { .glRepr = -1 },
    [NG_VERTEX_SHADER_MODULE]       = { .glRepr = GL_VERTEX_SHADER },
    [NG_FRAGMENT_SHADER_MODULE]     = { .glRepr = GL_FRAGMENT_SHADER },
};

typedef struct NgShaderDataTypeInfo {
    int perItemSize;
    int itemsCount;
    int gltype;
    int normalize;
} NgShaderDataTypeInfo;

static NgShaderDataTypeInfo shaderDataTypeInfos[NG_COUNT_SHADER_DATA_TYPE] = {
    [NG_INVALID_SHADER_DT] = { .perItemSize = 0, .itemsCount = -1, },
    [NG_SHADER_DT_FLOAT] = { .perItemSize = sizeof(GLfloat), .itemsCount = 1, .gltype=GL_FLOAT, .normalize=GL_FALSE },
    [NG_SHADER_DT_VEC2]  = { .perItemSize = sizeof(GLfloat), .itemsCount = 2, .gltype=GL_FLOAT, .normalize=GL_FALSE },
    [NG_SHADER_DT_VEC3]  = { .perItemSize = sizeof(GLfloat), .itemsCount = 3, .gltype=GL_FLOAT, .normalize=GL_FALSE },
    [NG_SHADER_DT_VEC4]  = { .perItemSize = sizeof(GLfloat), .itemsCount = 4, .gltype=GL_FLOAT, .normalize=GL_FALSE },

    [NG_SHADER_DT_UINT]  = { .perItemSize = sizeof(GLuint), .itemsCount = 1, .gltype=GL_UNSIGNED_INT, .normalize=GL_TRUE },
    [NG_SHADER_DT_UVEC2] = { .perItemSize = sizeof(GLuint), .itemsCount = 2, .gltype=GL_UNSIGNED_INT, .normalize=GL_TRUE },
    [NG_SHADER_DT_UVEC3] = { .perItemSize = sizeof(GLuint), .itemsCount = 3, .gltype=GL_UNSIGNED_INT, .normalize=GL_TRUE },
    [NG_SHADER_DT_UVEC4] = { .perItemSize = sizeof(GLuint), .itemsCount = 4, .gltype=GL_UNSIGNED_INT, .normalize=GL_TRUE },

    [NG_SHADER_DT_INT]   = { .perItemSize = sizeof(GLint), .itemsCount = 1, .gltype=GL_INT, .normalize=GL_TRUE },
    [NG_SHADER_DT_IVEC2] = { .perItemSize = sizeof(GLint), .itemsCount = 2, .gltype=GL_INT, .normalize=GL_TRUE },
    [NG_SHADER_DT_IVEC3] = { .perItemSize = sizeof(GLint), .itemsCount = 3, .gltype=GL_INT, .normalize=GL_TRUE },
    [NG_SHADER_DT_IVEC4] = { .perItemSize = sizeof(GLint), .itemsCount = 4, .gltype=GL_INT, .normalize=GL_TRUE },

    [NG_SHADER_DT_MAT3] = { .perItemSize = sizeof(GLfloat), .itemsCount = (3*3), .gltype=GL_FLOAT, .normalize=GL_FALSE },
    [NG_SHADER_DT_MAT4] = { .perItemSize = sizeof(GLfloat), .itemsCount = (4*4), .gltype=GL_FLOAT, .normalize=GL_FALSE },
    [NG_SHADER_DT_SAMPLER] = { .perItemSize = sizeof(GLint), .itemsCount = 1, .gltype=GL_INT, .normalize=GL_FALSE },
};

NgVertexBuffer *ngCreateVertexBuffer(const NgVertexBufferLayout *layout, const void *verticesData, uint32_t verticesCount)
{
    NgVertexBuffer *buffer = NG_MALLOC(sizeof(NgVertexBuffer));
    NG_ASSERT(buffer && "Failed to allocate memory for NgVertexBuffer in ngCreateVertexBuffer()");

    buffer->vaoID = 0;
    buffer->vboID = 0;
    buffer->vertexStride = 0;
    glGenVertexArrays(1, &buffer->vaoID);
    glBindVertexArray(buffer->vaoID);

    for(uint32_t i = 0; i < layout->count; ++i) {
        NgVertexAttribInfo info = layout->attribs[i];
        uint32_t x = shaderDataTypeInfos[info.type].itemsCount * shaderDataTypeInfos[info.type].perItemSize;
        buffer->vertexStride += x;
    }

    glGenBuffers(1, &buffer->vboID);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->vboID);
    glBufferData(GL_ARRAY_BUFFER, buffer->vertexStride * verticesCount, verticesData, GL_DYNAMIC_DRAW);

    GLsizeiptr offset = 0;
    for(uint32_t i = 0; i < layout->count; ++i) {
        NgVertexAttribInfo info = layout->attribs[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 
                shaderDataTypeInfos[info.type].itemsCount, 
                shaderDataTypeInfos[info.type].gltype, 
                shaderDataTypeInfos[info.type].normalize, 
                buffer->vertexStride, 
                (void *)offset
                );
        offset += shaderDataTypeInfos[info.type].itemsCount * shaderDataTypeInfos[info.type].perItemSize;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return buffer;
}

void ngUpdateVertexBufferData(NgVertexBuffer *buffer, const void *verticesData, uint32_t verticesCount)
{
    NG_ASSERT(buffer && "Invalid argument `buffer` passed to ngUpdateVertexBufferData()");

    glBindBuffer(GL_ARRAY_BUFFER, buffer->vboID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, verticesCount*buffer->vertexStride, verticesData);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ngDestroyVertexBuffer(NgVertexBuffer *buffer)
{
    NG_ASSERT(buffer && "Invalid argument `buffer` passed to ngDestroyVertexBuffer()");

    glDeleteBuffers(1, &buffer->vboID);
    glDeleteVertexArrays(1, &buffer->vaoID);
    NG_FREE(buffer);
}

NgIndexBuffer *ngCreateIndexBuffer(const uint32_t *indicesData, uint32_t indicesCount)
{
    NgIndexBuffer *buffer = NG_MALLOC(sizeof(NgIndexBuffer));
    NG_ASSERT(buffer && "Failed to allocate memory for NgIndexBuffer in ngCreateIndexBuffer()");

    glGenBuffers(1, &buffer->iboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->iboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indicesCount, indicesData, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return buffer;
}

void ngUpdateIndexBufferData(NgIndexBuffer *buffer, const void *indicesData, uint32_t indicesCount)
{
    NG_ASSERT(buffer && "Invalid argument `buffer` passed to ngUpdateIndexBufferData()");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->iboID);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32_t)*indicesCount, indicesData);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ngDestroyIndexBuffer(NgIndexBuffer *buffer)
{
    NG_ASSERT(buffer && "Invalid argument `buffer` passed to ngDestroyIndexBuffer()");
    glDeleteBuffers(1, &buffer->iboID);
    NG_FREE(buffer);
}

NgShader *ngCreateShader(void)
{
    NgShader *shader = NG_MALLOC(sizeof(NgShader));
    NG_ASSERT(shader && "Failed to allocate memory for NgShader in ngCreateShader()");
    shader->ready = GL_FALSE;
    shader->ID = glCreateProgram();
    return shader;
}

bool ngShaderReady(NgShader *shader)
{
    return shader->modules[NG_VERTEX_SHADER_MODULE] && shader->modules[NG_FRAGMENT_SHADER_MODULE];
}

void ngDestroyShader(NgShader *shader)
{
    glDeleteProgram(shader->ID);
    NG_FREE(shader);
}

void ngAddShaderModule(NgShader *shader, NgShaderModuleType moduleType, const char *source)
{
    NG_ASSERT((moduleType == NG_VERTEX_SHADER_MODULE || moduleType == NG_FRAGMENT_SHADER_MODULE) 
            && "Invalid shader module type");

    GLint success;
    GLuint module = glCreateShader(shaderModuleTypeInfos[moduleType].glRepr);
    glShaderSource(module, 1, (const char **)&source, NULL);
    glCompileShader(module);
    glGetShaderiv(module, GL_COMPILE_STATUS, &success);
    NG_ASSERT(success && "Failed to compile and add OpenGL shader module");

    glAttachShader(shader->ID, module);
    shader->modules[moduleType] = module;
}

void ngLinkShader(NgShader *shader)
{
    NG_ASSERT(ngShaderReady(shader) && "To link an OpenGL shader you need the vertex and the fragment shader");
    GLint success;
    glLinkProgram(shader->ID);
    glGetProgramiv(shader->ID, GL_LINK_STATUS, &success);
    NG_ASSERT(success && "Failed to link OpenGL shader");
    for(int i = 0; i < NG_COUNT_SHADER_MODULE_TYPE; ++i) {
        if(shader->modules[i]) 
            glDeleteShader(shader->modules[i]);
    }
}

void ngSetShaderUniform(NgShader *shader, const char *name, NgShaderDataType type, int count, const void *data, bool transpose)
{
    glUseProgram(shader->ID);
    int location = glGetUniformLocation(shader->ID, name);
    NG_ASSERT(location > -1 && "Invalid OpenGL uniform location");

    switch(type) {
        case NG_SHADER_DT_FLOAT:
            glUniform1fv(location, count, (const float *)data);
            break;
        case NG_SHADER_DT_VEC2:
            glUniform2fv(location, count, (const float *)data);
            break;
        case NG_SHADER_DT_VEC3:
            glUniform3fv(location, count, (const float *)data);
            break;
        case NG_SHADER_DT_VEC4:
            glUniform4fv(location, count, (const float *)data);
            break;
        case NG_SHADER_DT_UINT:
            glUniform1uiv(location, count, (const uint32_t *)data);
            break;
        case NG_SHADER_DT_UVEC2:
            glUniform2uiv(location, count, (const uint32_t *)data);
            break;
        case NG_SHADER_DT_UVEC3:
            glUniform3uiv(location, count, (const uint32_t *)data);
            break;
        case NG_SHADER_DT_UVEC4:
            glUniform4uiv(location, count, (const uint32_t *)data);
            break;
        case NG_SHADER_DT_INT:
            glUniform1iv(location, count, (const int *)data);
            break;
        case NG_SHADER_DT_IVEC2:
            glUniform2iv(location, count, (const int *)data);
            break;
        case NG_SHADER_DT_IVEC3:
            glUniform3iv(location, count, (const int *)data);
            break;
        case NG_SHADER_DT_IVEC4:
            glUniform4iv(location, count, (const int *)data);
            break;
        case NG_SHADER_DT_MAT3:
            glUniformMatrix3fv(location, count, transpose ? GL_TRUE : GL_FALSE, (const float *)data);
            break;
        case NG_SHADER_DT_MAT4:
            glUniformMatrix4fv(location, count, transpose ? GL_TRUE : GL_FALSE, (const float *)data);
            break;
        case NG_SHADER_DT_SAMPLER:
            glUniform1iv(location, count, data);
            break;
        default:
            break;
    }
    glUseProgram(0);
}

NgTexture *ngCreateTexture(const uint8_t *data, uint32_t width, uint32_t height, NgTextureFormat format)
{
}

void ngDestroyTexture(NgTexture *texture)
{
}
