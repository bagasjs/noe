#ifndef NOGUI_H_
#define NOGUI_H_

#include "noe.h"

void GuiInit(TextFont font, float fontSize);
bool GuiButton(Vector2 pos, const char *text);
void GuiSlider(Vector2 pos, int min, int max, int *value);
void GuiLabel(Vector2 pos, Color color, const char *text, ...);
void GuiBegin(void);

#endif // NOGUI_H_
