#include "user.h"
#define AD_VREF_VAL (2000)
#define TEMPTURE_R	(20000)
#define VBAT_AMP	(100)
#define AD_MAX		(1023)
extern void MXC622X_ReadFirst(Sensor_CtrlStruct *Sensor);
extern void MXC622X_Read(Sensor_CtrlStruct *Sensor);
extern void LIS3DH_ReadFirst(Sensor_CtrlStruct *Sensor);
extern void LIS3DH_Read(Sensor_CtrlStruct *Sensor);
Sensor_CtrlStruct __attribute__((section (".usr_ram"))) SensorCtrl;
extern const GPIO_ParamStruct PinParam[PIN_MAX];

int32_t Detect_CalTempture(uint32_t R)
{
	double Temp;
	int32_t Result;
	Temp = 762.9 * pow(R, -0.1232) - 159.9;
	Result = Temp * 10;
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
	GPIO_Write(I2C_SCL_PIN, 1);
	GPIO_Write(I2C_SCL_PIN, 1);
	GPIO_Write(I2C_SDA_PIN, 1);
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
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
		if (SensorCtrl.ResetCnt > 2)
		{
			Detect_GSensorUp();
			SensorCtrl.GSensorState = SENSOR_BREAK;
			break;
		}
		SensorCtrl.ResetCnt++;
#endif
		Detect_GSensorUp();
		SensorCtrl.GSensorState = SENSOR_READ_FIRST;
		break;
	case SENSOR_BREAK:
		break;
	default:
		Detect_GSensorDown();
		break;
	}
}

void Detect_ADC0Cal(void)
{
	uint16_t ADCVal = hal_AnaGpadcGetRaw(HAL_ANA_GPADC_CHAN_0);
#if (__CUST_CODE__ == __CUST_LY_IOTDEV__)
	IO_ValueUnion Temp;
	uint32_t R;
	int32_t T;

	if (ADCVal != 0xFFFF)
	{
		gSys.Var[ADC0_VAL] = ADCVal;
	}

	switch (SensorCtrl.ADCChannel)
	{
	case LY_IOT_ADC_CH_BAT_TEMP:
		R = (SensorCtrl.Vref  * TEMPTURE_R / gSys.Var[ADC0_VAL]) * AD_MAX / AD_VREF_VAL - TEMPTURE_R;
		T = Detect_CalTempture(R);
		//DBG("OUT %u %u %d", gSys.Var[ADC0_VAL], R, T);
		if (!SensorCtrl.BattryTempture)
		{
			SensorCtrl.BattryTempture = T;
		}
		else
		{
			SensorCtrl.BattryTempture = (SensorCtrl.BattryTempture * 1 + T * 9)/10;
		}
		GPIO_Write(ADC_SELECT_0_PIN, 1);
		GPIO_Write(ADC_SELECT_1_PIN, 0);
		SensorCtrl.ADCChannel = LY_IOT_ADC_CH_ENV_TEMP;
		break;
	case LY_IOT_ADC_CH_ENV_TEMP:
		R = (SensorCtrl.Vref  * TEMPTURE_R / gSys.Var[ADC0_VAL]) * AD_MAX / AD_VREF_VAL - TEMPTURE_R ;
		T = Detect_CalTempture(R);
		if (!SensorCtrl.EnvTempture)
		{
			SensorCtrl.EnvTempture = T;
		}
		else
		{
			SensorCtrl.EnvTempture = (SensorCtrl.EnvTempture * 1 + T * 9)/10;
		}
		//DBG("IN %u %u %d", gSys.Var[ADC0_VAL], R, T);
		//DBG("%u %u", gSys.Var[ADC0_VAL], LY->Vol);
		GPIO_Write(ADC_SELECT_0_PIN, 0);
		GPIO_Write(ADC_SELECT_1_PIN, 1);
		SensorCtrl.ADCChannel = LY_IOT_ADC_CH_BAT_VOL;
		break;
	case LY_IOT_ADC_CH_BAT_VOL:
		R = gSys.Var[ADC0_VAL] * AD_VREF_VAL * VBAT_AMP / AD_MAX / 10 ;
		if (!SensorCtrl.Vol)
		{
			SensorCtrl.Vol = R;
		}
		else
		{
			SensorCtrl.Vol = (SensorCtrl.Vol * 5 + R * 5)/10;
		}
		//DBG("%u %u", gSys.Var[ADC0_VAL], LY->Vol);
		Temp.IOVal.VCC = (SensorCtrl.Vol > 100)?1:0;
		Temp.IOVal.ACC = GPIO_Read(ACC_DET_PIN);
		Temp.IOVal.VACC = Temp.IOVal.ACC && Temp.IOVal.VCC;
		gSys.Var[IO_VAL] = Temp.Val;
		GPIO_Write(ADC_SELECT_0_PIN, 1);
		GPIO_Write(ADC_SELECT_1_PIN, 1);
		SensorCtrl.ADCChannel = LY_IOT_ADC_CH_UNUSE;
		break;
	default:
		SensorCtrl.Vref = 2980;
		GPIO_Write(ADC_SELECT_0_PIN, 0);
		GPIO_Write(ADC_SELECT_1_PIN, 0);
		SensorCtrl.ADCChannel = LY_IOT_ADC_CH_BAT_TEMP;
		break;
	}
#endif

}

