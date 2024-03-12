#ifndef NOGL_H_ 
#define NOGL_H_ 

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#if !defined(NG_ASSERT)
#include <assert.h>
#define NG_ASSERT(cond) assert(cond)
#endif

#if !defined(NG_MALLOC) && !defined(NG_FREE)
#include <stdlib.h>
#define NG_MALLOC(size) malloc(size)
#define NG_FREE(ptr)    free(ptr)
#endif

#if !defined(NG_MALLOC) || !defined(NG_FREE)
#error "Please define macro NG_MALLOC and NG_FREE"
#endif

#define NG_SUPPORTED_OPENGL_MAJOR_VERSION 3
#define NG_SUPPORTED_OPENGL_MINOR_VERSION 3
#define NG_VERTEX_BUFFER_LAYOUT_ATTRIBS_CAPACITY 16

typedef enum NgShaderModuleType {
    NG_INVALID_SHADER_MODULE_TYPE = 0,
    NG_VERTEX_SHADER_MODULE,
    NG_FRAGMENT_SHADER_MODULE,

    NG_COUNT_SHADER_MODULE_TYPE
} NgShaderModuleType;

typedef enum NgShaderDataType {
    NG_INVALID_SHADER_DT = 0,
    NG_SHADER_DT_FLOAT, NG_SHADER_DT_VEC2, NG_SHADER_DT_VEC3, NG_SHADER_DT_VEC4,
    NG_SHADER_DT_UINT, NG_SHADER_DT_UVEC2, NG_SHADER_DT_UVEC3, NG_SHADER_DT_UVEC4,
    NG_SHADER_DT_INT, NG_SHADER_DT_IVEC2, NG_SHADER_DT_IVEC3, NG_SHADER_DT_IVEC4,
    NG_SHADER_DT_MAT3, NG_SHADER_DT_MAT4, NG_SHADER_DT_SAMPLER,
    
    NG_COUNT_SHADER_DATA_TYPE,
} NgShaderDataType;

typedef enum NgTextureFormat {
    NG_TEXTURE_FORMAT_ALPHA = 0,
    NG_TEXTURE_FORMAT_RED_GREEN,
    NG_TEXTURE_FORMAT_RGB,
    NG_TEXTURE_FORMAT_RGBA,
} NgTextureFormat;

typedef struct NgVertexAttribInfo {
    const char *name;
    NgShaderDataType type;
} NgVertexAttribInfo;

typedef struct NgVertexBufferLayout {
    NgVertexAttribInfo attribs[NG_VERTEX_BUFFER_LAYOUT_ATTRIBS_CAPACITY];
    uint32_t count;
} NgVertexBufferLayout;

typedef struct NgVertexBuffer NgVertexBuffer;

typedef struct NgIndexBuffer NgIndexBuffer;

typedef struct NgShader NgShader;

typedef struct NgTexture NgTexture;

typedef struct NgContext NgContext;

NgVertexBuffer *ngCreateVertexBuffer(const NgVertexBufferLayout *layout, const void *verticesData, uint32_t verticesCount);
void ngUpdateVertexBufferData(NgVertexBuffer *buffer, const void *verticesData, uint32_t verticesCount);
void ngDestroyVertexBuffer(NgVertexBuffer *buffer);

NgIndexBuffer *ngCreateIndexBuffer(const uint32_t *indicesData, uint32_t indicesCount);
void ngUpdateIndexBufferData(NgIndexBuffer *buffer, const void *indicesData, uint32_t indicesCount);
void ngDestroyIndexBuffer(NgIndexBuffer *buffer);

NgShader *ngCreateShader(void);
void ngDestroyShader(NgShader *shader);
bool ngShaderReady(NgShader *shader);
void ngAddShaderModule(NgShader *shader, NgShaderModuleType moduleType, const char *source);
void ngLinkShader(NgShader *shader);
void ngSetShaderUniform(NgShader *shader, const char *name, NgShaderDataType type, int count, const void *value, bool transpose);

NgTexture *ngCreateTexture(const uint8_t *data, uint32_t width, uint32_t height, NgTextureFormat format);
void ngDestroyTexture(NgTexture *texture);

int ngGetDataTypeSize(NgShaderDataType type);
int ngGetDataTypeElementsAmount(NgShaderDataType type);

#endif // NOGL_H_
