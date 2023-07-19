#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "iot_i2c.h"
#include "iic_oled.h"
#include "oled_font.h"

#define OLED_ADDR  0x78
#define WIFI_IOT_I2C_IDX_1 1

uint8_t init_cmd_array[] = {0xAE, 0x00, 0x10, 0x40, 0xB0, 0x81, 0xFF, 0xA1, 0xA6, 0xA8, 0x3F, 0xC8, 0xD3,
                            0x00, 0xD5, 0x80, 0xD8, 0x05, 0xD9, 0xF1, 0xDA, 0x12, 0xDB, 0x30, 0x8D, 0x14, 0xAF};

typedef struct {
    /* Pointer to the buffer storing data to send */
    unsigned char *sendBuf;
    /* Length of data to send */
    unsigned int  sendLen;
    /* Pointer to the buffer for storing data to receive */
    unsigned char *receiveBuf;
    /* Length of data received */
    unsigned int  receiveLen;
} IotI2cData;

static void oled_write_data(uint8_t oled_dat)
{
    uint32_t status = 0;
    IotI2cData oled_cmd_data = {0};
    uint8_t send_data[2] = {0x40, oled_dat};

    oled_cmd_data.sendLen = 2;
    oled_cmd_data.sendBuf = send_data;

    status = IoTI2cWrite(WIFI_IOT_I2C_IDX_1, OLED_ADDR, oled_cmd_data.sendBuf,oled_cmd_data.sendLen);
    if(status != 0)
    {
        printf("======ERROR OLED IIC 0x%x = 0x%x!", oled_dat, status);
    }
    else
    {
        printf("success:0x%x\n", oled_dat);
    }
}

static void oled_write_cmd(uint8_t oled_command)
{
    uint32_t status = 0;
    IotI2cData oled_cmd_data = {0};
    uint8_t send_data[2] = {0x00, oled_command};

    printf("oled cmd\n");

    oled_cmd_data.sendLen = 2;
    oled_cmd_data.sendBuf = send_data;

    status = IoTI2cWrite(WIFI_IOT_I2C_IDX_1, OLED_ADDR, oled_cmd_data.sendBuf,oled_cmd_data.sendLen);
    if(status != 0)
    {
        printf("======ERROR OLED IIC 0x%x = 0x%x!", oled_command, status);
    }
    else
    {
        printf("success:0x%x\n", oled_command);
    }
}

//坐标设置
void OLED_Set_Pos(unsigned char x, unsigned char y) 
{ 	
    oled_write_cmd(0xb0+y);
	oled_write_cmd(((x&0xf0)>>4)|0x10);
	oled_write_cmd((x&0x0f)); 
}

//开启OLED显示    
void OLED_Display_On(void)
{
	oled_write_cmd(0X8D);  //SET DCDC命令
	oled_write_cmd(0X14);  //DCDC ON
	oled_write_cmd(0XAF);  //DISPLAY ON
}

//关闭OLED显示     
void OLED_Display_Off(void)
{
	oled_write_cmd(0X8D);  //SET DCDC命令
	oled_write_cmd(0X10);  //DCDC OFF
	oled_write_cmd(0XAE);  //DISPLAY OFF
}

//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!	  
void OLED_Clear(void)  
{  
	uint8_t i,n;		    
	for(i=0;i<8;i++)  
	{  
		oled_write_cmd(0xb0+i);    //设置页地址（0~7）
		oled_write_cmd(0x00);      //设置显示位置—列低地址
		oled_write_cmd(0x10);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)
            oled_write_data(0); 
	} //更新显示
}
void OLED_On(void)  
{  
	uint8_t i,n;		    
	for(i=0;i<8;i++)  
	{  
		oled_write_cmd(0xb0+i);    //设置页地址（0~7）
		oled_write_cmd(0x00);      //设置显示位置—列低地址
		oled_write_cmd(0x10);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)
            oled_write_data(1); 
	} //更新显示
}
//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示				 
//size:选择字体 16/12 
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size)
{      	
	unsigned char c=0,i=0;	
    c=chr-' ';//得到偏移后的值			
    if(x>Max_Column-1){x=0;y=y+2;}
    if(Char_Size ==16)
    {
        OLED_Set_Pos(x,y);	
        for(i=0;i<8;i++)
        oled_write_data(F8X16[c*16+i]);
        OLED_Set_Pos(x,y+1);
        for(i=0;i<8;i++)
        oled_write_data(F8X16[c*16+i+8]);
    }
    else {	
        OLED_Set_Pos(x,y);
        for(i=0;i<6;i++)
        oled_write_data(F6x8[c*6+i]);
    }
}

//显示一个字符串
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t Char_Size)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{		
        OLED_ShowChar(x,y,chr[j],Char_Size);	
        x += 8;
		if(x>120)
        {
            x=0;y+=2;
        }
		j++;
	}
}

void OLED_ShowFloat(uint8_t x,uint8_t y,float num,uint8_t Char_Size)
{
    int data = num*10;
    int num1=data/1000,num2=data/100%10,num3=data/10%10,num4=data%10;
    if(num1){
        OLED_ShowChar(x,y,(char)num1+48,Char_Size);	
        x += 8;
    }
    OLED_ShowChar(x,y,(char)num2+48,Char_Size);	
    x += 8;
    OLED_ShowChar(x,y,(char)num3+48,Char_Size);	
    x += 8;
    OLED_ShowChar(x,y,'.',Char_Size);	
    x += 8;
    OLED_ShowChar(x,y,(char)num4+48,Char_Size);	
    x += 8;
}

void oled_init(void)
{
    uint8_t i = 0;

    for(i = 0;i < 27; i++)
    {
        oled_write_cmd(init_cmd_array[i]);
    }
    OLED_Clear();
    OLED_ShowString(24, 0, (uint8_t *)"Hi,newfarmer!", 16);
    OLED_ShowString(40, 2, (uint8_t *)"BearPi", 16);
    sleep(4);
    OLED_Clear();
}


