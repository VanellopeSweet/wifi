#ifndef __DHT11_H
#define	__DHT11_H

#include "stm32f10x.h"
#include "sys.h" 
#include "delay.h"

#define HIGH  1
#define LOW   0

typedef struct
{
	uint8_t  humi_int;		//ʪ�ȵ���������
	uint8_t  humi_deci;	 	//ʪ�ȵ�С������
	uint8_t  temp_int;	 	//�¶ȵ���������
	uint8_t  temp_deci;	 	//�¶ȵ�С������
	uint8_t  check_sum;	 	//У���
		                 
}DHT11_Data_TypeDef;
//IO��������
#define DHT11_IO_IN()  {GPIOB->CRH&=0XFFF0FFFF;GPIOB->CRH|=8<<16;}
#define DHT11_IO_OUT() {GPIOB->CRH&=0XFFF0FFFF;GPIOB->CRH|=3<<16;}
////IO��������											   
#define	DHT11_DQ_OUT PBout(12) //���ݶ˿�	PB12 
#define	DHT11_DQ_IN  PBin(12)  //���ݶ˿�	PB12  

u8 DHT11_Init(void);//��ʼ��DHT11
void DHT11_Rst(void);//��λDHT11 
u8 DHT11_Read_Byte(void);//����һ���ֽ�
u8 DHT11_Read_Bit(void);//����һ��λ
u8 DHT11_Check(void);//����Ƿ����DHT11

uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data);

#endif /* __DHT11_H */







