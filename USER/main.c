#include "delay.h"
#include "sys.h"
#include "lcd.h"
#include "touch.h"
#include "gui.h"
#include "key.h" 
#include "led.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "pic.h"

//========================variable==========================//
u16 ColorTab[5]={RED,GREEN,BLUE,YELLOW,BRED};

void Touch_Test(void);
void DrawTestPage(u8 *str);
void Rotate_Test(void);
void Pic_test(void);

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
RTC_TimeTypeDef RTC_TimeStructure;
RTC_DateTypeDef RTC_DateStructure;

RTC_InitTypeDef RTC_InitStructure;
NVIC_InitTypeDef NVIC_InitStructur;
EXTI_InitTypeDef EXTI_InitStructure;

void Test_FillRec(void);
void DrawTestPage(u8 *str);
void Test_Color(void);

uint8_t RTC_HandlerFlag;



static int8_t sec, min, hour;
static int8_t day, month,weekday;
static int16_t year;
uint8_t TempStr[25];
uint8_t   strMonth[][12] = {"Ocak", "Subat", "Mart", "Nisan","Mayis","Haziran", "Temmuz", "Agustos","Eylul" ,"Ekim" ,"Kasim", "Aralik" };
uint8_t strWeekday[][12] = {"Pazartesi", "Sali", "Carsamba", "Persembe", "Cuma","Cumartesi", "Pazar"};

uint8_t AlarmWeekday[][12] = {"Pazartesi", "Sali", "Carsamba", "Persembe", "Cuma","Cumartesi", "Pazar"};

uint32_t AsynchPrediv = 0, SynchPrediv = 0;

void RTC_kur(void)
{
/* Enable the PWR clock */
RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
/* Allow access to RTC */
PWR_BackupAccessCmd(ENABLE);
// #if defined (RTC_CLOCK_SOURCE_LSI) /* LSI used as RTC source clock*/
/* The RTC Clock may varies due to LSI frequency dispersion */
/* Enable the LSI OSC */
RCC_LSICmd(ENABLE);
/* Wait till LSI is ready */
while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
{
}
/* Select the RTC Clock Source */
RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
/* Enable the RTC Clock */
RCC_RTCCLKCmd(ENABLE);
/* Wait for RTC APB registers synchronisation */
RTC_WaitForSynchro();
/* Configure the RTC data register and RTC prescaler */
RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
RTC_InitStructure.RTC_SynchPrediv = 0xFF;
RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
RTC_Init(&RTC_InitStructure);
/* Set the date: */
RTC_DateStructure.RTC_Year  = 21; //Year Set
RTC_DateStructure.RTC_Month = RTC_Month_October ; //Month date
RTC_DateStructure.RTC_Date  = 28; // Day count set
RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Wednesday; //Day set
RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
/* Set the time to */
//RTC_TimeStructure.RTC_H12 = RTC_H12_PM;
RTC_TimeStructure.RTC_Hours   = 17;
RTC_TimeStructure.RTC_Minutes = 18;
RTC_TimeStructure.RTC_Seconds = 30;
RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
/* Indicator for the RTC configuration */
RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
}

int timeFlag;

void TIM2_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  
    timeFlag++;
		
  }
}


void INT_TIM_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  /* Enable the TIM2 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 1000 - 1;  // 1 MHz down to 1 KHz (1 ms)
  TIM_TimeBaseStructure.TIM_Prescaler = 39999; 
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  /* TIM IT enable */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  /* TIM2 enable counter */
  TIM_Cmd(TIM2, ENABLE);
}


