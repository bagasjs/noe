#include "noe.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool LoadImageFromFile(Image *image, const char *filePath)
{
    int w, h, bpp;
    const stbi_uc *data = stbi_load(filePath, &w, &h, &bpp, 0);
    bool result = LoadImage(image, data, w, h, bpp);
    stbi_image_free((void *)data);
    return result;
}

bool LoadImage(Image *image, const uint8_t *data, uint32_t width, uint32_t height, uint32_t compAmount)
{
    if(!image || !data) {
        TRACELOG(LOG_INFO, "Failed to load image due to null argument");
        return false;
    }

    const size_t imageSize = sizeof(uint8_t) * width * height * compAmount;
    image->data = MemoryAlloc(imageSize);
    MemoryCopy(image->data, data, imageSize);
    image->compAmount = compAmount;
    image->width = width;
    image->height = height;
    return true;
}

void UnloadImage(Image image)
{
    if(image.data) 
        MemoryFree(image.data);
}

bool LoadTextureFromFile(Texture *texture, const char *filePath)
{
    Image image;
    LoadImageFromFile(&image,filePath);
    bool result = LoadTextureFromImage(texture, image);
    UnloadImage(image);
    return result;
}

bool LoadTextureFromImage(Texture *result, Image image)
{
    return LoadTexture(result, image.data, image.width, image.height, image.compAmount);
}

