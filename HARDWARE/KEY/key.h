
//    LCD            STM32
//    VCC            DC5V/3.3V      
//    GND            GND           
//    SDI(MOSI)      PB5         
//    SDO(MISO)      PB4          
//    LED            PB13         
//    SCK            PB3          
//    DC/RS          PB14         
//    RST            PB12         
//    CS             PB15    

//    T_IRQ           PB1          
//    T_DO            PB2          
//    T_DIN           PC4         
//    T_CS            PC5          
//    T_CLK           PB0       <  

#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"
	 
	 

#define KEY0 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4) //PE4
#define KEY1 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)	//PE3 
#define KEY2 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2) //PE2
#define WK_UP 	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)	//PA0


/*
#define KEY0 		PEin(4)   //PE4
#define KEY1 		PEin(3)		//PE3 
#define KEY2 		PEin(2)		//P32
#define WK_UP 	PAin(0)		//PA0
*/


#define KEY0_PRES 	1
#define KEY1_PRES	  2
#define KEY2_PRES	  3
#define WKUP_PRES   4

void KEY_Init(void);	
u8 KEY_Scan(u8);  		
#endif
