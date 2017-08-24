#ifndef __CUST_DETECT_H__
#define __CUST_DETECT_H__

enum
{


	SENSOR_READ_FIRST = 0,//传感器读数据
	SENSOR_READ,//传感器读取第一次数据
	SENSOR_DOWN	,//传感器下电
	SENSOR_BREAK,
};
typedef struct
{
	uint32_t *Param;
	uint32_t CrashCnt;
	int16_t Last16Bit[3];
	uint8_t GSensorState;
	int8_t Last8BitX;
	int8_t Last8BitY;
	uint8_t Firstread;
	uint8_t ResetCnt;
	uint8_t CrashDetectOff;
	uint8_t ADCChannel;
	uint16_t Vol;
	int16_t BattryTempture;
	int16_t EnvTempture;
	uint32_t Vref;
}Sensor_CtrlStruct;

typedef struct
{
	uint8_t VCC;
	uint8_t ACC;
	uint8_t VACC;
	uint8_t unuse;
}IO_ValueStruct;

typedef union
{
	uint32_t Val;
	IO_ValueStruct IOVal;
}IO_ValueUnion;
void Detect_Config(void);
void Detect_GSensorBot(void);
void Detect_ADC0Cal(void);
void Detect_CrashCal(void);
void Detect_GSensorDown(void);
void Detect_GSensorUp(void);
void Detect_VACCIrqHandle(void);
#endif
