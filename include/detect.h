#ifndef __CUST_DETECT_H__
#define __CUST_DETECT_H__

enum
{


	SENSOR_READ_FIRST = 0,//������������
	SENSOR_READ,//��������ȡ��һ������
	SENSOR_DOWN	,//�������µ�
};
typedef struct
{
	u32 *Param;
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
void I2C_Down(void);
#endif
