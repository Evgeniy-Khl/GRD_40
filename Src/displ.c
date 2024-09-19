#include "main.h"
#include "tft_proc.h"
#include "..\Lib\st7796\myLCD.h"
#include "..\Lib\st7796\myGUI.h"
#include "..\Lib\ds18b20\ds18b20.h"
#include "displ.h"
#include "rtc.h"
#include "nvRam.h"

extern char buffTFT[];
extern const char* setName[];
extern const char* modeName[];
extern const char* otherName[];
extern const char* relayName[];
extern const char* analogName[];
extern uint8_t displ_num, modeCell, ds18b20_amount, ds18b20_num, familycode[][8], newDate, errors, ticBeep;
extern uint16_t speedData[MAX_SPEED][2];
extern uint16_t fillScreen, Y_str, X_left, Y_top, Y_bottom, point_color, set[INDEX], mainTimer, tmrCounter, checkSmoke;
extern int8_t numSet, numDate;
extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;
extern union DataRam dataRAM;

extern int8_t relaySet[8],analogSet[2],analogOut[2];

int16_t min(int16_t a, int16_t b ) {
   return a < b ? a : b;
}

int16_t max(int16_t a, int16_t b ) {
   return a > b ? a : b;
}

//--------- ОСНОВНОЙ ЭКРАН ----------------------
void displ_0(void){
  Y_str = Y_top;  // 0
  const char* point[3] = {"  ","  ","  "};
  uint32_t curTime = sTime.Hours*3600 + sTime.Minutes*60 + sTime.Seconds;
  if(WORK){
    if(INSIDE) point[1] = "->";
    else if(set[TMR0]) point[2] = "->";
    else  point[0] = "->";
  }
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(3,1,40);// 3 колонки; одна строка; высота 40
    if(WORK|VENTIL|PURGING) drawButton(MAGENTA, 0, "СТОП");
    else drawButton(GREEN, 0, "ПУСК");
    drawButton(YELLOW, 1, "Керуван.");
    drawButton(CYAN, 2, "Налашув.");
  }
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  X_left = 15;
  if(WORK) GUI_WriteString(X_left, Y_str, " ON  ", Font_16x26, BLACK, GREEN);
  else if(VENTIL) GUI_WriteString(X_left, Y_str, "VENT ", Font_16x26, BLACK, YELLOW);
  else if(PURGING) GUI_WriteString(X_left, Y_str, "PURG ", Font_16x26, BLACK, CYAN);
  else {
    GUI_WriteString(X_left, Y_str, " OFF ", Font_16x26, YELLOW, RED);
    point_color = WHITE;
  }
  
  GUI_WriteString(120, Y_str, "РЕЖИМ:", Font_11x18, YELLOW, fillScreen);
  sprintf(buffTFT,"%8s", modeName[modeCell]);
  GUI_WriteString(190, Y_str, buffTFT, Font_11x18, BLACK, WHITE);
  Y_str = Y_str+26+10; //36
  //----------------------------------------------------------------------------------------------
