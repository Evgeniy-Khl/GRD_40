#ifndef _NVRAM_H
#define _NVRAM_H

#include "main.h"
#include <stdint.h>

#define MAGIC_KEY_DEFINE 0x19630301
#define FLASH_CONFIG_START_ADDR ((uint32_t) 0x0800FC00) // (63) ����� ������ � 127 �������� �� ������ 0x0801FC00
// 0x0800C800(50); 0x0800F000(60); 0x0800FC00(63); ����������[0x08010000](64); [0x08011800](70)
#define UNUSED_FIELDS 10
#define INDEX 12
struct Config {
    uint32_t notUsed[UNUSED_FIELDS];// ��� UNUSED_FIELDS ����� �������� ����������� �� ����������� �������!!
    uint32_t magicNum;
    uint16_t mode;
    uint16_t modeSet0[INDEX];
    uint16_t modeSet1[INDEX];
    uint16_t modeSet2[INDEX];
    uint16_t modeSet3[INDEX];
    uint16_t koff[2];
    int8_t relaySet[8];   // relaySet[0..7]- ������ ������� ������;
    int8_t analogSet[2];  // analogSet[0;1]- ������ ������� ������;
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
