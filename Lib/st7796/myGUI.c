#include "myLCD.h"
#include "myGUI.h"

/*****************************************************************************
 * @name       :void LCD_Clear(u16 Color)
 * @date       :2018-08-09 
 * @function   :Full screen filled LCD screen
 * @parameters :color:Filled color
 * @retvalue   :None
******************************************************************************/	
void GUI_Clear(uint16_t color){
  GUI_FillRectangle(0, 0, lcddev.width, lcddev.height, color);
}

void GUI_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
    // clipping
    if((x >= lcddev.width) || (y >= lcddev.height)) return;
    if((x + w - 1) >= lcddev.width) w = lcddev.width - x;
    if((y + h - 1) >= lcddev.height) h = lcddev.height - y;
    LCD_Select();
    LCD_SetWindow(x, y, x+w-1, y+h-1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET);
    for(y = h; y > 0; y--) {
        for(x = w; x > 0; x--) {
            HAL_SPI_Transmit(&TFT_SPI_PORT, data, sizeof(data), HAL_MAX_DELAY);
        }
    }
    LCD_Unselect();
}

void GUI_DrawPixel(uint16_t x, uint16_t y, uint16_t color){
    if((x >= lcddev.width) || (y >= lcddev.height)) return;
    LCD_Select();
    LCD_SetWindow(x, y, x+1, y+1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    LCD_WriteData(data, sizeof(data));
    LCD_Unselect();
}

static void GUI_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor){
    uint32_t i, b, j;
    LCD_SetWindow(x, y, x+font.width-1, y+font.height-1);
    for(i = 0; i < font.height; i++) {
      if (ch>=32 && ch<127) b = font.data[(ch - 32) * font.height + i];// латиница
      else b = font.data[(ch - 97) * font.height + i];  // кирилица 192 - 96 = 96
        for(j = 0; j < font.width; j++) {
            if((b << j) & 0x8000)  {
                uint8_t data[] = { color >> 8, color & 0xFF };
                LCD_WriteData(data, sizeof(data));
            } else {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                LCD_WriteData(data, sizeof(data));
            }
        }
    }
}

void GUI_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor){
  LCD_Select();
  while(*str){
    if(x + font.width >= lcddev.width){
      x = 0;
      y += font.height;
      if(y + font.height >= lcddev.height) break;
      if(*str == ' '){str++; continue;}// skip spaces in the beginning of the new line
    }
    GUI_WriteChar(x, y, *str, font, color, bgcolor);
    x += font.width;
    str++;
  }
  LCD_Unselect();
}