//  sprintf(buffTFT,"%3u",checkSmoke);
//  GUI_WriteString(3, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
  //----------------------
  X_left = 35;
  if(errors & 0x01) GUI_WriteString(X_left, Y_str, "  ПОМИЛКА ДАТЧИКА N1  ", Font_11x18, YELLOW, RED);
  else if(errors & 0x04) GUI_WriteString(X_left, Y_str, "  ПЕРЕГРЫВ В КАМЕРI   ", Font_11x18, YELLOW, RED);
  else if(errors & 0x10) GUI_WriteString(X_left, Y_str, "ВЫДХЫЛЕННЯ ТЕМПЕРАТУРИ", Font_11x18, YELLOW, RED);
  else GUI_WriteString(X_left, Y_str, " ТЕМПЕРАТУРА В КАМЕРI ", Font_11x18, YELLOW, fillScreen);
  Y_str = Y_str+18+10; //64
  
  GUI_WriteString(15, Y_str, point[0], Font_16x26, WHITE, BLACK); // ->
  
  if(ds.pvT[0]<1000) sprintf(buffTFT,"%3.1f$ ",(float)ds.pvT[0]/10);
  else if(ds.pvT[0]<1270) sprintf(buffTFT,"%5d$ ", ds.pvT[0]/10);
  else sprintf(buffTFT," ---  ");
  GUI_WriteString(55, Y_str, buffTFT, Font_16x26, point_color, BLACK);
  sprintf(buffTFT,"%3i.0$ ", set[T0]);
  GUI_WriteString(175, Y_str, buffTFT, Font_16x26, BLACK, WHITE);
  Y_str = Y_str+26+10; //100
  //-------------------------------------------------------------------------------------------------
  X_left = 35;
  if(errors & 0x02) GUI_WriteString(X_left, Y_str, "  ПОМИЛКА ДАТЧИКА N2  ", Font_11x18, YELLOW, RED);
  else if(errors & 0x08) GUI_WriteString(X_left, Y_str, " ПЕРЕГРЫВ В ПРОДУКТI ", Font_11x18, YELLOW, RED);
  else GUI_WriteString(X_left, Y_str, "ТЕМПЕРАТУРА В ПРОДУКТI", Font_11x18, YELLOW, fillScreen);
  Y_str = Y_str+18+10; // 128
  
  GUI_WriteString(15, Y_str, point[1], Font_16x26, WHITE, BLACK); // ->
  
  if(ds.pvT[1]<1000) sprintf(buffTFT,"%3.1f$ ",(float)ds.pvT[1]/10);
  else if(ds.pvT[1]<1270) sprintf(buffTFT,"%5d$ ", ds.pvT[1]/10);
  else sprintf(buffTFT," ---  ");
  GUI_WriteString(55, Y_str, buffTFT, Font_16x26, WHITE, BLACK);
  sprintf(buffTFT,"%3i.0$ ", set[T1]);
  GUI_WriteString(175, Y_str, buffTFT, Font_16x26, BLACK, WHITE);
  Y_str = Y_str+26+10; // 164
  //-------------------------------------------------------------------------------------------
  X_left = 30;
  GUI_WriteString(X_left, Y_str, "   ТРИВАЛIСТЬ РЕЖИМУ   ", Font_11x18, YELLOW, fillScreen);
  Y_str = Y_str+18+10; // 192
  if(WORK|PURGING){
    sprintf(buffTFT,"%2s %02u:%02u:%02u ", point[2], sTime.Hours, sTime.Minutes, sTime.Seconds);
    GUI_WriteString(15, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
  }
  sprintf(buffTFT," %iгод.%02iхвл.", set[TMR0]/60, set[TMR0]%60);
  GUI_WriteString(165, Y_str, buffTFT, Font_11x18, BLACK, WHITE);
  Y_str = Y_str+18+10;  // 220
  
  if(modeCell>1){
    if(modeCell==2) GUI_WriteString(80, Y_str, "ВОЛОГИЙ ДАТЧИК", Font_11x18, YELLOW, fillScreen);
    else if(modeCell==3){
      if(errors & 0x20) GUI_WriteString(30, Y_str, "ДИМ НИЗЬКОЪ ТЕМПЕРАТУРИ", Font_11x18, YELLOW, RED);
      else GUI_WriteString(110, Y_str, "ДАТЧИК ДИМУ", Font_11x18, YELLOW, fillScreen);
    }
    Y_str = Y_str+18+10; // 248
    
    if(ds.pvT[2]<1000) sprintf(buffTFT,"%3.1f$ ",(float)ds.pvT[2]/10);
    else if(ds.pvT[2]<1270) sprintf(buffTFT,"%5d$ ", ds.pvT[2]/10);
    else sprintf(buffTFT," ---  ");
    GUI_WriteString(55, Y_str, buffTFT, Font_16x26, WHITE, BLACK);
    sprintf(buffTFT,"%3i.0$ ", set[T2]);
    GUI_WriteString(175, Y_str, buffTFT, Font_16x26, BLACK, WHITE);
    Y_str = Y_str+26+10;  // 284
  }
  if(VENTIL){
    if(errors & 0x80) GUI_WriteString(30, Y_str, "  НЕ ПРАЦЮЭ ВЕНТИЛЯТОР  ", Font_11x18, YELLOW, RED);
    else {
      sprintf(buffTFT,"%12s: %4i об/хвл.", setName[5], speedData[set[5]][0]);
      GUI_WriteString(10, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
      }
  }
  Y_str = Y_str+18+10;  // 312
  
  if(modeCell<3 && VENTIL && curTime>2 && curTime<12){
    ticBeep = 10;
    GUI_FillRectangle(40, Y_str, lcddev.width - 80, 56, RED);
    if(modeCell) GUI_WriteString(70, Y_str+5, "ЗАКРИЙТЕ ЗАСЛЫНКИ", Font_11x18, YELLOW, RED);
    else GUI_WriteString(65, Y_str+5, "ВЫДКРИЙТЕ ЗАСЛЫНКИ", Font_11x18, YELLOW, RED);
    GUI_WriteString(110, Y_str+35, "вентиляцыъ!", Font_11x18, YELLOW, RED);
  }
  else GUI_FillRectangle(40, Y_str, lcddev.width - 80, 56, fillScreen);
}

//-------------------------------- СТАН ВЫХОДІВ ------------------------------------------------------
void displ_1(void){
 uint8_t i, bit;
 char txt[10];
 uint16_t color_txt, color_box; 
    Y_str = Y_top;
    if(NEWBUTT){ NEWBUTT = OFF; 
      GUI_Clear(fillScreen);
      GUI_WriteString(X_left+60, Y_str,"СТАН ВИХОДЫВ",Font_11x18,YELLOW,fillScreen);
      initializeButtons(4,1,40);// четире колонки; одна строка; высота 40
      drawButton(BLUE, 0, "Вихыд");
      drawButton(YELLOW, 1, "Вибыр");
      drawButton(MAGENTA, 2, "+");
      drawButton(CYAN, 3, "-");
    }
//---- РЕЛЕЙНЫЕ ВЫХОДЫ ----
    Y_str = Y_str+18+5;
    for (i=0;i<6;i++){
        bit = 1<<i;
        sprintf(buffTFT,"%7s",relayName[i]);
        sprintf(txt," N%u: ",i+1);
        strcat(buffTFT,txt);
        if(relaySet[i]<0) strcat(buffTFT,"AUTO"); else if(relaySet[i]==1) strcat(buffTFT," ON "); else strcat(buffTFT," OFF");
        if(i == numSet){color_txt = BLACK; color_box = WHITE;} else {color_txt = WHITE; color_box = BLACK;}
        GUI_WriteString(X_left+10, Y_str, buffTFT, Font_11x18, color_txt, color_box);
        if(relayOut.value & bit) color_box=YELLOW; else color_box=GRAY; // ILI9341_COLOR565(128, 128, 128);
        GUI_FillRectangle(X_left+200,Y_str,30,18,color_box);
        Y_str = Y_str+18+5;
    }
    if(ds.pvT[1]<1000) sprintf(buffTFT,"N3=%3.1f$ ",(float)ds.pvT[2]/10);
    else if(ds.pvT[1]<1270) sprintf(buffTFT,"N3=%5d$ ", ds.pvT[2]/10);
    else sprintf(buffTFT,"N3= ---  ");
    GUI_WriteString(X_left+30, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
    sprintf(buffTFT,"Уст.%3i.0$ ", set[T2]);
    GUI_WriteString(X_left+150, Y_str, buffTFT, Font_11x18, BLACK, WHITE);
//---- АНАЛОГОВЫЕ ВЫХОДЫ ----
//    for (i=0;i<2;i++){
//        sprintf(buffTFT,"%7s",analogName[i]);
//        sprintf(txt," U%u: ",i+1);
//        strcat(buffTFT,txt);
//        if(analogSet[i]<0) strcat(buffTFT,"AUTO"); else {strcat(buffTFT,"SET:"); analogOut[i]=analogSet[i];}
//        sprintf(txt," %3u%% ",analogOut[i]);
//        strcat(buffTFT,txt);
//        if(i+6 == numSet){color_txt = BLACK; color_box = WHITE;} else {color_txt = WHITE; color_box = BLACK;}
//        GUI_WriteString(X_left+10,Y_str, buffTFT, Font_11x18, color_txt, color_box);
//        Y_str = Y_str+18+5;    
//    }
}

//--------- НАЛАШТУВАННЯ ----------------------------------
void displ_2(void){
  int8_t i;
  uint16_t color_txt, color_box;

  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,1,40);// четыре колонки; одна строка; высота 40
    drawButton(BLUE, 0, "Вихыд");
    drawButton(GREEN, 1, "v");
    drawButton(GREEN, 2, "^");
    drawButton(YELLOW, 3, "Вибыр");
  }
  Y_str = Y_str+10;
  for (i=-1; i<MAX_SET; i++){
    if(i==-1) sprintf(buffTFT,"       РЕЖИМ: %8s", modeName[modeCell]);
    else if(i==3) sprintf(buffTFT,"%12s: %iгод.%02iхвл.", setName[i], set[i]/60, set[i]%60);
    else if(i==5) sprintf(buffTFT,"%12s: %4i об/хвл.", setName[i], speedData[set[i]][0]);
    else if(i==6) sprintf(buffTFT,"%12s:", setName[i]);
    else {
      if(set[i]){
        sprintf(buffTFT,"%12s: %3i", setName[i], set[i]);
        if(i==4) strcat(buffTFT,"сек."); else strcat(buffTFT,"$ ");
      } 
      else sprintf(buffTFT,"%12s:", "не заданий");
    }
    if(i == numSet){color_txt = BLACK; color_box = WHITE;} else {color_txt = WHITE; color_box = BLACK;}
    GUI_WriteString(X_left, Y_str, buffTFT, Font_11x18, color_txt, color_box);
    Y_str = Y_str+18+5;
  }
}

