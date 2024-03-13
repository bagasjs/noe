#include "./src/noe.h"
#include "./src/nomath.h"

#define WIDTH 800
#define HEIGHT 600

bool UI_Button(Vector2 pos, TextFont font, const char *text, float fontSize)
{
    Vector2 size = GetTextDimension(font, text, fontSize);
    DrawRectangle(RED, pos.x, pos.y, size.x, size.y);
    DrawTextEx(font, text, fontSize, pos);
    return false;
}

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
        UI_Button((Vector2){.x=10.0f, .y=10.0f}, font, "Click me!", 16);
        RenderFlush(shader);
        RenderPresent();
    }
    
    DeinitApplication();
}
