//单片机头文件
#include "stm32f10x.h"

//网络设备
#include "esp8266.h"

//协议文件
#include "onenet.h"
#include "mqttkit.h"

//硬件驱动
#include "usart.h"
#include "led.h"
//C库
#include <string.h>
#include <stdio.h>

#define PROID		"566124"       //产品ID

#define AUTH_INFO	"20010805TYQ"  //鉴权信息

#define DEVID		"1026894156"	//设备ID

extern unsigned char esp8266_buf[128];

//==========================================================
//	函数名称：	OneNet_DevLink
//
//	函数功能：	与onenet创建连接
//
//	入口参数：	无
//
//	返回参数：	1-成功	0-失败
//
//	说明：		与onenet平台建立连接
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//协议包

	unsigned char *dataPtr;
	
	_Bool status = 1;
	
	printf("OneNet_DevLink\r\nPROID: %s,	AUIF: %s,	DEVID:%s\r\n", PROID, AUTH_INFO, DEVID);
	
	if(MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//上传平台
		
		dataPtr = ESP8266_GetIPD(250);									//等待平台响应
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0:printf("Tips:	连接成功\r\n");status = 0;break;
					
					case 1:printf("WARN:	连接失败：协议错误\r\n");break;
					case 2:printf("WARN:	连接失败：非法的clientid\r\n");break;
					case 3:printf("WARN:	连接失败：服务器失败\r\n");break;
					case 4:printf("WARN:	连接失败：用户名或密码错误\r\n");break;
					case 5:printf("WARN:	连接失败：非法链接(比如token非法)\r\n");break;
					
					default:printf("ERR:	连接失败：未知错误\r\n");break;
				}
			}
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//删包
	}
	else
		printf("WARN:	MQTT_PacketConnect Failed\r\n");
	
	return status;
	
}

u8 key_LD = 0;    //开关路灯
u8 key_FS = 0;       //开关空调
u8 key_XYJ = 0;   //远程开关洗衣机
extern u8 humidityH;	  //湿度整数部分
extern u8 humidityL;	  //湿度小数部分
extern u8 temperatureH;   //温度整数部分
extern u8 temperatureL;   //温度小数部分
unsigned char OneNet_FillBuf(char *buf)
{
	char text[32];
	memset(text, 0, sizeof(text));
	
	strcpy(buf, ",;");
		
	memset(text, 0, sizeof(text));
	sprintf(text, "key_LD,%d;", key_LD);   
	strcat(buf, text);
    
    memset(text, 0, sizeof(text));
	sprintf(text, "key_FS,%d;", key_FS);   
	strcat(buf, text);
    
    memset(text, 0, sizeof(text));
	sprintf(text, "key_XYJ,%d;", key_XYJ);   
	strcat(buf, text);
	   
	memset(text, 0, sizeof(text));
	sprintf(text, "Tempreture,%d.%d;",temperatureH,temperatureL);
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "Humidity,%d.%d;",humidityH,humidityL);
	strcat(buf, text);
    
	return strlen(buf);
}


//==========================================================
//	函数名称：	OneNet_SendData
//
//	函数功能：	上传数据到平台
//
//	入口参数：	type：发送数据的格式
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_SendData(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};												//协议包
	
	char buf[128];
	
	short body_len = 0, i = 0;
	
//	printf("Tips:	OneNet_SendData-MQTT\r\n");
	
	memset(buf, 0, sizeof(buf));
	
	body_len = OneNet_FillBuf(buf);																	//获取当前需要发送的数据流的总长度
	
	if(body_len)
	{
		if(MQTT_PacketSaveData(DEVID, body_len, NULL, 5, &mqttPacket) == 0)							//封包
		{
			for(; i < body_len; i++)
				mqttPacket._data[mqttPacket._len++] = buf[i];
			
			ESP8266_SendData(mqttPacket._data, mqttPacket._len);									//上传数据到平台
//			printf("Send %d Bytes\r\n", mqttPacket._len);		
			MQTT_DeleteBuffer(&mqttPacket);															//删包
		}
		else
			printf("WARN:	EDP_NewBuffer Failed\r\n");
	}
	
}

//==========================================================
//	函数名称：	OneNet_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{

MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//协议包

char *req_payload = NULL;
char *cmdid_topic = NULL;

unsigned short req_len = 0;
  unsigned char type = 0;

short result = 0;

char *dataPtr = NULL;
char numBuf[10];
int num = 0;

type = MQTT_UnPacketRecv(cmd);//MQTT数据接收类型判断
switch(type)
{
	case MQTT_PKT_CMD:															//命令下发
		
		//参数1收到的
		result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);	//解出topic和消息体
		if(result == 0)
		{
			//打印收到的信息，参数2数据，参数3数据长度
			printf(  "cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);

			if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)	//命令回复组包
			{
				printf( "Tips:	Send CmdResp\r\n");
				
				ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//回复命令
				MQTT_DeleteBuffer(&mqttPacket);									//删包
			}
		}
	break;
		
	case MQTT_PKT_PUBACK:														//发送Publish消息，平台回复的Ack
	
		if(MQTT_UnPacketPublishAck(cmd) == 0)
			printf(  "Tips:	MQTT Publish Send OK\r\n");	
	break;
	
	default:
		result = -1;
	break;
}

ESP8266_Clear();									//清空缓存

if(result == -1)
	return;

dataPtr = strchr(req_payload, ':');					//搜索':'

if(dataPtr != NULL && result != -1)					//如果找到了
{
	dataPtr++;
	
	while(*dataPtr >= '0' && *dataPtr <= '9')		//判断是否是下发的命令控制数据
	{
		numBuf[num++] = *dataPtr++;
	}
	numBuf[num] = 0;
	
	num = atoi((const char *)numBuf);				//转为数值形式
	
//        /****************************************************/	
	if(strstr((char *)req_payload, "key_LD"))		//搜索"key_LD"
	{
		if(num == 1)								//控制数据如果为1，代表开
		{
			LED=1;
		}
		else if(num == 0)							//控制数据如果为0，代表关
		{
			LED=0;
		}
        key_LD = num;                		 //更新数据到云平台
	}
    
    if(strstr((char *)req_payload, "key_FS"))		//搜索"key_FS"
	{
		if(num == 1)								//控制数据如果为0，代表开
		{
			JDQ_FS = 0;
		}
		else if(num == 0)							//控制数据如果为1，代表关
		{
			JDQ_FS = 1;
		}
        key_FS = num;                		 //更新数据到云平台
	}
    
    if(strstr((char *)req_payload, "key_XYJ"))		//搜索"key_FS"
	{
		if(num == 1)								//控制数据如果为0，代表开
		{
			JDQ_XYJ = 0;
		}
		else if(num == 0)							//控制数据如果为1，代表关
		{
			JDQ_XYJ = 1;
		}
        key_XYJ = num;                		 //更新数据到云平台
	}

}

    if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
    {
        MQTT_FreeBuffer(cmdid_topic);
        MQTT_FreeBuffer(req_payload);
    }

}
