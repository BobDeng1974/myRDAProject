#include "user.h"

Sensor_CtrlStruct __attribute__((section (".usr_ram"))) SensorCtrl;
extern GPIO_ParamStruct __attribute__((section (".usr_ram"))) PinParam[PIN_MAX];
void Detect_GSensorDown(void)
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


	SensorCtrl.GSensorState = SENSOR_DOWN;
}

void Detect_GSensorUp(void)
{
	GPIO_Write(GSENSOR_POWER_PIN,1);
	//GPIO_Write(TEST_PIN, 1);
	OS_I2COpen();
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_sysCtrl->Cfg_Reserved |= SYS_CTRL_UART1_TCO;
#endif
}

void Detect_GSensorBot(void)
{
	u16 ADCVal;
#ifdef __NO_G_SENSOR__
#else
	switch (SensorCtrl.GSensorState)
	{
	case SENSOR_READ_FIRST:
		MXC622X_ReadFirst(&SensorCtrl);
		break;
	case SENSOR_READ:
		MXC622X_Read(&SensorCtrl);
		break;
	case SENSOR_DOWN:
		Detect_GSensorUp();
		SensorCtrl.GSensorState = SENSOR_READ_FIRST;
		break;
	default:
		Detect_GSensorDown();
		break;
	}
#endif
	ADCVal = hal_AnaGpadcGet(HAL_ANA_GPADC_CHAN_0);
    if (0xFFFF != ADCVal)
    {
    	gSys.Var[ADC0_VAL] = ADCVal;
    }

}

void Detect_VACCIrqHandle(void)
{
	IO_ValueUnion Temp;
	Temp.IOVal.VCC = GPIO_Read(VCC_DET_PIN);
	Temp.IOVal.ACC = GPIO_Read(ACC_DET_PIN);
	Temp.IOVal.VACC = Temp.IOVal.ACC && Temp.IOVal.VCC;
	gSys.Var[IO_VAL] = Temp.Val;
	DBG("IO %d %d %d", Temp.IOVal.VCC, Temp.IOVal.ACC, Temp.IOVal.VACC);
#ifdef __UART_AUTO_SLEEP_BY_VACC__
	if (!Temp.IOVal.VACC)
	{
		COM_Sleep();
	}
	else
	{
		COM_Wakeup(gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_COM_BR]);
	}
#endif
}

void Detect_CrashCal(void)
{
	u32 A = SensorCtrl.CrashCnt;
	u32 LastA = gSys.Var[GSENSOR_VAL];
	SensorCtrl.CrashCnt = 0;
	DBG("%d", A);
	gSys.Var[GSENSOR_VAL] = A;
	gSys.Var[GSENSOR_ALARM_VAL] = (gSys.Var[GSENSOR_ALARM_VAL] < A)?A:gSys.Var[GSENSOR_ALARM_VAL];
	gSys.Var[GSENSOR_MONITOR_VAL] = (gSys.Var[GSENSOR_MONITOR_VAL] < A)?A:gSys.Var[GSENSOR_MONITOR_VAL];
	gSys.Var[GSENSOR_KEEP_VAL] = (gSys.Var[GSENSOR_KEEP_VAL] < A)?A:gSys.Var[GSENSOR_KEEP_VAL];
	if (!LastA && !A)
	{
		DBG("stop check!");
		OS_StopTimer(gSys.TaskID[MAIN_TASK_ID], G_SENSOR_TIMER_ID);
		SensorCtrl.CrashDetectOff = 1;
	}
}

void Detect_CrashIrqHandle(void)
{
	SensorCtrl.CrashCnt++;
	if (SensorCtrl.CrashDetectOff || !gSys.Var[GSENSOR_VAL])
	{
		SensorCtrl.CrashDetectOff = 0;
		OS_StartTimer(gSys.TaskID[MAIN_TASK_ID],
				G_SENSOR_TIMER_ID,
				COS_TIMER_MODE_PERIODIC,
				SYS_TICK/16);
		gSys.Var[GSENSOR_VAL] = SensorCtrl.CrashCnt;
	}
}

void Detect_Config(void)
{
	HAL_GPIO_CFG_T DetectIrqCfg;
	IO_ValueUnion Temp;
	SensorCtrl.GSensorState = SENSOR_READ_FIRST;
	SensorCtrl.LastX = 0;
	SensorCtrl.LastY = 0;
	SensorCtrl.Param = gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param;
	SensorCtrl.CrashCnt++;
	SensorCtrl.CrashDetectOff = 1;
	Detect_GSensorDown();
#if (__CUST_CODE__ == __CUST_KQ__)
	SYS_Error(SENSOR_ERROR, 0);
	Temp.IOVal.ACC = 1;
	Temp.IOVal.VCC = 1;
	Temp.IOVal.VACC = 1;
	gSys.Var[IO_VAL] = Temp.Val;
#else
	Temp.IOVal.VCC = GPIO_Read(VCC_DET_PIN);
	Temp.IOVal.ACC = GPIO_Read(ACC_DET_PIN);
	Temp.IOVal.VACC = Temp.IOVal.ACC && Temp.IOVal.VCC;
	gSys.Var[IO_VAL] = Temp.Val;
#endif
	gSys.Var[VBAT] = OS_GetVbatADC();
	DetectIrqCfg.direction = HAL_GPIO_DIRECTION_INPUT;
	DetectIrqCfg.irqMask.debounce = TRUE;
	DetectIrqCfg.irqMask.level = FALSE;
	DetectIrqCfg.irqMask.falling = TRUE;
	DetectIrqCfg.irqMask.rising = TRUE;

	DetectIrqCfg.irqHandler = Detect_VACCIrqHandle;
	OS_GPIOInit(PinParam[VCC_DET_PIN].APO.gpioId, &DetectIrqCfg);
	DetectIrqCfg.irqHandler = Detect_VACCIrqHandle;
	OS_GPIOInit(PinParam[ACC_DET_PIN].APO.gpioId, &DetectIrqCfg);

}

