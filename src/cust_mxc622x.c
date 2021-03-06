#include "user.h"
#define MXC6225_I2C_ADDR (0x15)
#define MXC6226_ID (0x25)
#define MXC622X_XOUT_REG (0x00)
#if (__BOARD__ == __AIR200__)
#define I2C_BUS HAL_I2C_BUS_ID_2
#endif

#if (__BOARD__ == __AIR201__ || __BOARD__ == __AIR202__)
#define I2C_BUS HAL_I2C_BUS_ID_3
#endif
void MXC622X_ReadFirst(Sensor_CtrlStruct *Sensor)
{
	int8_t Data[9];
	int32_t Error;
	uint8_t Reg = MXC622X_XOUT_REG;
	Error = OS_I2CXfer(I2C_BUS, MXC6225_I2C_ADDR, &Reg, 1, Data, 9, 0, 10);
	if (Error)
	{
		//if (!gSys.Error[SENSOR_ERROR])
		//{
			DBG("i2c error %u",Error);
		//}
		Sensor->GSensorState = SENSOR_DOWN;
		Detect_GSensorDown();
		SYS_Error(SENSOR_ERROR, 1);
	}
	else
	{
		if ((Data[8] & 0x3f) == MXC6226_ID)
		{
			Sensor->Last8BitX = Data[0];
			Sensor->Last8BitY = Data[1];
			SYS_Error(SENSOR_ERROR, 0);
			Sensor->GSensorState = SENSOR_READ;
			Sensor->Firstread = 2;
		}
		else
		{
			DBG("gsensor error");
			HexTrace(Data, 9);
			Sensor->GSensorState = SENSOR_DOWN;
			Detect_GSensorDown();
			SYS_Error(SENSOR_ERROR, 1);
		}
	}
}

void MXC622X_Read(Sensor_CtrlStruct *Sensor)
{
	int8_t Data[9];
	int32_t Error,X,Y,A;
	uint8_t Reg = MXC622X_XOUT_REG;
	Error = OS_I2CXfer(I2C_BUS, MXC6225_I2C_ADDR, &Reg, 1, Data, 9, 0, 10);
	if (Error)
	{
		DBG("i2c error %u",Error);
		Sensor->GSensorState = SENSOR_DOWN;
		Detect_GSensorDown();
		SYS_Error(SENSOR_ERROR, 1);
	}
	else
	{
		if ((Data[8] & 0x3f) == MXC6226_ID)
		{
			X = (int32_t)Data[0] - (int32_t)Sensor->Last8BitX;
			Y = (int32_t)Data[1] - (int32_t)Sensor->Last8BitY;
			Sensor->Last8BitX = Data[0];
			Sensor->Last8BitY = Data[1];
			if (Sensor->Firstread)
			{
				Sensor->Firstread--;
				return;
			}
			A = X*X +Y*Y;
			gSys.Var[GSENSOR_VAL] = A;
			gSys.Var[GSENSOR_ALARM_VAL] = (gSys.Var[GSENSOR_ALARM_VAL] < A)?A:gSys.Var[GSENSOR_ALARM_VAL];
			gSys.Var[GSENSOR_MONITOR_VAL] = (gSys.Var[GSENSOR_MONITOR_VAL] < A)?A:gSys.Var[GSENSOR_MONITOR_VAL];
			gSys.Var[GSENSOR_KEEP_VAL] = (gSys.Var[GSENSOR_KEEP_VAL] < A)?A:gSys.Var[GSENSOR_KEEP_VAL];
			if (A >= 1000)
			{
				DBG("%u", A);
			}
			else if (A >= 100)
			{
				CORE("%u", A);
			}
		}
		else
		{
			DBG("gsensor error %02x",Data[8]);
			Detect_GSensorDown();
			SYS_Error(SENSOR_ERROR, 1);
		}
	}
}
