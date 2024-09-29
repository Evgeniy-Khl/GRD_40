/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Input0_Pin GPIO_PIN_0
#define Input0_GPIO_Port GPIOA
#define T_IRQ_Pin GPIO_PIN_3
#define T_IRQ_GPIO_Port GPIOA
#define T_CS_Pin GPIO_PIN_4
#define T_CS_GPIO_Port GPIOA
#define OneWR_Pin GPIO_PIN_11
#define OneWR_GPIO_Port GPIOB
#define TFT_RST_Pin GPIO_PIN_12
#define TFT_RST_GPIO_Port GPIOB
#define TFT_DC_Pin GPIO_PIN_12
#define TFT_DC_GPIO_Port GPIOA
#define TFT_CS_Pin GPIO_PIN_15
#define TFT_CS_GPIO_Port GPIOA
#define Beep_Pin GPIO_PIN_6
#define Beep_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define DIAGONAL    28          // 24 -> для дисплеев 2,4"; 28 -> для дисплеев 2,8"; 32 -> для дисплеев 3,2"
#define TOUCHMODE   0           // 0 или 1
#define MAX_SENSOR  4
#define MAX_MODE    4
#define MAX_SET     8
#define MAX_OTHER   7
#define MAX_SPEED   8
#define ON          1
#define OFF         0
#define HEATER_DIR  10
#define HUMIDI_DIR  11
#define T0    0 // Уставка T1 грд. 
#define T1    1 // Уставка T2 грд. 
#define T2    2 // Уставка T3 грд. 
#define TMR0  3 // Длительность режима мин.
#define VENT  4 // Скорость вентилятора %
#define TMON  5 // Таймер ON сек.
#define TMOFF 6 // Таймер OFF сек.
#define TMR1  7 // Длительность продувки сек.
#define ALRM  8 // Авария грд.
#define HIST  9 // Гистерезис грд/10

#define MANUAL_CHECK

#ifdef MANUAL_CHECK
  #define CHKSMOKE  180 // (3 мин.) отжидание проверки температуры дыма в сек.
#else
  #define CHKSMOKE  1500 // (25 мин.) отжидание проверки температуры дыма в сек.
#endif
#define BEGINCOOL 400 // температура 40 грд. выше которой ЗАПРЕЩЕНО включение охлаждения
#define BEGINHUM  400 // запрет увлажнения при температуре ниже 40 грд.


/* ---структура с битовыми полями -----*/
struct byte {
    unsigned a0: 1;
    unsigned a1: 1;
    unsigned a2: 1;
    unsigned a3: 1;
    unsigned a4: 1;
    unsigned a5: 1;
    unsigned a6: 1;
    unsigned a7: 1;
};
 
union Byte {
    unsigned char value;
    struct byte bitfield;
};

struct Ds{
  int16_t pvT[MAX_SENSOR];
  uint8_t err[MAX_SENSOR];
};

extern struct Ds ds;

#define CHECK   portFlag.bitfield.a0  // Start of all checks
#define SPEED   portFlag.bitfield.a1  // Speed Ok.
#define WORK 	  portFlag.bitfield.a2  // At work flag
#define NEWBUTT portFlag.bitfield.a3  // New screen flag
#define VENTIL	portFlag.bitfield.a4  // Fan speed flag
#define PERFECT	portFlag.bitfield.a5  // Достигли желаемой температуры
#define INSIDE  portFlag.bitfield.a6  // End by product temperature
#define PURGING portFlag.bitfield.a7  // Продувка

#define TRIAC   relayOut.bitfield.a0  // SSR-25DA
#define HEATER  relayOut.bitfield.a1  // НАГРЕВАТЕЛЬ
#define TIMER 	relayOut.bitfield.a2  // ТАЙМЕР
#define HUMIDI	relayOut.bitfield.a3  // УВЛАЖНИТЕЛЬ
#define ELECTRO	relayOut.bitfield.a4  // Электроподжиг
#define SMOKE   relayOut.bitfield.a5  // Клапан дыма
#define OTHER1	relayOut.bitfield.a6  // прочее 1
#define ALARM   relayOut.bitfield.a7  // Тревога

extern union Byte portFlag;
extern union Byte relayOut;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
