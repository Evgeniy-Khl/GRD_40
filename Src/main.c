/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include "..\Lib\st7796\myLCD.h"
#include "..\Lib\st7796\myGUI.h"
#include "..\Lib\Touch\XPT2046_touch.h"
#include "..\Lib\ds18b20\ds18b20.h"
#include "tft_proc.h"
#include "procedure.h"
#include "displ.h"
#include "nvRam.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

char buffTFT[40];
const char* modeName[4]={"СУШЫННЯ","ОБЖАРКА","ВАРЫННЯ","КОПЧЕННЯ"};
const char* setName[MAX_SET]={"t КАМЕРИ","t ПРОДУКТА","t ВОЛОГОСТЫ","ТРИВАЛЫСТЬ","ПРОДУВАННЯ","ШВИДКЫСТЬ","ЫНШЕ"};
const char* otherName[MAX_OTHER]={"ТАЙМ.ON","ТАЙМ.OFF","АВАРЫЯ","ГЫСТЕРЕЗ","ОХОЛОДЖ.","ОСУШЕННЯ","Prop","Integ"};
const char* relayName[6]={"ПЫД","НАГРЫВ","ТАЙМЕР","ВОЛОГА","ЕЛЕКТРО","t ДИМА"};
const char* analogName[2]={"ВЕНТИЛ.","ЫНШЕ"};
//        2.00V        3.15V        4.30V        5.45V        6.60V        7.75V        8.90V        10.00V
//={{1000,0x2F4},{1200,0x4A6},{1400,0x658},{1600,0x80A},{1800,0x9BC},{2000,0xB6E},{2200,0xD20},{2400,0xFFF}};//d=434->1.15V
//={{1000,0x2F4},{1200,0x4A6},{1400,0x655},{1600,0x804},{1800,0x9B6},{2000,0xB65},{2200,0xD14},{2400,0xFFF}};//d=434+коррекция
struct Ds ds;
uint16_t speedData[MAX_SPEED][2];
int16_t pvTH, pvRH, tmrCounter;
uint16_t set[INDEX], touch_x, touch_y, Y_str, X_left, Y_top, Y_bottom, fillScreen, point_color, checkTime, checkSmoke;
uint8_t displ_num=0, modeCell, oldNumSet, buttonAmount, lost, errors;
uint8_t timer10ms, tmrVent, ticBeep, pwTriac, invers;
uint8_t familycode[MAX_SENSOR][8];
int8_t ds18b20_amount, numSet=0, resetDispl=0;
int8_t relaySet[8]={-1,-1,-1,-1,-1,-1,-1,-1};
int8_t analogSet[2]={-1,-1};
uint8_t analogOut[2]={0};
union Byte portFlag;
union Byte relayOut;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM1_Init(void);
static void MX_CRC_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//-------- Обратный вызов с истекшим периодом --------------
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  if(htim->Instance == TIM1){ //check if the interrupt comes from TIM1 (10 ms)
    checkTime++; timer10ms++;
    if(pwTriac) --pwTriac; else {
      TRIAC = OFF;                    // отключить (SSR-25DA)
      invers = ~relayOut.value;
      HAL_I2C_Master_Transmit(&hi2c1,0x4E,&invers,1,1000);
    }
    if(ticBeep){ --ticBeep; HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);}// индикация нажатия
    else {HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);}
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  int16_t i16;
  uint16_t u16;
