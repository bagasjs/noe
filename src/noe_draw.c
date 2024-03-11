#include "noe.h"

void ClearBackground(Color color)
{
    RenderClear(COLOR2VECTOR4(color));
}

void DrawTriangle(Color color, int x1, int y1, int x2, int y2, int x3, int y3)
{
    RenderPutElement(RenderPutVertex((float)x1, (float)y1, 0.0f, COLOR2VECTOR4(color), 0.0f, 0.0f, -1.0f));
    RenderPutElement(RenderPutVertex((float)x2, (float)y2, 0.0f, COLOR2VECTOR4(color), 0.0f, 0.0f, -1.0f));
    RenderPutElement(RenderPutVertex((float)x3, (float)y3, 0.0f, COLOR2VECTOR4(color), 0.0f, 0.0f, -1.0f));
}

void DrawRectangle(Color color, int x, int y, uint32_t w, uint32_t h)
{
    int v0 = RenderPutVertex((float)x,  (float)y, 0.0f, COLOR2VECTOR4(color), 0.0f, 0.0f, -1.0f);
    int v1 = RenderPutVertex((float)x + (float)w,  (float)y, 0.0f, COLOR2VECTOR4(color), 0.0f, 0.0f, -1.0f);
    int v2 = RenderPutVertex((float)x + (float)w,  (float)y + (float)h, 0.0f, COLOR2VECTOR4(color), 0.0f, 0.0f, -1.0f);
    int v3 = RenderPutVertex((float)x,  (float)y + (float)h, 0.0f, COLOR2VECTOR4(color), 0.0f, 0.0f, -1.0f);
    RenderPutElement(v0);
    RenderPutElement(v1);
    RenderPutElement(v2);
    RenderPutElement(v2);
    RenderPutElement(v3);
    RenderPutElement(v0);
}

void DrawTexture(Texture texture, int x, int y, uint32_t w, uint32_t h)
{
    int textureIndex = RenderEnableTexture(texture);
    int tl = RenderPutVertex((float)x, (float)y, 0.0f,  
            0.0f, 0.0f, 0.0f, 0.0f, 
            0.0f, 0.0f, textureIndex);
    int tr = RenderPutVertex((float)x + (float)w, (float)y, 0.0f,  
            0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, textureIndex);
    int br = RenderPutVertex((float)x + (float)w, (float)y + (float)h,  
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, textureIndex);
    int bl = RenderPutVertex((float)x, (float)y + (float)h, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, textureIndex);
    RenderPutElement(tl);
    RenderPutElement(tr);
    RenderPutElement(br);
    RenderPutElement(br);
    RenderPutElement(bl);
    RenderPutElement(tl);
}

void DrawTextureEx(Texture texture, Rectangle src, Rectangle dst)
{
    int textureIndex = RenderEnableTexture(texture);
    int tl = RenderPutVertex((float)dst.x, (float)dst.y, 0.0f,  
            0.0f, 0.0f, 0.0f, 0.0f, 
            ((float)src.x)/texture.width, ((float)src.y)/texture.height, 
            textureIndex);
    int tr = RenderPutVertex((float)dst.x + (float)dst.width, (float)dst.y, 0.0f,  
            0.0f, 0.0f, 0.0f, 0.0f,
            ((float)src.x + (float)src.width)/texture.width, ((float)src.y)/texture.height, 
            textureIndex);
    int br = RenderPutVertex((float)dst.x + (float)dst.width, (float)dst.y + (float)dst.height,  
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            ((float)src.x + (float)src.width)/texture.width, ((float)src.y + (float)src.height)/texture.height, 
            textureIndex);
    int bl = RenderPutVertex((float)dst.x, (float)dst.y + (float)dst.height, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            ((float)src.x)/texture.width, ((float)src.y + (float)src.height)/texture.height, 
            textureIndex);
    RenderPutElement(tl);
    RenderPutElement(tr);
    RenderPutElement(br);
    RenderPutElement(br);
    RenderPutElement(bl);
    RenderPutElement(tl);
}
