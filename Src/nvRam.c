#include "main.h"
#include "nvRam.h"

extern uint8_t familycode[MAX_SENSOR][8], modeCell;
extern uint16_t speedData[MAX_SPEED][2];
extern uint16_t set[INDEX];
extern CRC_HandleTypeDef hcrc;

union DataRam dataRAM;

uint32_t calcChecksum(void){
    __HAL_RCC_CRC_CLK_ENABLE(); // ��������� clock ��� CRC ������
//    CRC->CR = CRC_CR_RESET;     // ���������� CRC ������
    HAL_CRC_Init(&hcrc);
    // ��������� ����� ��� ���������� CRC
    uint32_t* configPtr = (uint32_t*)&dataRAM.config.magicNum;
    // ��������� CRC ��� ���� ����� ��������� Config, ����� ����� notUsed[5], ���� checkSum � ���� countSave
    uint8_t sizeBuff = (sizeof(struct Config) - sizeof(uint32_t)*(UNUSED_FIELDS+1) - sizeof(uint16_t)) / sizeof(uint32_t);
    uint32_t CRCVal = HAL_CRC_Calculate(&hcrc, configPtr, sizeBuff);
    
    __HAL_RCC_CRC_CLK_DISABLE();// ��������� clock ��� CRC ������
    return CRCVal;// ���������� �������� CRC
}

void setData(uint8_t m){
  uint8_t i, x;
  switch (m){
  	case 0: for(i=0;i<INDEX;i++){set[i] = dataRAM.config.modeSet0[i];} break;
  	case 1: for(i=0;i<INDEX;i++){set[i] = dataRAM.config.modeSet1[i];} break;
    case 2: for(i=0;i<INDEX;i++){set[i] = dataRAM.config.modeSet2[i];} break;
  	case 3: for(i=0;i<INDEX;i++){set[i] = dataRAM.config.modeSet3[i];} break;
  	default: modeCell = dataRAM.config.mode = 0; for(i=0;i<INDEX;i++){set[i] = dataRAM.config.modeSet0[i];}	break;
  }
  for(i=0;i<MAX_SENSOR;i++){
    for(x=0;x<8;x++){
      familycode[i][x] = dataRAM.config.familycode[i][x];
    }
  }
  for(i=0;i<MAX_SPEED;i++){
    for(x=0;x<2;x++){
      speedData[i][x] = dataRAM.config.speedData[i][x];
    }
  }
}