//  uint8_t temp=0, pvspeed=0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_TIM1_Init();
  MX_CRC_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  u16 = sendToI2c(0);//  отключение вентилятора
  
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);

  Y_bottom=lcddev.height-22; Y_str = 5;
  fillScreen = BLACK; point_color = WHITE;
  
  LCD_Init(USE_VERTICAL1);
  GUI_Clear(fillScreen);
  if((lcddev.dir&1)==0) X_left = 20; else X_left = 100;
  GUI_WriteString(35, Y_str, "GRD Max", Font_16x26, WHITE, fillScreen);
  GUI_WriteString(165, Y_str+5, " v 2.8", Font_11x18, WHITE, fillScreen);
  Y_str = Y_str+18+35;
  
  i16 = initData();
  ds18b20_port_init();      // линия 1-Wire
  ds18b20_checkSensor(3);   // check DS18B20 sensors ?????????????????????????????????????
 
  switch (i16){
  	case 0: GUI_WriteString(5, Y_str, "Ыныцыалызацыя успышна.", Font_11x18, GREEN, BLACK);	break;
  	case 1: GUI_WriteString(5, Y_str, "Первинна ыныцыалызацыя.", Font_11x18, YELLOW, BLACK);	break;
    case 3: GUI_WriteString(5, Y_str, "Помилки читання FLASH!", Font_11x18, YELLOW, RED);	break;
  	default:GUI_WriteString(5, Y_str, "Невыдома помилка!", Font_11x18, MAGENTA, BLACK);	break;
  }
  Y_str = Y_str+18+5;
  
  if(i16){
    sprintf(buffTFT,"Check sum: 0x%08X",dataRAM.config.checkSum);
    GUI_WriteString(5, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
    Y_str = Y_str+18+5;
    sprintf(buffTFT,"Number of saves: %u",dataRAM.config.countSave);
    GUI_WriteString(5, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
    Y_str = Y_str+18+5;
    HAL_Delay(1000);
  }
  //---------------------------- линия 1-Wire -----------------------------------
  if(ds18b20_amount){
    for(uint8_t i=0;i<ds18b20_amount;i++) ds.pvT[i]=1999;
    ds18b20_Convert_T();
  }
  sprintf(buffTFT,"Датчикыв температури: %d шт.",ds18b20_amount);
  GUI_WriteString(5, Y_str, buffTFT, Font_11x18, CYAN, BLACK);
  Y_str = Y_str+18+5;
  
  sprintf(buffTFT,"WIDTH: %u; HEIGHT: %u",lcddev.width,lcddev.height);
  GUI_WriteString(5, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
  Y_str = Y_str+18+5;
  
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
  
  HAL_RTCEx_SetSecond_IT(&hrtc);          /* ------  таймер 1Гц.  период  1 сек.    ----*/
  HAL_TIM_Base_Start_IT(&htim1);          /* ------  таймер 100Гц.  период  10 мс.  ----*/
  
  HAL_Delay(3000);
  NEWBUTT = ON;
//  ds.pvT[0]=320; ds.pvT[1]=220; ds.pvT[2]=20; //???????????????????????????????????????????????????????????????
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    Y_str = 5; X_left = 5;
    //-------------------------- ТАЧСКРИН ---------------------------------------
    if(XPT2046_TouchPressed()&& checkTime>40){
      uint8_t butt_num;
      if(XPT2046_TouchGetCoordinates(&touch_x, &touch_y)){
        for (butt_num=0; butt_num<buttonAmount; butt_num++){
            if(contains(touch_x, touch_y, butt_num)) break; // проверка попадания новой координаты в область кнопки
        }
      }
      checkButtons(butt_num);                           // проверка нажатой кнопки
      checkTime = 0; CHECK = ON;
    }
    // ----------------- УВЛАЖНИТЕЛЬ -------------------------------------
    if(WORK && modeCell==3){          // только в режиме варка
      if(timer10ms){                  // шаг отсчета интервала таймера 10 милисек.
        timer10ms=0; 
        HUMIDI=humidifier(HUMIDI);    // проверим выход на увлажнитель
        invers = ~relayOut.value;
        HAL_I2C_Master_Transmit(&hi2c1,0x4E,&invers,1,1000);
      }
    }
    else HUMIDI = OFF;
    //-------------- Начало проверки каждую 1 сек. -----------------------
    if(CHECK){ CHECK = OFF; errors=0;  //if(++temp>10) {temp=0; ++pvspeed; pvspeed&=7; ds.pvT[1] = speedData[pvspeed][0]; sendToI2c(speedData[pvspeed][1]);}
      if(resetDispl) --resetDispl; else if(displ_num){displ_num = 0; NEWBUTT = 1;}  // возврат к главному дисплею
      temperature_check();
      //---------------------------------- Проверка работы вентилятора -------------------------------------
      if(VENTIL){
        if(HAL_GPIO_ReadPin(Input0_GPIO_Port, Input0_Pin) == GPIO_PIN_RESET) {SPEED=ON; tmrVent=0;} // если контакт замкнут
        else SPEED=OFF;
        if(tmrVent) --tmrVent;    // ожидаем замыкания контакта частотного преобразователя
        else if(SPEED) WORK=ON;
        else {errors |= 0x80; WORK=OFF; relayOut.value=OFF;}  // НЕ ПРАЦЮЭ ВЕНТИЛЯТОР
      }
      //------------------------------------------- В РАБОТЕ -----------------------------------------------
      if(WORK){
        TIMER=ON;
        //??????????????????????
//        int16_t pverr = abs(set[T0]*10 - ds.pvT[0]);
//        int8_t pv = 0;
//        if(pverr>50) pv = 5;
//        else if(abs(pverr)>20) pv = 2;
//        else pv = 1;
//        if(HEATER) {ds.pvT[0]+=pv; ds.pvT[1]+=pv; ds.pvT[2]+=pv;}  //???????????????????????????????????????????
//        else {ds.pvT[0]-=pv; ds.pvT[1]-=pv; ds.pvT[2]-=pv;}  //???????????????????????????????????????????
        //??????????????????????
        
        i16 = set[T0]*10 - ds.pvT[0];         // величина ошибки регулирования датчика 0
        if(abs(i16)<set[HIST]) PERFECT=ON;     // Выщли на заданную температуру
        u16 = set[HIST]*4;                    // HIST = 0.5 * 4 = 2.0 грд. Ц. цветовая индикация
        if(u16>=set[ALRM]) u16 = set[ALRM]/2; // тогда привяжем к аварии
        //--- кстанавливаем point_color в соответсвии с отклонением
        if(i16+set[ALRM]*10<0) {errors|=0x04; point_color = RED;} // ПЕРЕГРЕВ В КАМЕРЕ
        else if(i16>-u16 && i16<u16) point_color = GREEN; // норма
        else if(i16<-u16){                    // ВЫЩЕ нормы
          point_color = MAGENTA;
          if(PERFECT) errors|=0x10;           // ВІДХІЛЕННЯ ТЕМПЕРАТУРИ
        }
        else if(i16>u16){                     // НИЖЕ нормы
          point_color = CYAN;
          if(PERFECT) errors|=0x10;           // ВІДХІЛЕННЯ ТЕМПЕРАТУРИ
        }
        
        i16 = set[T1]*10 - ds.pvT[1];    // величина ошибки регулирования датчика 1
        if(i16+set[ALRM]*10<0) {errors|=0x08;} // ПЕРЕГРЕВ В ПРОДУКТЕ
        // ---------------------------------------- НАГРЕВАТЕЛЬ / ОХЛАДИТЕЛЬ -------------------------------------
        //------ работает как нагреватель
        if(ds.pvT[0]<1999 && ds.pvT[0]>1){
          i16 = Relay(set[T0]*10 - ds.pvT[0], set[HIST]);  // величина ошибки температуры воздуха
          u16 = 0; // номер канала по которому расчитана ошибка
        }
        else if(ds18b20_amount>1 && ds.pvT[1]<1999){// если датчик потерян то опираемся на температура среды
          i16 = Relay(set[T1]*10 - ds.pvT[1], set[HIST]);
          u16 = 1; // номер канала по которому расчитана ошибка
        }
        else {i16 = OFF; u16 = 255;}    // несуществующий канал
        
        if(u16<2){    // только если 0 или 1 канал
          pwTriac = UpdatePID(u16);     // ПИД нагреватель 
          if(pwTriac) TRIAC = ON;       // включить (SSR-25DA)
        }
        //------ работает как охладитель
        if(set[HEATER_DIR]){  
          i16 = Relay(ds.pvT[0] - set[T0]*10, set[HIST]);
          if(ds.pvT[0] > BEGINCOOL) i16 = OFF;      // температура выше которой ЗАПРЕЩЕНО включение охлаждения
        }
        switch (i16){
          case ON:  HEATER = ON;  break;
          case OFF: HEATER = OFF; break;
        }
        //-------------------------- Только для режима КОПЧЕНИЯ ---------------------------------
        if(modeCell==3){
          ELECTRO = ignition(ELECTRO);
          if(tmrCounter==-1){
            i16 = set[T2]*10 - ds.pvT[2];     // величина ошибки регулирования датчика 2 (Дым)
            if(i16-set[ALRM]*2>0){            // ( ниже 2 грд.Ц) ДЫМ НИЗКОЙ ТЕМПЕРАТУРЫ
              if(++checkSmoke>CHKSMOKE) {checkSmoke=CHKSMOKE; errors|=0x20;} 
            }
            u16 = Relay(i16, set[HIST]);  // величина ошибки температуры дыма
            switch (u16){
              case ON:  SMOKE = ON;  break;
              case OFF: SMOKE = OFF; break;
            }
          }
        }
        
        u16 = sTime.Hours*60 + sTime.Minutes;         // всего в минутах
        i16 = (set[TMR0] - u16)*60 - sTime.Seconds;   // осталось до выключения в секундах
        if(i16<30) ticBeep = 5;                      // включить сигнал
        
        if(ds18b20_amount==1 && set[TMR0]==0){      // если 1 датчик и продолжительность 0 то завершение по температуре камеры.          
          i16 = Relay(set[T0]*10 - ds.pvT[0], 0);   // температура камеры
          if(i16==OFF){
            portFlag.value = OFF; PURGING=ON; relayOut.value=OFF; ticBeep=200;
            //------- далее продувка ---------
            sTime.Hours=0; sTime.Minutes=0; sTime.Seconds=0;
            HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
          }
        }
        else if(INSIDE){                            // если завершение по температуре продукта.          
          i16 = Relay(set[T1]*10 - ds.pvT[1], 0);   // температура продукта
          if(i16==OFF){
            portFlag.value = OFF; PURGING=ON; relayOut.value=OFF; ticBeep=200;
            //------- далее продувка ---------
            sTime.Hours=0; sTime.Minutes=0; sTime.Seconds=0;
            HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
          }
        }
        else if(i16<=0){                    // если завершение программы по таймеру
          portFlag.value = OFF; PURGING=ON; relayOut.value=OFF; ticBeep=200;
          //------- далее продувка ---------
            sTime.Hours=0; sTime.Minutes=0; sTime.Seconds=0;
            HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        }
        
      } //------------------------------------ В РАБОТЕ -----------------------------------------------
      else if(PURGING){
        u16 = sTime.Minutes*60+sTime.Seconds;           // всего в секундах
        if(u16>=set[TMR1]) {PURGING=OFF; sendToI2c(0); NEWBUTT=ON; ticBeep=200;}
      }
      //------ Проверка на ручное управление ---------------------------------------
      for (i16=0;i16<6;i16++){
          if(relaySet[i16]==1) relayOut.value |= (1<<(i16)); // ручной On
          if(relaySet[i16]==0) relayOut.value &= ~(1<<(i16));// ручной Off
      }
      invers = ~relayOut.value;
      HAL_I2C_Master_Transmit(&hi2c1,0x4E,&invers,1,1000);
//      for (i16=0;i16<2;i16++){  // ручное управление аналоговыми выводами
//        if(analogSet[i16]>-1) analogOut[i16]=analogSet[i16];
//      }
      if(errors){
        ALARM = ON; 
        switch (errors){
          case 0x01: ticBeep = 80; break;// ПОМИЛКА ДАТЧИКА N1
          case 0x02: ticBeep = 80; break;// ПОМИЛКА ДАТЧИКА N2
          case 0x04: ticBeep =120; break;// ПЕРЕГРЫВ В КАМЕРI
          case 0x08: ticBeep =120; break;// ПЕРЕГРЫВ В ПРОДУКТI
          case 0x10: ticBeep = 20; break;// ВЫДХЫЛЕННЯ ТЕМПЕРАТУРИ В КАМЕРI
          case 0x20: ticBeep = 20; break;// ДИМ НИЗЬКОЪ ТЕМПЕРАТУРИ
          case 0x40: ticBeep = 60; break;//
          case 0x80: ticBeep = 60; break;// НЕ ПРАЦЮЭ ВЕНТИЛЯТОР
          default: ticBeep =200; break;
        }
      } else ALARM = OFF;  // есть ошибки

      display();
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 719;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, T_CS_Pin|TFT_DC_Pin|TFT_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, TFT_RST_Pin|Beep_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : Input0_Pin */
  GPIO_InitStruct.Pin = Input0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Input0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : T_IRQ_Pin */
  GPIO_InitStruct.Pin = T_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(T_IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : T_CS_Pin TFT_CS_Pin */
  GPIO_InitStruct.Pin = T_CS_Pin|TFT_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OneWR_Pin */
  GPIO_InitStruct.Pin = OneWR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(OneWR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TFT_RST_Pin */
  GPIO_InitStruct.Pin = TFT_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(TFT_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TFT_DC_Pin */
  GPIO_InitStruct.Pin = TFT_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TFT_DC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Beep_Pin */
  GPIO_InitStruct.Pin = Beep_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Beep_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
