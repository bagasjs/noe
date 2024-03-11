#include "./src/noe.h"
#include "./src/nomath.h"
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600

void GLCheckLastError(int exitOnError);

bool UI_Button(int x, int y, const char *text, TextFont font, int fontSize)
{
    Vector2 position = { .x=(float)x, .y=(float)y };

    int defaultFontSize = 10;   // Default Font chars height in pixel
    if (fontSize < defaultFontSize) fontSize = defaultFontSize;
    int spacing = fontSize/defaultFontSize;

    float scaleFactor = (float)fontSize/font.baseSize;

    Rectangle rec;
    rec.x = x;
    rec.y = y;

    size_t textLength = StringLength(text);

    for(size_t i = 0; i < textLength; ++i) {
        int index = GetGlyphIndex(font, text[i]);

        if (font.glyphsInfo[index].advanceX == 0) rec.width += ((float)font.recs[index].width*scaleFactor + spacing);
        else rec.width += ((float)font.glyphsInfo[index].advanceX*scaleFactor + spacing);
    }

    rec.height = fontSize;

    DrawRectangle(RED, rec.x, rec.y, rec.width, rec.height);
    DrawTextEx(font, text, position, (float)fontSize, (float)spacing, BLACK, 15);

    Vector2 cpos = GetCursorPos();
    return (rec.x <= cpos.x && cpos.x <= (rec.x + rec.width))
        && (rec.y <= cpos.y && cpos.y <= (rec.y + rec.height));
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

    int x = 10;
    int y = 10;
    int world_speed = 5;

    TextFont font;
    LoadFontFromFile(&font, "./res/inter.ttf");

    while(!WindowShouldClose()) {
        PollInputEvents();
        if(IsKeyPressed(KEY_ESCAPE)) break;
        if(IsKeyDown(KEY_W)) y -= world_speed;
        if(IsKeyDown(KEY_A)) x -= world_speed;
        if(IsKeyDown(KEY_S)) y += world_speed;
        if(IsKeyDown(KEY_D)) x += world_speed;

        ClearBackground(WHITE);
        DrawTexture(texture, x, y, texture.width*10, texture.height*10);
        if(UI_Button(10, 10, "Play", font, 24)) {
            TRACELOG(LOG_INFO, "Clicked");
        }
        RenderFlush(shader);
        RenderPresent();
    }
    
    DeinitApplication();
}
