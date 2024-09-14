#ifndef _NVRAM_H
#define _NVRAM_H

#include "main.h"
#include <stdint.h>

#define INDEX 12
struct Config {
    uint32_t notUsed[5];// эти 5 полей портятся процессором по неизвестной причине!!
    uint32_t magicNum;
    uint16_t mode;
    uint16_t modeSet0[INDEX];
    uint16_t modeSet1[INDEX];
    uint16_t modeSet2[INDEX];
    uint16_t modeSet3[INDEX];
    uint16_t koff[2];
    int8_t relaySet[8];   // relaySet[0..7]- ручное задание выхода;
    int8_t analogSet[2];  // analogSet[0;1]- ручное задание выхода;
    uint8_t familycode[MAX_SENSOR][8];
    uint16_t speedData[MAX_SPEED][2];
    uint32_t checkSum;
    uint32_t countSave;
};

union DataRam {
    struct Config config;
    uint32_t data32[256];
};

extern union DataRam dataRAM;

uint8_t initData(void);
uint32_t calcChecksum(void);
uint32_t writeData(void);

#endif /* _NVRAM_H */
