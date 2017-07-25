#include "user.h"
#define LIS3DH_I2C_ADDR (0x18)
#define LIS3DH_ID (0x33)
#define LIS3DH_STATUS_REG (0x07)
#define LIS3DH_P_MAX	(2047)
#define LIS3DH_N_MAX	(-2048)
#if (__BOARD__ == __AIR200__)
#define I2C_BUS HAL_I2C_BUS_ID_2
#endif

#if (__BOARD__ == __AIR201__ || __BOARD__ == __AIR202__)
#define I2C_BUS HAL_I2C_BUS_ID_3
#endif
void LIS3DH_ReadFirst(Sensor_CtrlStruct *Sensor)
{
	s8 Data[9];
	s32 Error;
	u8 i;
	u8 Reg = LIS3DH_STATUS_REG;
	Error = OS_I2CXfer(I2C_BUS, LIS3DH_I2C_ADDR, &Reg, 1, Data, 9, 0, 10);
	if (Error)
	{
		DBG("i2c error %u",Error);
		Sensor->GSensorState = SENSOR_DOWN;
		Detect_GSensorDown();
		SYS_Error(SENSOR_ERROR, 1);
	}
	else
	{
		if (Data[8] == LIS3DH_ID)
		{
			for (i = 0; i < 3; i++)
			{
				memcpy(&Sensor->Last16Bit[i], &Data[i * 2 + 1], 2);
			}
			SYS_Error(SENSOR_ERROR, 0);
			Sensor->GSensorState = SENSOR_READ;
			Sensor->Firstread = 2;
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

void LIS3DH_Read(Sensor_CtrlStruct *Sensor)
{
	s8 Data[9];
	u8 i;
	s16 Temp[3];
	s32 Error,X,Y,Z;
	u32 A;
	u8 Reg = LIS3DH_STATUS_REG;
	Error = OS_I2CXfer(I2C_BUS, LIS3DH_I2C_ADDR, &Reg, 1, Data, 9, 0, 10);
	if (Error)
	{
		DBG("i2c error %u",Error);
		Sensor->GSensorState = SENSOR_DOWN;
		Detect_GSensorDown();
		SYS_Error(SENSOR_ERROR, 1);
	}
	else
	{
		if (Data[8] == LIS3DH_ID)
		{
			for (i = 0; i < 3; i++)
			{
				memcpy(&Temp[i], &Data[i * 2 + 1], 2);
				if ( (Temp[i] > LIS3DH_P_MAX) || (Temp[0] < LIS3DH_N_MAX) )
				{
					DBG("%u", Temp[i]);
					return ;
				}

			}


			X = (s32)Temp[0] - (s32)Sensor->Last16Bit[0];
			Y = (s32)Temp[1] - (s32)Sensor->Last16Bit[1];
			Z = (s32)Temp[2] - (s32)Sensor->Last16Bit[2];

			for (i = 0; i < 3; i++)
			{
				Sensor->Last16Bit[i] = Temp[i];
			}

			if (Sensor->Firstread)
			{
				Sensor->Firstread--;
				return;
			}
			A = (X*X + Y*Y + Z*Z) / 256;
			gSys.Var[GSENSOR_VAL] = A;
			gSys.Var[GSENSOR_ALARM_VAL] = (gSys.Var[GSENSOR_ALARM_VAL] < A)?A:gSys.Var[GSENSOR_ALARM_VAL];
			gSys.Var[GSENSOR_MONITOR_VAL] = (gSys.Var[GSENSOR_MONITOR_VAL] < A)?A:gSys.Var[GSENSOR_MONITOR_VAL];
			gSys.Var[GSENSOR_KEEP_VAL] = (gSys.Var[GSENSOR_KEEP_VAL] < A)?A:gSys.Var[GSENSOR_KEEP_VAL];
			if (A >= 100)
			{
				DBG("%u %u %u %u", Temp[0], Temp[1], Temp[2], A);
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
