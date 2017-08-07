#include "user.h"
#define LIS3DH_I2C_ADDR (0x18)
#define LIS3DH_ID (0x33)
#define LIS3DH_STATUS_REG 	(0x07)
#define LIS3DH_OUTX_L 		(0x28)
#define LIS3DH_WHO_AM_I		(0x0f)
#define LIS3DH_CTRL1_REG	(0x20)
#define LIS3DH_CTRL4_REG	(0x23)

#define LIS3DH_P_MAX	(2047)
#define LIS3DH_N_MAX	(-2048)
#if (__BOARD__ == __AIR200__)
#define I2C_BUS HAL_I2C_BUS_ID_2
#endif

#if (__BOARD__ == __AIR201__ || __BOARD__ == __AIR202__)
#define I2C_BUS HAL_I2C_BUS_ID_3
#endif

uint8_t LIS3DH_WriteReg(uint8_t Reg, uint8_t Data)
{
	int32_t Error;
	uint8_t OrgData = Data;
	Error = OS_I2CXfer(I2C_BUS, LIS3DH_I2C_ADDR, &Reg, 1, &Data, 1, 1, 10);
	if (Error)
	{
		if (!gSys.Error[SENSOR_ERROR])
		{
			DBG("i2c error %u",Error);
		}
		return 0;
	}
	Data = 0;
	Error = OS_I2CXfer(I2C_BUS, LIS3DH_I2C_ADDR, &Reg, 1, &Data, 1, 0, 10);
	if (Error)
	{
		if (!gSys.Error[SENSOR_ERROR])
		{
			DBG("i2c error %u",Error);
		}
		return 0;
	}

	if (Data != OrgData)
	{
		DBG("write %02x result %02x %02x", Reg, OrgData, Data);
		return 0;
	}
	return 1;
}

uint8_t LIS3DH_ReadReg(uint8_t Reg, uint8_t *Data, uint8_t Num)
{
	int32_t Error;
	Error = OS_I2CXfer(I2C_BUS, LIS3DH_I2C_ADDR, &Reg, 1, Data, Num, 0, 10);
	if (Error)
	{
		if (!gSys.Error[SENSOR_ERROR])
		{
			DBG("i2c error %u",Error);
		}
		return 0;
	}
	return 1;
}

void LIS3DH_ReadFirst(Sensor_CtrlStruct *Sensor)
{
	uint8_t Data[1];
#ifdef __PLATFORM_8955__
	hal_SysRequestFreq((HAL_SYS_FREQ_USER_ID_T)(HAL_SYS_FREQ_APP_USER_0 + CSW_LP_RESOURCE_UNUSED_2), (HAL_SYS_FREQ_T)CSW_SYS_FREQ_104M, NULL);
#endif
	if (!LIS3DH_WriteReg(LIS3DH_CTRL1_REG, 0x47))
	{
		goto ERROR_OUT;
	}

	if (!LIS3DH_WriteReg(LIS3DH_CTRL4_REG, 0x08))
	{
		goto ERROR_OUT;
	}

	if (!LIS3DH_ReadReg(LIS3DH_WHO_AM_I, Data, 1))
	{
		goto ERROR_OUT;
	}
	else if (Data[0] != LIS3DH_ID)
	{
		DBG("%02x", Data[0]);
		goto ERROR_OUT;
	}
	SYS_Error(SENSOR_ERROR, 0);
	Sensor->GSensorState = SENSOR_READ;
	Sensor->Firstread = 2;
#ifdef __PLATFORM_8955__
	hal_SysRequestFreq((HAL_SYS_FREQ_USER_ID_T)(HAL_SYS_FREQ_APP_USER_0 + CSW_LP_RESOURCE_UNUSED_2), (HAL_SYS_FREQ_T)CSW_SYS_FREQ_32K, NULL);
#endif
	return ;
ERROR_OUT:
	Sensor->GSensorState = SENSOR_DOWN;
	Detect_GSensorDown();
	SYS_Error(SENSOR_ERROR, 1);
#ifdef __PLATFORM_8955__
	hal_SysRequestFreq((HAL_SYS_FREQ_USER_ID_T)(HAL_SYS_FREQ_APP_USER_0 + CSW_LP_RESOURCE_UNUSED_2), (HAL_SYS_FREQ_T)CSW_SYS_FREQ_32K, NULL);
#endif
	return ;
}

void LIS3DH_Read(Sensor_CtrlStruct *Sensor)
{
	uint8_t Data[2];
	uint8_t i ,flag;
	int16_t Temp[3];
	int32_t X,Y,Z;
	uint32_t A;
#ifdef __PLATFORM_8955__
	hal_SysRequestFreq((HAL_SYS_FREQ_USER_ID_T)(HAL_SYS_FREQ_APP_USER_0 + CSW_LP_RESOURCE_UNUSED_2), (HAL_SYS_FREQ_T)CSW_SYS_FREQ_104M, NULL);
#endif
	if (!LIS3DH_ReadReg(LIS3DH_WHO_AM_I, Data, 1))
	{
		goto ERROR_OUT;
	}
	else if (Data[0] != LIS3DH_ID)
	{
		DBG("%02x", Data[0]);
		goto ERROR_OUT;
	}

	for (i = 0; i < 3; i++)
	{
		if (!LIS3DH_ReadReg(LIS3DH_OUTX_L + 2 * i, Data, 1))
		{
			goto ERROR_OUT;
		}

		if (!LIS3DH_ReadReg(LIS3DH_OUTX_L + 2 * i + 1, Data + 1, 1))
		{
			goto ERROR_OUT;
		}

		memcpy(&Temp[i], &Data[0], 2);

		flag = 0;
		if (Temp[i] < 0)
		{
			flag = 1;
			Temp[i] = -Temp[i];
		}
		Temp[i] = (Temp[i] >> 4);
		if (flag)
		{
			Temp[i] = -Temp[i];
		}
		if ( (Temp[i] > LIS3DH_P_MAX) || (Temp[i] < LIS3DH_N_MAX) )
		{
			DBG("%d % %d", i, Temp[i]);
		}
	}
	//DBG("%d %d %d", Temp[0], Temp[1], Temp[2]);
	X = (int32_t)Temp[0] - (int32_t)Sensor->Last16Bit[0];
	Y = (int32_t)Temp[1] - (int32_t)Sensor->Last16Bit[1];
	Z = (int32_t)Temp[2] - (int32_t)Sensor->Last16Bit[2];

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
		DBG("%d %d %d %u", Temp[0], Temp[1], Temp[2], A);
	}
#ifdef __PLATFORM_8955__
	hal_SysRequestFreq((HAL_SYS_FREQ_USER_ID_T)(HAL_SYS_FREQ_APP_USER_0 + CSW_LP_RESOURCE_UNUSED_2), (HAL_SYS_FREQ_T)CSW_SYS_FREQ_32K, NULL);
#endif
	return ;
ERROR_OUT:
	Sensor->GSensorState = SENSOR_DOWN;
	Detect_GSensorDown();
	SYS_Error(SENSOR_ERROR, 1);
#ifdef __PLATFORM_8955__
	hal_SysRequestFreq((HAL_SYS_FREQ_USER_ID_T)(HAL_SYS_FREQ_APP_USER_0 + CSW_LP_RESOURCE_UNUSED_2), (HAL_SYS_FREQ_T)CSW_SYS_FREQ_32K, NULL);
#endif
	return ;
}
