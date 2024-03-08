#ifndef NOE_RENDERER_H_ 
#define NOE_RENDERER_H_ 

#include <stddef.h>
#include <stdint.h>

#define NG_VERTEX_BUFFER_LAYOUT_ATTRIBS_CAPACITY 16

typedef enum NgDataType {
    NG_INVALID_DATA_TYPE = 0,
    NG_FLOAT,
} NgDataType;

typedef struct NgVertexAttribInfo {
    const char *name;
    NgDataType type;
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

NgVertexBuffer *ngCreateVertexBuffer(const NgVertexBufferLayout *layout, const void *verticesData, uint32_t verticesCount);
void ngUpdateVertexBufferData(NgVertexBuffer *buffer, const void *verticesData, uint32_t verticesCount);
void ngDestroyVertexBuffer(NgVertexBuffer *buffer);

NgVertexBuffer *ngCreateVertexBuffer(const NgVertexBufferLayout *layout, const void *verticesData, uint32_t verticesCount);
void ngUpdateVertexBufferData(NgVertexBuffer *buffer, const void *verticesData, uint32_t verticesCount);
void ngDestroyVertexBuffer(NgVertexBuffer *buffer);

#endif // NOE_RENDERER_H_
