#ifndef NOGL_H_ 
#define NOGL_H_ 

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define NG_SUPPORTED_OPENGL_MAJOR_VERSION 3
#define NG_SUPPORTED_OPENGL_MINOR_VERSION 3

#define NG_VERTEX_BUFFER_LAYOUT_ATTRIBS_CAPACITY 16

typedef enum NgShaderModuleType {
    NG_INVALID_SHADER_MODULE_TYPE = 0,
    NG_VERTEX_SHADER_MODULE,
    NG_FRAGMENT_SHADER_MODULE,
} NgShaderModuleType;

typedef enum NgShaderDataType {
    INVALID_SHADER_UNIFORM = 0,
    NG_SHADER_DT_FLOAT, NG_SHADER_DT_VEC2, NG_SHADER_DT_VEC3, NG_SHADER_DT_VEC4,
    NG_SHADER_DT_UINT, NG_SHADER_DT_UVEC2, NG_SHADER_DT_UVEC3, NG_SHADER_DT_UVEC4,
    NG_SHADER_DT_INT, NG_SHADER_DT_IVEC2, NG_SHADER_DT_IVEC3, NG_SHADER_DT_IVEC4,
    NG_SHADER_DT_MAT3, NG_SHADER_DT_MAT4, NG_SHADER_DT_SAMPLER,
} NgShaderDataType;

typedef struct NgVertexAttribInfo {
    const char *name;
    NgShaderDataType type;
} NgVertexAttribInfo;

typedef struct NgVertexBufferLayout {
    NgVertexAttribInfo attribs[NG_VERTEX_BUFFER_LAYOUT_ATTRIBS_CAPACITY];
    uint32_t amount;
} NgVertexBufferLayout;

typedef struct NgVertexBuffer NgVertexBuffer;

typedef struct NgIndexBuffer NgIndexBuffer;

typedef struct NgShaderModule NgShaderModule;

typedef struct NgShader NgShader;

typedef struct NgTexture NgTexture;

typedef struct NgContext NgContext;

NgVertexBuffer *ngCreateVertexBuffer(const NgVertexBufferLayout *layout, const void *verticesData, uint32_t verticesCount);
void ngUpdateVertexBufferData(NgVertexBuffer *buffer, const void *verticesData, uint32_t verticesCount);
void ngDestroyVertexBuffer(NgVertexBuffer *buffer);

NgVertexBuffer *ngCreateVertexBuffer(const NgVertexBufferLayout *layout, const void *verticesData, uint32_t verticesCount);
void ngUpdateVertexBufferData(NgVertexBuffer *buffer, const void *verticesData, uint32_t verticesCount);
void ngDestroyVertexBuffer(NgVertexBuffer *buffer);

NgShaderModule *ngCreateShaderModule(NgShaderModuleType type, const char *source);
void ngDestroyShaderModule(NgShaderModule *module);

NgShader *ngCreateShader(void);
void ngDestroyShader(NgShader *shader);
bool ngShaderReady(NgShader *shader);
NgShader ngAddShaderModule(NgShader *shader, NgShaderModule *module);
NgShader ngLinkShader(NgShader *shader);
void ngSetShaderUniform(NgShader *shader, const char *name, NgShaderDataType type, int count, const void *value, bool transpose);

NgTexture *ngCreateTexture(void);
void ngDestroyTexture(NgTexture *texture);

void ngDraw

#endif // NOGL_H_
