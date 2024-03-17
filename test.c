#include "./src/noe.h"
#include "./src/nomath.h"

#define WIDTH 800
#define HEIGHT 600

typedef Color UI_Background;

typedef struct UI_Style {
    UI_Background background;
    TextFont font;
    float fontSize;
    float padding;
} UI_Style;

bool UI_Button(Vector2 pos, const char *text, UI_Style style)
{
    Vector2 size = GetTextDimension(style.font, text, style.fontSize);
    Rectangle r;
    r.x = pos.x;
    r.y = pos.y;
    r.width  = size.x + style.padding*2;
    r.height = size.y + style.padding*2;
    DrawRectangle(style.background, r.x, r.y, r.width, r.height);

    pos.x += style.padding;
    pos.y += style.padding;
    DrawTextEx(style.font, text, style.fontSize, pos, BLUE);

    Vector2 cursor = GetCursorPosition();
    bool is_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
        && (r.x <= cursor.x && cursor.x <= (r.x + r.width))
        && (r.y <= cursor.y && cursor.y <= (r.y + r.height));

    return is_pressed;
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
    if(!LoadTextureFromFile(&texture, "./res/ikan.jpg")) {
        TRACELOG(LOG_FATAL, "Failed to load texture");
        return -1;
    }

    Matrix projection = MatrixOrthographic(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);
    SetProjectionMatrix(projection);

    TextFont font;
    uint8_t *data = LoadFileData("./res/inter.ttf", 0);
    LoadFont(&font, data, 64, 0, NULL);
    UnloadFileData(data);

    UI_Style style;
    style.background = RED;
    style.font = font;
    style.fontSize = 32.0f;
    style.padding = 4.0f;

    while(!WindowShouldClose()) {
        PollInputEvents();
        if(IsKeyPressed(KEY_ESCAPE)) break;

        ClearBackground(HEX2COLOR(0x202020FF));

        if(UI_Button((Vector2){.x=10.0f, .y=10.0f}, "CLICK ME!", style)) 
            style.background = COLOR2HEX(style.background) != COLOR2HEX(RED) ? RED : GREEN;

        DrawTexture(texture, 200, 200, (float)texture.width, (float)texture.height);

        RenderFlush(shader);
        RenderPresent();
    }
    
    DeinitApplication();
}
