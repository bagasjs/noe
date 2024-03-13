#include "./src/noe.h"
#include "./src/nomath.h"
#include <stdio.h>

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
    if(!LoadTextureFromFile(&texture, "./res/ikan.png")) {
        TRACELOG(LOG_FATAL, "Failed to load texture");
        return -1;
    }

    Matrix projection = MatrixOrthographic(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);
    SetProjectionMatrixUniform(shader, projection.elements);

    TextFont font;
    uint8_t *data = LoadFileData("./res/firacode.ttf", 0);
    LoadFont(&font, data, 64, 0, NULL);

    while(!WindowShouldClose()) {
        PollInputEvents();
        if(IsKeyPressed(KEY_ESCAPE)) break;

        ClearBackground(WHITE);
        DrawTextEx(font, "Hello, World", 32, (Vector2){.x=0, .y=0});
        RenderFlush(shader);
        RenderPresent();
    }
    
    DeinitApplication();
}
