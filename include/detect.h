#ifndef __CUST_DETECT_H__
#define __CUST_DETECT_H__

enum
{
	LED_OFF,
	LED_ON,
	LED_FLUSH_SLOW,
	LED_FLUSH_FAST,

	SENSOR_READ_FIRST = 0,//传感器读数据
	SENSOR_READ,//传感器读取第一次数据
	SENSOR_DOWN	,//传感器下电
};
typedef struct
{
	u32 *Param;
	u8 Delay;
	u8 NetType;
	u8 NetState;
	u8 GPSType;
	u8 GPSState;
	u8 SensorState;
	s8 LastX;
	s8 LastY;

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
s32 Detect_Flush(void *pData);
void Led_FlushType(u8 Pin, u8 NewType);
void I2C_Down(void);
#endif
