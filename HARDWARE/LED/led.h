#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#define LED PCout(13)	//远程控制路灯
#define JDQ_FS PCout(14)	// 远程控制空调
#define JDQ_XYJ PCout(15)	// 远程控制洗衣机

#define LED_OFF1	1
#define LED_ON1		0

void LED_Init(void);//初始化
			    
#endif
