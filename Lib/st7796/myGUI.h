
#ifndef __MYGUI_H
#define __MYGUI_H

#include "main.h"
#include "..\Lib\st7796\fonts.h"

void GUI_Clear(uint16_t color);
void GUI_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void GUI_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void GUI_DrawPixel(uint16_t x, uint16_t y, uint16_t color);

#endif // __MYGUI_H
