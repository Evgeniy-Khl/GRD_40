#ifndef __RTC_H__
#define __RTC_H__

#include "main.h"
#include <stdio.h>
#include <stdlib.h>

void setDataAndTime(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday, 
             uint8_t hour, uint8_t min,  uint8_t sec, uint32_t format);
void writeDateToBackup(uint32_t bkp_reg);
void readBackupToDate(uint32_t bkp_reg);
void readSetToBackup(uint32_t bkp_reg);
void writeSetToBackup(uint32_t bkp_reg);
uint32_t colodarToCounter (void);
void Fix_Date(RTC_DateTypeDef* date);

extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

#endif // __RTC_H__
