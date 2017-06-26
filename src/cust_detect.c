#include "user.h"

Sensor_CtrlStruct __attribute__((section (".usr_ram"))) SensorCtrl;
void I2C_Down(void)
{
	OS_I2CClose();

	GPIO_Write(I2C_SDA_PIN, 0);
	GPIO_Write(I2C_SCL_PIN, 0);
#if (__CUST_CODE__ == __CUST_KQ__)
#else
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_sysCtrl->Cfg_Reserved &= ~SYS_CTRL_UART1_TCO;

#endif
	GPIO_Write(GSENSOR_POWER_PIN, 0);
#endif


	SensorCtrl.SensorState = SENSOR_DOWN;
}

void I2C_Up(void)
{
	GPIO_Write(GSENSOR_POWER_PIN,1);
	//GPIO_Write(TEST_PIN, 1);
	OS_I2COpen();
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_sysCtrl->Cfg_Reserved |= SYS_CTRL_UART1_TCO;
#endif
}

s32 Detect_Flush(void *pData)
{
//	double D;
//	u32 U;
	IO_ValueUnion Temp;
	if (SensorCtrl.Param[PARAM_SENSOR_EN])
	{
		Temp.Val = 0;
		Temp.IOVal.VCC = GPIO_Read(VCC_DET_PIN);
		Temp.IOVal.ACC = GPIO_Read(ACC_DET_PIN);
		Temp.IOVal.VACC = Temp.IOVal.ACC && Temp.IOVal.VCC;
		if (gSys.Var[IO_VAL] != Temp.Val)
		{
			gSys.Var[IO_VAL] = Temp.Val;
			DBG("IO %d %d %d", Temp.IOVal.VCC, Temp.IOVal.ACC, Temp.IOVal.VACC);
		}
		switch (SensorCtrl.SensorState)
		{

		case SENSOR_READ_FIRST:
			MXC622X_ReadFirst(&SensorCtrl);
			break;
		case SENSOR_READ:
			MXC622X_Read(&SensorCtrl);
			break;
		case SENSOR_DOWN:
			I2C_Up();
			SensorCtrl.SensorState = SENSOR_READ_FIRST;
			break;
		default:
			I2C_Down();
			break;
		}
	}
	else
	{
#if (__CUST_CODE__ == __CUST_KQ__)
		Temp.IOVal.ACC = 1;
		Temp.IOVal.VCC = 1;
		Temp.IOVal.VACC = 1;
		gSys.Var[IO_VAL] = Temp.Val;
#else
		Temp.IOVal.ACC = 0;
		Temp.IOVal.VCC = 1;
		Temp.IOVal.VACC = 0;
		gSys.Var[IO_VAL] = Temp.Val;
#endif
	}

	gSys.Var[VBAT] = (gSys.Var[VBAT] * 9 + OS_GetVbatADC()) / 10;
	GPS_StateCheck();
	Alarm_StateCheck();
	Monitor_StateCheck();
	return 0;
}

void Detect_Config(void)
{
	SensorCtrl.SensorState = SENSOR_READ_FIRST;
	SensorCtrl.LastX = 0;
	SensorCtrl.LastX = 1;
	SensorCtrl.Param = gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param;
	gSys.Var[IO_VAL] = 0xffffffff;
	I2C_Down();
#if (__CUST_CODE__ == __CUST_KQ__)
	SYS_Error(SENSOR_ERROR, 0);
#endif
	gSys.Var[VBAT] = OS_GetVbatADC();
}