int main(void)
{	
	delay_init(168);    
	LCD_Init();	
  TP_Init();
	KEY_Init();
	LED_Init();
	RTC_kur();

	LCD_Fill(0,0,lcddev.width,lcddev.height,BLACK);
	
	LCD_Fill(0,100,lcddev.width,150,BLUE);
	Show_Str2412(10,110,WHITE,BLUE,"Saat ",1);
	Gui_Drawbmp40(lcddev.width-50,105,time);
	
	LCD_Fill(0,155,lcddev.width,205,BLUE);
	Show_Str2412(10,165,WHITE,BLUE,"Alarm ",1);
	Gui_Drawbmp40(lcddev.width-50,160,alarm);
	
	LCD_Fill(0,210,lcddev.width,260,BLUE);
	Show_Str2412(10,220,WHITE,BLUE,"Tarih ",1);
  Gui_Drawbmp40(lcddev.width-50,215,takvim);
	
	LCD_Fill(0,265,lcddev.width,315,BLUE);
	Show_Str2412(10,275,WHITE,BLUE,"Ses ",1);
	Gui_Drawbmp40(lcddev.width-50,270,ses);
	
	Gui_Drawbmp40(lcddev.width-50,0,bataryadolu);//pil dolu
	
	INT_TIM_Config();


	
	while(1)
	{
	

    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
	  RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
		
		sec = RTC_TimeStructure.RTC_Seconds;
		min = RTC_TimeStructure.RTC_Minutes;
		hour = RTC_TimeStructure.RTC_Hours;
		
		year = RTC_DateStructure.RTC_Year + 2000;
		month = RTC_DateStructure.RTC_Month;
		day = RTC_DateStructure.RTC_Date;
		weekday= RTC_DateStructure.RTC_WeekDay;
		
	   if(timeFlag){ 
		 GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
	   LCD_ShowString4824(100,0,":",1);
		 delay_ms(500);
		 LCD_ShowString4824(100,0," ",1);
		 timeFlag=0;
		 }

		sprintf((char *)TempStr, "%02d %02d", hour ,min);
		Show_Str4824(50,0,WHITE,BLACK,(u8*)TempStr,0);
	
		sprintf((char *)TempStr, "%d %s", day , strMonth[month-1]);
		Show_Str2412(0,50,WHITE,BLACK,(u8*)TempStr,0);
		 
		sprintf((char *)TempStr, "%s", strWeekday[weekday-1]);
		Show_Str2412(0,72,WHITE,BLACK,(u8*)TempStr,0);
		
    Touch_Test();

   }
}




int counterflag=0;
void Touch_Test(void)
{
		 tp_dev.scan(0); 
	
				 if((tp_dev.x>=190&&tp_dev.x<=230)&&(tp_dev.y>=105&&tp_dev.y<=145)&&(tp_dev.sta&TP_PRES_DOWN))
				 {     
					  // tp_dev.sta&=~(1<<7);//Mark the button to release
	             delay_ms(150);
					     counterflag++;
							if(counterflag==1){
								LED1;
								//LCD_Fill(0,100,lcddev.width,150,YELLOW);
								Gui_Drawbmp16(100,50,lambaon);

							}
							else if(counterflag==2) 
							{
								counterflag=0;							
								LED1=0;
							  Gui_Drawbmp16(100,50,lambaoff);

							}
	       } 	  
}







void DrawTestPage(u8 *str)
{

LCD_Clear(WHITE);
LCD_Fill(0,0,lcddev.width,20,BLUE);
LCD_Fill(0,lcddev.height-20,lcddev.width,lcddev.height,BLUE);
POINT_COLOR=WHITE;
Gui_StrCenter(0,2,WHITE,BLUE,str,16,1);
Gui_StrCenter(0,lcddev.height-18,WHITE,BLUE," Siemens A.S ",16,1);
}



void Test_FillRec(void)
{
	u8 i=0;
	DrawTestPage("GIRGIN");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	for (i=0; i<5; i++) 
	{
		POINT_COLOR=ColorTab[i];
		LCD_DrawRectangle(lcddev.width/2-80+(i*15),lcddev.height/2-80+(i*15),lcddev.width/2-80+(i*15)+60,lcddev.height/2-80+(i*15)+60); 
	}
	delay_ms(1500);	
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE); 
	for (i=0; i<5; i++) 
	{
		POINT_COLOR=ColorTab[i];
		LCD_DrawFillRectangle(lcddev.width/2-80+(i*15),lcddev.height/2-80+(i*15),lcddev.width/2-80+(i*15)+60,lcddev.height/2-80+(i*15)+60); 
	}
	delay_ms(1500);
}




