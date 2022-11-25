
#ifndef __LED_H
#define __LED_H
#include "sys.h"

#define LED0 PDout(12)	 
#define LED1 PDout(13)		 

void LED_Init(void);		 				    
#endif
