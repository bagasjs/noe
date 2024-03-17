#include "nogui.h"
#include "noe.h"

typedef struct GuiState {
    bool initialized;
    TextFont font;
    float fontSize;
    int widgetCount;
} GuiState;

static GuiState GUI;

void GuiInit(TextFont font, float fontSize)
{
    if(GUI.initialized) return;
    GUI.font = font;
    GUI.initialized = true;
    GUI.fontSize = fontSize;
    GUI.widgetCount = 0;
}

void GuiBegin(void)
{
    GUI.widgetCount = 0;
}

bool GuiButton(Vector2 pos, const char *text)
{
    float padding = 4.0f;
    Vector2 size = GetTextDimension(GUI.font, text, GUI.fontSize);
    Rectangle shape;
    shape.x = pos.x;
    shape.y = pos.y;
    shape.width  = size.x + (padding * 2);
    shape.height = size.y + (padding * 2);
    pos.x += padding;
    pos.y += padding;

    Vector2 cursor = GetCursorPosition();
    bool is_hover = (shape.x <= cursor.x && cursor.x <= (shape.x + shape.width))
        && (shape.y <= cursor.y && cursor.y <= (shape.y + shape.height));

    Color background = HEX2COLOR(0x101010FF);
    if(is_hover) background = HEX2COLOR(0x202020FF);

    DrawRectangle(background, shape.x, shape.y, shape.width, shape.height);
    DrawTextEx(GUI.font, text, GUI.fontSize, pos, BLUE);

    GUI.widgetCount += 1;

    return is_hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void GuiSlider(Vector2 pos, int min, int max, int *value)
{
    Rectangle line;
    Rectangle slider;
    float scale = 5.0f;
    int range = max - min;
    line.x = pos.x;
    line.y = pos.y;
    line.width = range * scale;
    line.height = scale;

    Vector2 cursor = GetCursorPosition();
    bool is_hover = (line.x <= cursor.x && cursor.x <= (line.x + line.width)) && (line.y <= cursor.y && cursor.y <= (line.y + line.height));

    if(is_hover && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        *value = ((cursor.x - line.x) / line.width) * range + min;
        *value = CLAMP(*value, min, max);
    }

    slider.width = scale * 5;
    slider.height = scale * 5;
    slider.x = (((float)(*value) - min) / range) * line.width + line.x;
    slider.y = line.y - ((slider.height-line.height)*0.5f);

    if (CheckColissionPointRec(cursor, slider)) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            slider.x = cursor.x - slider.width * 0.5f;
            *value = ((cursor.x - line.x) / line.width) * range + min;
            *value = CLAMP(*value, min, max);
        }
    }

    DrawRectangle(WHITE, line.x, line.y, line.width, line.height);
    DrawRectangle(WHITE, slider.x, slider.y, slider.width, slider.height);

    GUI.widgetCount += 1;
}

void GuiLabel(Vector2 pos, Color color, const char *text, ...)
{
    DrawTextEx(GUI.font, text, GUI.fontSize, pos, color);

    GUI.widgetCount += 1;
}
