#ifndef __CUST_DETECT_H__
#define __CUST_DETECT_H__

enum
{


	SENSOR_READ_FIRST = 0,//传感器读数据
	SENSOR_READ,//传感器读取第一次数据
	SENSOR_DOWN	,//传感器下电
};
typedef struct
{
	u32 *Param;
	u32 CrashCnt;
	u8 GSensorState;
	s8 LastX;
	s8 LastY;
	u8 Firstread;
	u8 CrashDetectOff;
}Sensor_CtrlStruct;

typedef struct
{
	u8 VCC;
	u8 ACC;
	u8 VACC;
	u8 unuse;
}IO_ValueStruct;

typedef union
{
	u32 Val;
	IO_ValueStruct IOVal;
}IO_ValueUnion;
void Detect_Config(void);
void Detect_GSensorBot(void);
void Detect_ADC0Cal(void);
void Detect_CrashCal(void);
void Detect_GSensorDown(void);
void Detect_GSensorUp(void);
#endif