//--------- ЗМІНА ТЕМПЕРАТУР ----------------------------------
void displ_3(void){
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,2,40);// четыре колонки; две строки; высота 40
    drawButton(BLUE, 0, "Отм.");
    drawButton(GREEN, 1, "+1");
    drawButton(GREEN, 2, "-1");
    drawButton(MAGENTA, 3, "Зап.");
    drawButton(YELLOW, 4, "+10");
    drawButton(YELLOW, 5, "-10");
    drawButton(CYAN, 6, "+50");
    drawButton(CYAN, 7, "-50");
  }
  Y_str = Y_str+50;
  sprintf(buffTFT,"%12s:", setName[numSet]);
  GUI_WriteString(X_left+20, Y_str, buffTFT, Font_11x18, WHITE, BLACK);

  if(numSet==3) sprintf(buffTFT,"%iгод.%02iхвл.", newval[numSet]/60, newval[numSet]%60);
  else if(numSet==4) sprintf(buffTFT,"%3iсекунд", newval[numSet]);
  else sprintf(buffTFT,"%3i$", newval[numSet]);
  if(numSet==3||numSet==4) GUI_WriteString(X_left+180, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
  else {
    Y_str = Y_str-4;
    GUI_WriteString(X_left+180, Y_str, buffTFT, Font_16x26, WHITE, BLACK);
  }
}