uint8_t initData(void){
  // ������ ������ �� FLASH ������
  uint8_t i, x, err=0;
  uint32_t l_Address = FLASH_CONFIG_START_ADDR;
  uint32_t l_End = FLASH_CONFIG_START_ADDR + sizeof(struct Config);
  uint32_t l_Index = 0x00;
  
  while (l_Address < l_End){    // FLASH_CONFIG_END_ADDR
    // ((__IO uint32_t *)l_Address)
    // ���������� ������ l_Address � ��������� �� 32-������ (4-�������) ����������� ����� ����� � ��������� __IO, 
    // ������� ����������, ��� ��� "����/�����" (Input/Output) ����������.
    // *(...) - ������������� ����� ���������, ����� �������� �������� �� ����� ������.
    dataRAM.data32[l_Index] = *(__IO uint32_t *)l_Address;
    l_Index = l_Index + 1;
    l_Address = l_Address + 4;
  }
//  for(uint8_t i=0;i<5;i++){dataRAM.config.notUsed[i]=0;}
  // ���� CRC �� ��������� ...
  l_Index = calcChecksum();
  if(l_Index != dataRAM.config.checkSum) err |= 2;
  
  // ���� ����� ������ ���� �� ������� �� ��� ������ ������
  if(dataRAM.config.magicNum != MAGIC_KEY_DEFINE || err){
  /*
  //----------------    T1,T2,T3,Tmr,cTmr,ON,OFF,al,hysteresis
  uint16_t modeSet0[9]={70,60,50,180,160,  60,180,10,5}  // ���, ���, ���, ���, ���., ���., ���. ���, ���/10,
  uint16_t modeSet1[9]={71,61,51,181,161,  60,180,10,5}  // ���, ���, ���, ���, ���., ���., ���. ���, ���/10,
  uint16_t modeSet2[9]={72,62,52,182,162,  60,180,10,5}  // ���, ���, ���, ���, ���., ���., ���. ���, ���/10,
  uint16_t modeSet3[9]={73,63,53,183,163,  60,180,10,5}  // ���, ���, ���, ���, ���., ���., ���. ���, ���/10,
  //------------------auto
  int8_t relaySet[8]={-1,-1,-1,-1,-1,-1,-1,-1};
  //------------------auto
  int8_t analogSet[2]={-1,-1};
  */  
    err |= 1;
    for(uint8_t i=0;i<UNUSED_FIELDS;i++){dataRAM.config.notUsed[i]=0;}
    
    dataRAM.config.magicNum=MAGIC_KEY_DEFINE;
    dataRAM.config.mode = 0;
// ----------- "�������"
    dataRAM.config.modeSet0[0]=70;  // ������� T1 ���. 
    dataRAM.config.modeSet0[1]=60;  // ������� T2 ���.
    dataRAM.config.modeSet0[2]=0;   // ������� T3 ���.
    dataRAM.config.modeSet0[3]=180; // ������������ ������ ���.
    dataRAM.config.modeSet0[4]=160; // ������������ �������� ���.
    dataRAM.config.modeSet0[5]=0;   // �������� ����������� %
    dataRAM.config.modeSet0[6]=0;   // ������ ON �� ������������
    dataRAM.config.modeSet0[7]=0;   // ������ OFF �� ������������
    dataRAM.config.modeSet0[8]=10;  // ������ ���.
    dataRAM.config.modeSet0[9]=5;   // ���������� ���/10
    dataRAM.config.modeSet0[10]=0;  // ������ 0-������, 1-�������� ����������
    dataRAM.config.modeSet0[11]=0;  // ���������� 0-������, 1-�������� ����������
// ----------- "�������"
    dataRAM.config.modeSet1[0]=71;  // ���
    dataRAM.config.modeSet1[1]=61;  // ���
    dataRAM.config.modeSet1[2]=0;   // ���
    dataRAM.config.modeSet1[3]=170; // ���
    dataRAM.config.modeSet1[4]=150; // ���
    dataRAM.config.modeSet1[5]=1;   // �������� ����������� %
    dataRAM.config.modeSet1[6]=0;   // ������ ON �� ������������
    dataRAM.config.modeSet1[7]=0;   // ������ OFF �� ������������
    dataRAM.config.modeSet1[8]=10;  // ������ ���.
    dataRAM.config.modeSet1[9]=5;   // ���/10
    dataRAM.config.modeSet1[10]=0;  // ������ 0-������, 1-�������� ����������
    dataRAM.config.modeSet1[11]=0;  // ���������� 0-������, 1-�������� ����������
// ----------- "�������"
    dataRAM.config.modeSet2[0]=72;  // ���
    dataRAM.config.modeSet2[1]=62;  // ���
    dataRAM.config.modeSet2[2]=52;  // ���
    dataRAM.config.modeSet2[3]=160; // ���
    dataRAM.config.modeSet2[4]=140; // ���
    dataRAM.config.modeSet2[5]=2;   // �������� ����������� %
    dataRAM.config.modeSet2[6]=6;   // ����������� ON 0.6 ���.
    dataRAM.config.modeSet2[7]=18;  // ����������� OFF 1.8���.
    dataRAM.config.modeSet2[8]=10;  // ������ ���.
    dataRAM.config.modeSet2[9]=5;   // ���/10
    dataRAM.config.modeSet2[10]=0;  // ������ 0-������, 1-�������� ����������
    dataRAM.config.modeSet2[11]=0;  // ���������� 0-������, 1-�������� ����������
// ----------- "��������"
    dataRAM.config.modeSet3[0]=73;  // ���
    dataRAM.config.modeSet3[1]=63;  // ���
    dataRAM.config.modeSet3[2]=53;  // ���
    dataRAM.config.modeSet3[3]=150; // ���
    dataRAM.config.modeSet3[4]=130; // ���
    dataRAM.config.modeSet3[5]=3;   // �������� ����������� %
    dataRAM.config.modeSet3[6]=180; // ������ ON 180���.
    dataRAM.config.modeSet3[7]=2;   // ����� ����� �������� OFF 2 ���.
    dataRAM.config.modeSet3[8]=10;  // ������ ���.
    dataRAM.config.modeSet3[9]=5;   // ���/10
    dataRAM.config.modeSet3[10]=0;  // ������ 0-������, 1-�������� ����������
    dataRAM.config.modeSet3[11]=0;  // ���������� 0-������, 1-�������� ����������
    
    dataRAM.config.koff[0]=10;      // ����������������
    dataRAM.config.koff[1]=500;     // ������������
    
    for(i=0;i<8;i++){dataRAM.config.relaySet[i]=-1;}  // �������������� ����������
    for(i=0;i<2;i++){dataRAM.config.analogSet[i]=-1;} // �������������� ����������
    for(i=0;i<MAX_SENSOR;i++){
      for(x=0;x<8;x++){
        dataRAM.config.familycode[i][x]=0;
      }
    }
    dataRAM.config.speedData[0][0]=1000;  //0x3E8
    dataRAM.config.speedData[0][1]=0x2E0; //0x2F4
    dataRAM.config.speedData[1][0]=1200;  //0x4B0
    dataRAM.config.speedData[1][1]=0x492; //0x4A6
    dataRAM.config.speedData[2][0]=1400;  //0x578
    dataRAM.config.speedData[2][1]=0x63C; //0x655
    dataRAM.config.speedData[3][0]=1600;  //0x640
    dataRAM.config.speedData[3][1]=0x7E6; //0x804
    dataRAM.config.speedData[4][0]=1800;  //0x708
    dataRAM.config.speedData[4][1]=0x998; //0x9B6
    dataRAM.config.speedData[5][0]=2000;  //0x7D0
    dataRAM.config.speedData[5][1]=0xB3D; //0xB65
    dataRAM.config.speedData[6][0]=2200;  //0x898
    dataRAM.config.speedData[6][1]=0xCEC; //0xD14
    dataRAM.config.speedData[7][0]=2400;  //0x960
    dataRAM.config.speedData[7][1]=0xFFF; //0xFFF
    
    dataRAM.config.checkSum = calcChecksum();
    dataRAM.config.countSave = 0;
  }
  modeCell = dataRAM.config.mode;
  setData(modeCell);
  return err;
}

