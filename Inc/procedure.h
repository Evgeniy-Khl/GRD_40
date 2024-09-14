#ifndef _PROCEDURE_H
#define _PROCEDURE_H

#include <main.h>

void startPrg(void);
uint8_t Relay(int16_t err, uint8_t hst);
uint8_t humidifier(uint8_t value);
uint8_t UpdatePID(uint8_t cn);
void permutation (char a, char b);
uint8_t sendToI2c(uint16_t val);
uint8_t ignition(uint8_t value);

#endif /* _PROCEDURE_H */
