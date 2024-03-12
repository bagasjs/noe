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

    int x = 10;
    int y = 10;
    int world_speed = 5;

    TextFont font;
    LoadFontFromFile(&font, "./res/firacode.ttf");

    while(!WindowShouldClose()) {
        PollInputEvents();
        if(IsKeyPressed(KEY_ESCAPE)) break;
        if(IsKeyDown(KEY_W)) y -= world_speed;
        if(IsKeyDown(KEY_A)) x -= world_speed;
        if(IsKeyDown(KEY_S)) y += world_speed;
        if(IsKeyDown(KEY_D)) x += world_speed;

        ClearBackground(WHITE);
        DrawTexture(texture, x, y, texture.width*10, texture.height*10);
        DrawText(font, "Mantap jiwa", 0, 0, 24, RED);
        RenderFlush(shader);
        RenderPresent();
    }
    
    DeinitApplication();
}
