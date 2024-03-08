#include "./src/noe.h"
#include "./src/nomath.h"
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

char *LoadFileText(const char *filePath);
bool LoadTextureFromFile(Texture *texture, const char *filePath, bool flipVerticallyOnLoad);
bool LoadShaderFromFile(Shader *shader, const char *vertSourceFilePath, const char *fragSourceFilePath);

#define WIDTH 800
#define HEIGHT 600

int main(void)
{
    SetupWindow("My Window", WIDTH, HEIGHT, WINDOW_SETUP_DEFAULT);
    if(!InitApplication()) return -1;
    Shader shader;
    if(!LoadShaderFromFile(&shader, "./res/main.vert", "./res/main.frag")) {
        TRACELOG(LOG_FATAL, "Failed to load shader");
        return -1;
    }

    Texture texture;
    if(!LoadTextureFromFile(&texture, "./res/ikan.png", false)) {
        TRACELOG(LOG_FATAL, "Failed to load texture");
        return -1;
    }

    Matrix projection = MatrixOrthographic(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);
    SetProjectionMatrixUniform(shader, projection.elements);

    int x = 10;
    int y = 10;
    int world_speed = 5;

    while(!WindowShouldClose()) {
        PollInputEvents();
        if(IsKeyPressed(KEY_ESCAPE)) break;
        if(IsKeyDown(KEY_W)) y -= world_speed;
        if(IsKeyDown(KEY_A)) x -= world_speed;
        if(IsKeyDown(KEY_S)) y += world_speed;
        if(IsKeyDown(KEY_D)) x += world_speed;

        ClearBackground(WHITE);
        DrawTexture(texture, x, y, texture.width*10, texture.height*10);
        RenderFlush(shader);
        RenderPresent();
    }
    
    DeinitApplication();
}

char *LoadFileText(const char *filePath)
{
    FILE *f = fopen(filePath, "r");
    if(!f) return NULL;

    fseek(f, 0L, SEEK_END);
    size_t filesz = ftell(f);
    fseek(f, 0L, SEEK_SET);
    char *result = MemoryAlloc(sizeof(char) * (filesz + 1));
    if(!result) {
        fclose(f);
        return NULL;
    }

    size_t read_length = fread(result, sizeof(char), filesz, f);
    result[read_length] = '\0';
    fclose(f);
    return result;
}

bool LoadTextureFromFile(Texture *texture, const char *filePath, bool flipVerticallyOnLoad)
{
    if(!texture) return false;
    if(!filePath) return false;

    int width, height, compAmount;
    stbi_set_flip_vertically_on_load(flipVerticallyOnLoad);
    stbi_uc *data = stbi_load(filePath, &width, &height, &compAmount, 0);
    if(!data) return false;
    bool result = LoadTexture(texture, data, width, height, compAmount);
    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);

    return result;
}

bool LoadShaderFromFile(Shader *shader, const char *vertSourceFilePath, const char *fragSourceFilePath)
{
    char *vertSource = LoadFileText(vertSourceFilePath);
    char *fragSource = LoadFileText(fragSourceFilePath);
    bool result = LoadShader(shader, vertSource, fragSource);
    MemoryFree(vertSource);
    MemoryFree(fragSource);
    return result;
}
