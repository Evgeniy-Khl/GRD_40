#ifndef __MYLCD_H
#define __MYLCD_H

#include "main.h"


typedef struct  
{										    
	uint16_t width;     //LCD ������
	uint16_t height;    //LCD �������
	uint16_t id;        //LCD ID
	uint8_t  dir;			  //���������� �������������� ��� ������� �������: 0, ������� ����� 1, ��������� �����;	
	uint16_t wramcmd;		//����� �������
	uint16_t setxcmd;		//������� ��������� ���������� x
	uint16_t setycmd;		//������� ��������� ���������� Y	 
}_lcd_dev; 	

//��������� ��-�������
extern _lcd_dev lcddev;	//���������� ������� ����������� ��-�������

#define USE_VERTICAL0   0  //���������� ����������� �������� ��-������ �� ������� �������: �������� 0�0 ��������, 
#define USE_HORIZONT0   1  // �������� 1�90 ��������,
#define USE_VERTICAL1   2  // �������� 2�180 ��������,
#define USE_HORIZONT1   3  // �������� 3�270 ��������.

//������ ��-�������
#define LCD_W 320
#define LCD_H 480

extern SPI_HandleTypeDef hspi2;
#define TFT_SPI_PORT hspi2

////���������� GPIO (������� �������)
//#define	LCD_CS_SET  HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET)    //����� ����
//#define	LCD_DC_SET	HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET)    //������/�������
//#define	LCD_RST_SET	HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET)  //�������������

////����� GPIO (������ �������)							    
//#define	LCD_CS_CLR  HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET)  //����� ����
//#define	LCD_DC_CLR	HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET)  //������/������� 
//#define	LCD_RST_CLR	HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET)//�������������

//���� �����
#define WHITE       0xFFFF
#define BLACK      	0x0000	  
#define BLUE       	0x001F  
//#define BRED        0XF81F
//#define GRED 			 	0XFFE0
#define GBLUE			 	0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define BROWN 			0xBC40 //����������
#define BRRED 			0xFC07 //���������-�������
#define GRAY  			0x8430 //�����
//���� ������������ ����������
#define DARKBLUE    0x01CF	//�����-�����
#define LIGHTBLUE   0x7D7C	//������-�������  
#define GRAYBLUE    0x5458 //����-�����
//������������� ��� �����-��� ����� ������. 
#define LIGHTGREEN  0x841F //������-�������
#define LIGHTGRAY   0xEF5B //������-�����(PANNEL)
#define LGRAY       0xC618 //������-�����(PANNEL),���� ���� �����
#define LGRAYBLUE   0xA651 //������-����-����� (���� �������� ����)
#define LBBLUE      0x2B12 //������-���������-����� (�������� �������� ���� ������)

void LCD_Select(void);
void LCD_Unselect(void);
void LCD_Reset(void);
static void LCD_WriteCommand(uint8_t cmd);
void LCD_WriteData(uint8_t* buff, uint16_t buff_size);
void LCD_Init(uint8_t dir);
void LCD_Direction(uint8_t direction);
void LCD_Clear(uint16_t Color);
void LCD_SetWindow(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd);


#endif // __MYLCD_H