//--------- ЗМІНА РЕЖИМУ ----------------------------------
void displ_4(void){
  uint8_t i;
  uint16_t color_txt, color_box;
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,1,40);// четыре колонки; одна строка; высота 40
    drawButton(BLUE, 0, "Вихыд");
    drawButton(GREEN, 1, "v");
    drawButton(GREEN, 2, "^");
    drawButton(YELLOW, 3, "Вибыр");
  }
  Y_str = Y_str+10;
  for (i=0; i<MAX_MODE; i++){
    sprintf(buffTFT,"%10s", modeName[i]);
    if(i == newval[0]){color_txt = BLACK; color_box = WHITE;} else {color_txt = WHITE; color_box = BLACK;}
    GUI_WriteString(X_left, Y_str, buffTFT, Font_11x18, color_txt, color_box);
    Y_str = Y_str+18+5;
  }
}

//--------- IНШЕ ----------------------------------
void displ_5(void){
  uint8_t i;
  uint16_t color_txt, color_box;
  float flSet;
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,1,40);// четыре колонки; одна строка; высота 40
    drawButton(BLUE, 0, "Вихыд");
    drawButton(GREEN, 1, "v");
    drawButton(GREEN, 2, "^");
    drawButton(YELLOW, 3, "Вибыр");
  }
  Y_str = Y_str+10;
  for (i=0; i<MAX_OTHER; i++){
    sprintf(buffTFT,"%10s", otherName[i]);
    if(i<2){
      // если ВАРКА (modeCell==2) задается в mсек.[от 0.1сек. до 10 сек.] (период 10 mсек.)
      if(modeCell==2) flSet = (float)set[i+MAX_SET-1]/10; else flSet = set[i+MAX_SET-1];
      sprintf(buffTFT,"%12s:%2.1fсек.", otherName[i], flSet); // "ТАЙМ.ON","ТАЙМ.OFF"
    }
    else if(i==2) sprintf(buffTFT,"%12s:%3i$", otherName[i], set[i+MAX_SET-1]); // "АВАРИЯ"
    else if(i==3) sprintf(buffTFT,"%12s:%2.1f$", otherName[i], (float)set[i+MAX_SET-1]/10); // "ГИСТЕРЕЗ"
    else if(i<6) sprintf(buffTFT,"%12s:%3i", otherName[i], set[i+MAX_SET-1]); // "ОХОЛОДЖ.","ОСУШЕННЯ"
    else sprintf(buffTFT,"%12s:%4i", otherName[i], dataRAM.config.koff[i-6]); // "Prop","Integ"
    if(i == numSet){color_txt = BLACK; color_box = WHITE;} else {color_txt = WHITE; color_box = BLACK;}
    GUI_WriteString(X_left, Y_str, buffTFT, Font_11x18, color_txt, color_box);
    Y_str = Y_str+18+5;
  }
}

