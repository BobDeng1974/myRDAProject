#include "user.h"
#define MXC6225_I2C_ADDR (0x15)
#define MXC6226_ID (0x25)
#define MXC622X_XOUT_REG (0x00)

void MXC622X_ReadFirst(Sensor_CtrlStruct *Sensor)
{
	s8 Data[9];
	s32 Error,X,Y,A;
	u8 Reg = MXC622X_XOUT_REG;
	Error = OS_I2CXfer(MXC6225_I2C_ADDR, &Reg, 1, Data, 9, 0);
	if (Error)
	{
		DBG("i2c error %d",Error);
		Sensor->GSensorState = SENSOR_DOWN;
		Detect_GSensorDown();
		SYS_Error(SENSOR_ERROR, 1);
	}
	else
	{
		if ((Data[8] & 0x3f) == MXC6226_ID)
		{
			Sensor->LastX = Data[0];
			Sensor->LastY = Data[1];
			SYS_Error(SENSOR_ERROR, 0);
			Sensor->GSensorState = SENSOR_READ;
			Sensor->Firstread = 1;
		}
		else
		{
			DBG("gsensor error");
			__HexTrace(Data, 9);
			Sensor->GSensorState = SENSOR_DOWN;
			Detect_GSensorDown();
			SYS_Error(SENSOR_ERROR, 1);
		}
	}
}

void MXC622X_Read(Sensor_CtrlStruct *Sensor)
{
	s8 Data[9];
	s32 Error,X,Y,A;
	u8 Reg = MXC622X_XOUT_REG;
	Error = OS_I2CXfer(MXC6225_I2C_ADDR, &Reg, 1, Data, 9, 0);
	if (Error)
	{
		DBG("i2c error %d",Error);
		Sensor->GSensorState = SENSOR_DOWN;
		Detect_GSensorDown();
		SYS_Error(SENSOR_ERROR, 1);
	}
	else
	{
		if ((Data[8] & 0x3f) == MXC6226_ID)
		{
			X = (s32)Data[0] - (s32)Sensor->LastX;
			Y = (s32)Data[1] - (s32)Sensor->LastY;
			Sensor->LastX = Data[0];
			Sensor->LastY = Data[1];
			if (Sensor->Firstread)
			{
				Sensor->Firstread = 0;
				return;
			}
			A = X*X +Y*Y;
			gSys.Var[GSENSOR_VAL] = A;
			gSys.Var[GSENSOR_ALARM_VAL] = (gSys.Var[GSENSOR_ALARM_VAL] < A)?A:gSys.Var[GSENSOR_ALARM_VAL];
			gSys.Var[GSENSOR_MONITOR_VAL] = (gSys.Var[GSENSOR_MONITOR_VAL] < A)?A:gSys.Var[GSENSOR_MONITOR_VAL];
			gSys.Var[GSENSOR_KEEP_VAL] = (gSys.Var[GSENSOR_KEEP_VAL] < A)?A:gSys.Var[GSENSOR_KEEP_VAL];
			if (A >= 100)
			{
				DBG("%d", A);
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
