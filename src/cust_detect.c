#include "user.h"
extern void MXC622X_ReadFirst(Sensor_CtrlStruct *Sensor);
extern void MXC622X_Read(Sensor_CtrlStruct *Sensor);
extern void LIS3DH_ReadFirst(Sensor_CtrlStruct *Sensor);
extern void LIS3DH_Read(Sensor_CtrlStruct *Sensor);
Sensor_CtrlStruct __attribute__((section (".usr_ram"))) SensorCtrl;
extern GPIO_ParamStruct __attribute__((section (".usr_ram"))) PinParam[PIN_MAX];
#if (__CUST_CODE__ == __CUST_LY_IOTDEV__)
extern Monitor_CtrlStruct __attribute__((section (".usr_ram"))) LYCtrl;
#endif

u32 Detect_CalTempture(u32 R)
{
	double Temp;
	u32 Result;
	Temp = 762.9 * pow(R, -0.1232) - 159.9;
	Result = Temp;
	return Result;
}

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
	switch (SensorCtrl.GSensorState)
	{
	case SENSOR_READ_FIRST:
		G_SENSOR_READFIRST(&SensorCtrl);
		break;
	case SENSOR_READ:
		G_SENSOR_READ(&SensorCtrl);
		break;
	case SENSOR_DOWN:
		Detect_GSensorUp();
		SensorCtrl.GSensorState = SENSOR_READ_FIRST;
		break;
	default:
		Detect_GSensorDown();
		break;
	}
}

void Detect_ADC0Cal(void)
{
	u16 ADCVal = hal_AnaGpadcGetRaw(HAL_ANA_GPADC_CHAN_0);
#if (__CUST_CODE__ == __CUST_LY_IOTDEV__)
	IO_ValueUnion Temp;
	LY_CustDataStruct *LY = (LY_CustDataStruct *)LYCtrl.CustData;
	if (ADCVal != 0xFFFF)
	{
		gSys.Var[ADC0_VAL] = ADCVal;
		if (gSys.Var[ADC0_VAL])
		{
			//DBG("%u", gSys.Var[ADC0_VAL]);
		}
	}
	switch (LY->ADCChannel)
	{
	case LY_IOT_ADC_CH_BAT_TEMP:
		GPIO_Write(ADC_SELECT_0_PIN, 1);
		GPIO_Write(ADC_SELECT_1_PIN, 0);
		LY->ADCChannel = LY_IOT_ADC_CH_ENV_TEMP;
		break;
	case LY_IOT_ADC_CH_ENV_TEMP:
		GPIO_Write(ADC_SELECT_0_PIN, 0);
		GPIO_Write(ADC_SELECT_1_PIN, 1);
		LY->ADCChannel = LY_IOT_ADC_CH_BAT_VOL;
		break;
	case LY_IOT_ADC_CH_BAT_VOL:
		Temp.IOVal.VCC = (LY->Vol > 100)?1:0;
		Temp.IOVal.ACC = GPIO_Read(ACC_DET_PIN);
		Temp.IOVal.VACC = Temp.IOVal.ACC && Temp.IOVal.VCC;
		gSys.Var[IO_VAL] = Temp.Val;
		GPIO_Write(ADC_SELECT_0_PIN, 0);
		GPIO_Write(ADC_SELECT_1_PIN, 0);
		LY->ADCChannel = LY_IOT_ADC_CH_BAT_TEMP;
		break;
	default:
		GPIO_Write(ADC_SELECT_0_PIN, 0);
		GPIO_Write(ADC_SELECT_1_PIN, 0);
		LY->ADCChannel = LY_IOT_ADC_CH_BAT_TEMP;
		break;
	}
#endif

}
void Detect_VACCIrqHandle(void)
{
	IO_ValueUnion Temp;
#if (__CUST_CODE__ == __CUST_LY_IOTDEV__)
	LY_CustDataStruct *LY = (LY_CustDataStruct *)LYCtrl.CustData;
	Temp.IOVal.VCC = (LY->Vol > 100)?1:0;
#else
	Temp.IOVal.VCC = GPIO_Read(VCC_DET_PIN);
#endif
	Temp.IOVal.ACC = GPIO_Read(ACC_DET_PIN);
	Temp.IOVal.VACC = Temp.IOVal.ACC && Temp.IOVal.VCC;
	gSys.Var[IO_VAL] = Temp.Val;
	DBG("IO %u %u %u", Temp.IOVal.VCC, Temp.IOVal.ACC, Temp.IOVal.VACC);
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
	DBG("%u", A);
	gSys.Var[GSENSOR_VAL] = A;
	gSys.Var[GSENSOR_ALARM_VAL] = (gSys.Var[GSENSOR_ALARM_VAL] < A)?A:gSys.Var[GSENSOR_ALARM_VAL];
	gSys.Var[GSENSOR_MONITOR_VAL] = (gSys.Var[GSENSOR_MONITOR_VAL] < A)?A:gSys.Var[GSENSOR_MONITOR_VAL];
	gSys.Var[GSENSOR_KEEP_VAL] = (gSys.Var[GSENSOR_KEEP_VAL] < A)?A:gSys.Var[GSENSOR_KEEP_VAL];
	if (!LastA && !A)
	{
		DBG("stop check!");
		OS_StopTimer(gSys.TaskID[MAIN_TASK_ID], DETECT_TIMER_ID);
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
				DETECT_TIMER_ID,
				COS_TIMER_MODE_PERIODIC,
				SYS_TICK/16);
		gSys.Var[GSENSOR_VAL] = SensorCtrl.CrashCnt;
	}
}

void Detect_Config(void)
{
	HAL_GPIO_CFG_T DetectIrqCfg;
	IO_ValueUnion Temp;
	memset(&SensorCtrl, 0, sizeof(SensorCtrl));
	SensorCtrl.GSensorState = SENSOR_READ_FIRST;
	SensorCtrl.Param = gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param;
	SensorCtrl.CrashDetectOff = 1;
	Detect_GSensorDown();
#if (__CUST_CODE__ == __CUST_KQ__)
	SYS_Error(SENSOR_ERROR, 0);
	Temp.IOVal.ACC = 1;
	Temp.IOVal.VCC = 1;
	Temp.IOVal.VACC = 1;
	gSys.Var[IO_VAL] = Temp.Val;
#else
#if (__CUST_CODE__ == __CUST_LY_IOTDEV__)
	Temp.IOVal.VCC = 1;
#else
	Temp.IOVal.VCC = GPIO_Read(VCC_DET_PIN);
#endif
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

#if (__CUST_CODE__ == __CUST_LY_IOTDEV__)
	DetectIrqCfg.irqHandler = Detect_CrashIrqHandle;
	OS_GPIOInit(PinParam[CRASH_DET_PIN].APO.gpioId, &DetectIrqCfg);
#else
	DetectIrqCfg.irqHandler = Detect_VACCIrqHandle;
	OS_GPIOInit(PinParam[VCC_DET_PIN].APO.gpioId, &DetectIrqCfg);
#endif
	DetectIrqCfg.irqHandler = Detect_VACCIrqHandle;
	OS_GPIOInit(PinParam[ACC_DET_PIN].APO.gpioId, &DetectIrqCfg);

	hal_AnaGpadcOpen(HAL_ANA_GPADC_CHAN_0, HAL_ANA_GPADC_ATP_2S);

}