//--------- ЗМІНА IНШЕ ----------------------------------
void displ_6(void){
  float flSet;
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,2,40);// четыре колонки; две строки; высота 40
    drawButton(BLUE, 0, "Отм.");
    drawButton(GREEN, 1, "+1");
    drawButton(GREEN, 2, "-1");
    drawButton(MAGENTA, 3, "Зап.");
    drawButton(YELLOW, 4, "+10");
    drawButton(YELLOW, 5, "-10");
    drawButton(CYAN, 6, "+50");
    drawButton(CYAN, 7, "-50");
  }
  Y_str = Y_str+50;
  sprintf(buffTFT,"%12s:", otherName[numSet]);
  GUI_WriteString(X_left+20, Y_str, buffTFT, Font_11x18, WHITE, BLACK);

  if(numSet<2){
    // если ВАРКА (modeCell==2) задается в mсек.[от 0.1сек. до 10 сек.] (период 10 mсек.)
    if(modeCell==2) flSet = (float)newval[numSet]/10; else flSet = newval[numSet];
    sprintf(buffTFT,"%2.1fсек.", flSet); // "ТАЙМ.ON","ТАЙМ.OFF"
  }
  else if(numSet==2) sprintf(buffTFT,"%3i$", newval[numSet]); // "АВАРИЯ"
  else if(numSet==3) sprintf(buffTFT,"%1.1f$", (float)newval[numSet]/10); // "ГИСТЕРЕЗ"
  else sprintf(buffTFT,"%4i", newval[numSet]); // "ОХОЛОДЖ.","ОСУШЕННЯ","Prop","Integ"
  if(numSet<2) GUI_WriteString(X_left+180, Y_str, buffTFT, Font_11x18, WHITE, BLACK); // "ТАЙМ.ON","ТАЙМ.OFF"
  else {
    Y_str = Y_str-4;
    GUI_WriteString(X_left+180, Y_str, buffTFT, Font_16x26, WHITE, BLACK);
  }
}

//--------- вибір ШВИДКІСТІ обертання ----------------------------------
void displ_7(void){
  uint8_t i;
  uint16_t color_txt, color_box;
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,1,40);// четыре колонки; одна строка; высота 40
    drawButton(BLUE, 0, "Вихыд");
    drawButton(GREEN, 1, "v");
    drawButton(MAGENTA, 2, "Корек");
    drawButton(YELLOW, 3, "Вибыр");
  }
  Y_str = Y_str+10;
  for (i=0; i<MAX_SPEED; i++){
    sprintf(buffTFT,"%4u об/хвл.", speedData[i][0]);
    if(i == numSet){color_txt = BLACK; color_box = WHITE;} else {color_txt = WHITE; color_box = BLACK;}
    GUI_WriteString(X_left, Y_str, buffTFT, Font_11x18, color_txt, color_box);
    Y_str = Y_str+18+5;
  }
}

//--------- ЗМІНА ЗНАЧЕННЯ ШВИДКІСТІ обертання ----------------------------------
void displ_8(void){
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,1,40);// четыре колонки; одна строка; высота 40
    drawButton(BLUE, 0, "Отм.");
    drawButton(GREEN, 1, "+");
    drawButton(GREEN, 2, "-");
    drawButton(MAGENTA, 3, "Зап.");
  }
  Y_str = Y_str+50;
  sprintf(buffTFT,"Значення: 0x%03x выд.один.", newval[0]);
  GUI_WriteString(X_left+20, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
}

void display(void){
  switch (displ_num){
  	case 0: displ_0(); break;//- СТАН КАМЕРИ --
  	case 1: displ_1(); break;//- СТАН ВЫХОДІВ -
    case 2: displ_2(); break;//- НАЛАШТУВАННЯ -
    case 3: displ_3(); break;//- ЗМІНА ТЕМПЕРАТУР -
    case 4: displ_4(); break;//- ЗМІНА РЕЖИМУ -
    case 5: displ_5(); break;//- ІНШЕ -
    case 6: displ_6(); break;//- ЗМІНА ІНШЕ -
    case 7: displ_7(); break;//- вибір ШВИДКІСТІ обертання -
    case 8: displ_8(); break;//- ЗМІНА ЗНАЧЕННЯ ШВИДКІСТІ обертання -
  	default: displ_0();	break;//- СТАН КАМЕРИ -
  }
}