#ifdef __IRQ_CB_WITH_PARAM__
void Detect_VCCIrqHandle(UINT32 Param)
#else
void Detect_VCCIrqHandle(void)
#endif
{
	DBG("!");
	Detect_VACCIrqHandle();
}

#ifdef __IRQ_CB_WITH_PARAM__
void Detect_ACCIrqHandle(UINT32 Param)
#else
void Detect_ACCIrqHandle(void)
#endif
{
	//DBG("!");
	Detect_VACCIrqHandle();
}


void Detect_UserIrqHandle(void)
{
#if (__CUST_CODE__ == __CUST_LB__ || __CUST_CODE__ == __CUST_LB_V2__)
	DBG("! %d", GPIO_Read(USER_IO_PIN));
#endif
}

void Detect_VACCIrqHandle(void)
{
	IO_ValueUnion Temp;
#if (__CUST_CODE__ == __CUST_LY_IOTDEV__)


	Temp.IOVal.VCC = (SensorCtrl.Vol)?1:0;

#else
	Temp.IOVal.VCC = GPIO_Read(VCC_DET_PIN);
#endif
	Temp.IOVal.ACC = GPIO_Read(ACC_DET_PIN);
	Temp.IOVal.VACC = Temp.IOVal.ACC && Temp.IOVal.VCC;
	if (gSys.Var[IO_VAL] != Temp.Val)
	{
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

}

void Detect_CrashCal(void)
{
	uint32_t A = SensorCtrl.CrashCnt;
	uint32_t LastA = gSys.Var[GSENSOR_VAL];
	SensorCtrl.CrashCnt = 0;

	gSys.Var[GSENSOR_VAL] = A * 100 + 1;
	if (gSys.Var[GSENSOR_VAL] > 100)
	{
		DBG("%d", gSys.Var[GSENSOR_VAL]);
	}
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
				SYS_TICK/8);
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
#ifdef __G_SENSOR_ENABLE__
#else
	SYS_Error(SENSOR_ERROR, 0);
#endif

#if (__CUST_CODE__ == __CUST_KQ__)
	Temp.IOVal.ACC = 1;
	Temp.IOVal.VCC = 1;
	Temp.IOVal.VACC = 1;
	gSys.Var[IO_VAL] = Temp.Val;

	return ;
#else
#if (__CUST_CODE__ == __CUST_LY_IOTDEV__)
	Temp.IOVal.VCC = 1;
#else
	Temp.IOVal.VCC = GPIO_Read(VCC_DET_PIN);
#endif
	Temp.IOVal.ACC = GPIO_Read(ACC_DET_PIN);
	Temp.IOVal.VACC = Temp.IOVal.ACC && Temp.IOVal.VCC;
	gSys.Var[IO_VAL] = Temp.Val;
	DBG("IO %u %u %u", Temp.IOVal.VCC, Temp.IOVal.ACC, Temp.IOVal.VACC);
#endif

	DetectIrqCfg.direction = HAL_GPIO_DIRECTION_INPUT;
	DetectIrqCfg.irqMask.debounce = TRUE;
	DetectIrqCfg.irqMask.level = FALSE;
	DetectIrqCfg.irqMask.falling = TRUE;
	DetectIrqCfg.irqMask.rising = TRUE;



#ifdef __CRASH_ENABLE__
	DetectIrqCfg.irqHandler = Detect_CrashIrqHandle;
	OS_GPIOInit(PinParam[CRASH_DET_PIN].APO.gpioId, &DetectIrqCfg);
#endif

#ifdef __IO_POLL_CHECK__

#else
	if (PinParam[VCC_DET_PIN].IsWork)
	{
		DetectIrqCfg.irqHandler = Detect_VCCIrqHandle;
		OS_GPIOInit(PinParam[VCC_DET_PIN].APO.gpioId, &DetectIrqCfg);
	}

	if (PinParam[ACC_DET_PIN].IsWork)
	{
		DetectIrqCfg.irqHandler = Detect_ACCIrqHandle;
		OS_GPIOInit(PinParam[ACC_DET_PIN].APO.gpioId, &DetectIrqCfg);
	}
#endif

#ifdef __AD_ENABLE__
	hal_AnaGpadcOpen(HAL_ANA_GPADC_CHAN_0, HAL_ANA_GPADC_ATP_2S);
#endif
#if (__CUST_CODE__ == __CUST_LY_IOTDEV__)
	GPIO_Write(ADC_SELECT_0_PIN, 0);
	GPIO_Write(ADC_SELECT_1_PIN, 1);
	SensorCtrl.ADCChannel = LY_IOT_ADC_CH_BAT_VOL;
#endif

#if (__CUST_CODE__ == __CUST_LB__ || __CUST_CODE__ == __CUST_LB_V2__)
	DetectIrqCfg.irqHandler = Detect_UserIrqHandle;
	OS_GPIOInit(PinParam[USER_IO_PIN].APO.gpioId, &DetectIrqCfg);
#endif
}

