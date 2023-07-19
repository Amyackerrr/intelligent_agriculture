#ifndef __OLED_H
#define __OLED_H			  	 

#include <stdint.h>

//OLED模式设置
//0:4线串行模式
//1:并行8080模式

#define SIZE 16
#define XLevelL		0x02
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	    						  

//OLED控制用函数
void oled_init(void);
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t Char_Size);
void OLED_ShowFloat(uint8_t x,uint8_t y,float num,uint8_t Char_Size);
void OLED_Display_On(void);  
void OLED_Display_Off(void);
void OLED_Clear(void);
void OLED_On(void);

#endif  