uint32_t writeData(void){
  static FLASH_EraseInitTypeDef EraseInitStruct;
  // ��������� ������ ��� ������� ������
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = FLASH_CONFIG_START_ADDR;
  EraseInitStruct.NbPages     = 0x01;

  uint32_t l_Address = FLASH_CONFIG_START_ADDR;
  uint32_t l_End = FLASH_CONFIG_START_ADDR + sizeof(struct Config);
  uint32_t l_Error = 0x00;
  uint32_t l_Index = 0x00;
  // ��������� �����
  dataRAM.config.mode = modeCell;
  switch (modeCell){
  	case 0: for(uint8_t i=0;i<INDEX;i++){dataRAM.config.modeSet0[i] = set[i];} break;
  	case 1: for(uint8_t i=0;i<INDEX;i++){dataRAM.config.modeSet1[i] = set[i];} break;
    case 2: for(uint8_t i=0;i<INDEX;i++){dataRAM.config.modeSet2[i] = set[i];} break;
  	case 3: for(uint8_t i=0;i<INDEX;i++){dataRAM.config.modeSet3[i] = set[i];} break;
  	default: dataRAM.config.mode=modeCell=0;  for(uint8_t i=0;i<INDEX;i++){dataRAM.config.modeSet0[i] = set[i];}	break;
  }
  dataRAM.config.checkSum = calcChecksum();
  dataRAM.config.countSave = dataRAM.config.countSave + 1;
  // ��������� � ������ � ������
  HAL_FLASH_Unlock();
  // ������� �������� ������
  HAL_FLASHEx_Erase(&EraseInitStruct, &l_Error);
  // ����� ������ � ������
  while (l_Address < l_End){   //  FLASH_CONFIG_END_ADDR
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, l_Address, dataRAM.data32[l_Index]) == HAL_OK){
      l_Index = l_Index + 1;
      l_Address = l_Address + 4;
    }
  }
  // ��������� ������ �� ������
  HAL_FLASH_Lock();
  
  // ��������� ����������� ������
  l_Address = FLASH_CONFIG_START_ADDR;
  l_Error = 0x00;
  l_Index = 0x00; // 5 ����� �������� ����������� �� ����������� �������!!
  while (l_Address < l_End){
    if(dataRAM.data32[l_Index] != *(__IO uint32_t *)l_Address){
      if(l_Index>4) l_Error++;
    }
    l_Index = l_Index + 1;
    l_Address = l_Address + 4;
  }
  return l_Error;
}
/*
void checkData(void){
  static FLASH_EraseInitTypeDef EraseInitStruct;
  // ��������� ������ ��� ������� ������
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = FLASH_CONFIG_START_ADDR;
  EraseInitStruct.NbPages     = 0x01;
  
  // ��������� ����������� ������
  uint32_t l_Address = FLASH_CONFIG_START_ADDR;
  uint32_t l_Error = 0x00;
  uint32_t l_Index = 0x00;
  while (l_Address < FLASH_CONFIG_END_ADDR){
    if(dataRAM.data32[l_Index] != *(__IO uint32_t *)l_Address){
      l_Error++;
    }
    l_Index = l_Index + 1;
    l_Address = l_Address + 4;
  }
  if(l_Error){
    // ��������� � ������ � ������
    HAL_FLASH_Unlock();
    // ������� �������� ������
    HAL_FLASHEx_Erase(&EraseInitStruct, &l_Error);
    // ����� ������ � ������
    l_Address = FLASH_CONFIG_START_ADDR;
    l_Error = 0x00;
    l_Index = 0x00;
    dataRAM.config.countSave = dataRAM.config.countSave + 1;
    while (l_Address < FLASH_CONFIG_END_ADDR){     
      if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, l_Address, dataRAM.data32[l_Index]) == HAL_OK){
        l_Index = l_Index + 1;
        l_Address = l_Address + 4;
      }
    }
    // ��������� ������ �� ������
    HAL_FLASH_Lock();
  }
}*/
