	
#ifndef __LCD_H
#define __LCD_H		
#include "sys.h"	 
#include "stdlib.h"


typedef struct  
{										    
	u16 width;			
	u16 height;			
	u16 id;				//LCD ID
	u8  dir;				
	u16	 wramcmd;	
	u16  setxcmd;		
	u16  setycmd;		 
}_lcd_dev; 	


extern _lcd_dev lcddev;	
#define USE_HORIZONTAL  	 0

#define LCD_W 240
#define LCD_H 320

extern uint16_t SPI_LCD_RAM[320*240];
	   
extern u16  POINT_COLOR;    
extern u16  BACK_COLOR; 

#define LED  13        
#define CS   15       
#define RS   14       
#define RST  12       

#define	LCD_LED PBout(LED)  		 
#define LCD_CS  PBout(CS)
#define LCD_RS  PBout(RS)
#define LCD_RST PBout(RST)

#define	LCD_CS_SET  LCD_CS=1   //GPIO_TYPE->BSRRL=1<<LCD_CS    //	  PB11
#define	LCD_RS_SET	LCD_RS=1   //GPIO_TYPE->BSRRL=1<<LCD_RS    //   PB10	  
#define	LCD_RST_SET	LCD_RST=1  //GPIO_TYPE->BSRRL=1<<LCD_RST  //		PB12

 							    
#define	LCD_CS_CLR  LCD_CS=0   //GPIO_TYPE->BSRRH=1<<LCD_CS     // 	PB11
#define	LCD_RS_CLR	LCD_RS=0   //GPIO_TYPE->BSRRH=1<<LCD_RS     //  PB10	 
#define	LCD_RST_CLR	LCD_RST=0  //GPIO_TYPE->BSRRH=1<<LCD_RST   //	PB12


#define WHITE       0xFFFF
#define BLACK      	0x0000	  
#define BLUE       	0x001F  
#define BRED        0XF81F
#define GRED 			 	0XFFE0
#define GBLUE			 	0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define BROWN 			0XBC40 
#define BRRED 			0XFC07 
#define GRAY  			0X8430 



#define SKYBLUE          0X867D
#define DARKBLUE      	 0X01CF	
#define LIGHTBLUE      	 0X7D7C	  
#define GRAYBLUE       	 0X5458 
 
#define LIGHTGREEN     	0X841F 
#define LIGHTGRAY       0XEF5B 
#define LGRAY 			 		0XC618 

#define LGRAYBLUE      	0XA651 
#define LBBLUE          0X2B12 
	    															  
void LCD_Init(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(u16 Color);	 
void LCD_SetCursor(u16 Xpos, u16 Ypos);
void LCD_DrawPoint(u16 x,u16 y);
u16  LCD_ReadPoint(u16 x,u16 y);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);		   
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd);

u16 LCD_RD_DATA(void);									    
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue);
void LCD_WR_DATA(u8 data);
u16 LCD_ReadReg(u8 LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(u16 RGB_Code);
u16 LCD_ReadRAM(void);		   
u16 LCD_BGR2RGB(u16 c);
void LCD_SetParam(void);
void Lcd_WriteData_16Bit(u16 Data);
void LCD_direction(u8 direction );

void LCD_RESET(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Backlight(uint8_t pwm);




void Lcd_WriteData_16Bitgg(u16 Data);
//#if LCD_USE8BIT_MODEL==1
//	#define LCD_WR_DATA(data){\
//	LCD_RS_SET;\
//	LCD_CS_CLR;\
//	DATAOUT(data);\
//	LCD_WR_CLR;\
//	LCD_WR_SET;\
//	DATAOUT(data<<8);\
//	LCD_WR_CLR;\
//	LCD_WR_SET;\
//	LCD_CS_SET;\
//	}
//	#else
//	#define LCD_WR_DATA(data){\
//	LCD_RS_SET;\
//	LCD_CS_CLR;\
//	DATAOUT(data);\
//	LCD_WR_CLR;\
//	LCD_WR_SET;\
//	LCD_CS_SET;\
//	} 	
//#endif
	
#endif  
