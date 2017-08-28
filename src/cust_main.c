#include "user.h"

extern PVOID   g_CosBaseAdd;
extern PVOID   g_CswBaseAdd;
extern UINT32  g_CosHeapSize;
extern UINT32  g_CswHeapSize;
extern PUBLIC BOOL HAL_BOOT_FUNC_INTERNAL hal_HstSendEvent(UINT32 ch);
extern void GL_Config(void);
SysVar_Struct __attribute__((section (".cache_ram"))) gSys;
extern Sensor_CtrlStruct __attribute__((section (".usr_ram"))) SensorCtrl;

#ifdef __ANT_TEST__
Monitor_CtrlStruct __attribute__((section (".usr_ram"))) DummyCtrl;
#endif
void SYS_PrintInfo(void);


void Main_GetRTC(void)
{
	HAL_TIM_RTC_TIME_T RTC;
	Date_Union uDate;
	Time_Union uTime;
	if (HAL_ERR_NO != hal_TimRtcGetTime(&RTC))
	{
		DBG("!");
	}
	else
	{
		uDate.Date.Year = RTC.year + 2000;
		uDate.Date.Mon = RTC.month;
		uDate.Date.Day = RTC.day;
		uTime.Time.Hour = RTC.hour;
		uTime.Time.Min = RTC.min;
		uTime.Time.Sec = RTC.sec;
		gSys.Var[UTC_DATE] = uDate.dwDate;
		gSys.Var[UTC_TIME] = uTime.dwTime;
		if ((gSys.Var[SYS_TIME] == 1) && gSys.uDateSave.dwDate)
		{
			SYS_CheckTime(&gSys.uDateSave.Date, &gSys.uTimeSave.Time);
		}
		gSys.uDateSave = uDate;
		gSys.uTimeSave = uTime;
//		DBG("%u %u %u %u:%u:%u", uDate.Date.Year, uDate.Date.Mon, uDate.Date.Day,
//				uTime.Time.Hour, uTime.Time.Min, uTime.Time.Sec);
	}
}

void Main_StateBot(void)
{
	OS_SendEvent(gSys.TaskID[MAIN_TASK_ID], EV_MMI_GET_RTC_ENABLE, 0, 0, 0);

}

void Main_Task(void *pData)
{
	COS_EVENT Event = { 0 };
	CFW_EVENT CFWEvent;
	uint32_t *Param = gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param;
	uint8_t *TempBuf;
	uint8_t LedType;
	DBG("Task start! %u %u %u %u %u %u", Param[PARAM_DETECT_PERIOD], Param[PARAM_STOP_VBAT], Param[PARAM_LOW_VBAT],
			Param[PARAM_NORMAL_VBAT], Param[PARAM_SMS_ALARM], Param[PARAM_CALL_AUTO_GET]);

    memset(&Event, 0, sizeof(COS_EVENT));

#if (defined(__G_SENSOR_ENABLE__) || defined(__AD_ENABLE__) || defined(__IO_POLL_CHECK__))
    if (Param[PARAM_DETECT_PERIOD])
    {
		OS_StartTimer(gSys.TaskID[MAIN_TASK_ID],
				G_SENSOR_TIMER_ID,
				COS_TIMER_MODE_PERIODIC,
				SYS_TICK/Param[PARAM_DETECT_PERIOD]);
    }
#endif

    while(1)
    {
        if(Event.nParam1)
        {
            if(EV_CFW_SIM_READ_RECORD_RSP == Event.nEventId )
            {
                hal_HstSendEvent(0xfa022212);
                hal_HstSendEvent(Event.nParam1);
                if ((((UINT32)(UINT32 *)Event.nParam1) >= (UINT32)g_CswBaseAdd)
                && (((UINT32)(UINT32 *)Event.nParam1) < (UINT32)(g_CswBaseAdd + g_CswHeapSize)))
                {
                    UINT8* pPrm= (UINT8 *)Event.nParam1;
                    memset(pPrm+Event.nParam2, 0x0f, 1);
                }
            }
            if ((Event.nParam1 & 0xff000000) == 0x82000000)
            {
            	DBG("free event %u mem %08x",Event.nEventId, Event.nParam1);
                COS_FREE((VOID *)Event.nParam1); // Clear the memory of the Event.nParam1, then it will do the getting value...
                Event.nParam1 = NULL;
            }

        }

        COS_WaitEvent(gSys.TaskID[MAIN_TASK_ID], &Event, COS_WAIT_FOREVER);

		switch (Event.nEventId)
		{

		case EV_TIMER:
			if ( (Event.nParam1 >= LED_TIMER_ID) && ((Event.nParam1 - LED_TIMER_ID) < LED_TYPE_MAX) )
			{
				LedType = Event.nParam1 - LED_TIMER_ID;
				gSys.State[LED_STATE + LedType] = !gSys.State[LED_STATE + LedType];
				GPIO_Write(LED_NET_PIN + LedType, gSys.State[LED_STATE + LedType]);
			}
			else
			{
				switch (Event.nParam1)
				{
				case TRACE_TIMER_ID:
					gSys.State[TRACE_STATE] = 0;
					__SetDeepSleep(1);
					break;
				case G_SENSOR_TIMER_ID:

#ifdef __G_SENSOR_ENABLE__
					Detect_GSensorBot();
#endif

#ifdef __AD_ENABLE__
					Detect_ADC0Cal();
#endif
#ifdef __IO_POLL_CHECK__
					Detect_VACCIrqHandle();
#endif
					break;
				case DETECT_TIMER_ID:
					Detect_CrashCal();
					break;
				default:
					DBG("%d");
					OS_StopTimer(gSys.TaskID[MAIN_TASK_ID], Event.nParam1);
					break;
				}
			}
			break;
		case EV_MMI_GET_RTC_ENABLE:
			gSys.Var[SYS_TIME]++;
			Main_GetRTC();
			SYS_PowerStateBot();
			//gSys.Var[MAIN_FREQ] = hal_SysGetFreq();
#ifdef __NO_GPS__
#else
			GPS_StateCheck();
#endif
#ifdef __ANT_TEST__
#else
			Monitor_StateCheck();
			Alarm_StateCheck();
			GPRS_MonitorTask();
#endif
			COM_StateCheck();
#if (__CUST_CODE__ == __CUST_LY_IOTDEV__)
			if (!(gSys.Var[SYS_TIME] % 3))
				DBG("BAT %d ENV %d VBAT %u", SensorCtrl.BattryTempture, SensorCtrl.EnvTempture, SensorCtrl.Vol);
#endif
			if (PRINT_TEST == gSys.State[PRINT_STATE])
			{
				//LV协议输出
				TempBuf = COS_MALLOC(1024);
				if (TempBuf)
				{
					LV_Print(TempBuf);
					OS_SendEvent(gSys.TaskID[COM_TASK_ID], EV_MMI_COM_TX_REQ, 0, 0, 0);
					COS_FREE(TempBuf);
				}
			}
#if (__CUST_CODE__ == __CUST_KQ__)

#elif defined __MINI_SYSTEM__

#else
//#if (__CUST_CODE__ == __CUST_LB_V2__)
//			GPIO_Write(VCC_DET_PIN, gSys.State[WDG_STATE]);
//#endif
			GPIO_Write(WDG_PIN, gSys.State[WDG_STATE]);
			gSys.State[WDG_STATE] = !gSys.State[WDG_STATE];

#endif
			break;
		case EV_MMI_REBOOT:
			sxr_Sleep(SYS_TICK/4);
			DM_Reset();
			break;
		case EV_DM_POWER_ON_IND:
			CFW_ShellControl(CFW_CONTROL_CMD_POWER_ON);

			break;
		case EV_TIM_SET_TIME_IND:
			hal_TimRtcIrqSetMask(FALSE);
			hal_TimRtcSetIrqIntervalMode(HAL_TIM_RTC_INT_PER_SEC);
			hal_TimRtcIrqSetHandler(Main_StateBot);
			hal_TimRtcIrqSetMask(TRUE);
			break;
		case EV_PM_BC_IND:
			break;
		case EV_DM_SPEECH_IND:
			switch(Event.nParam1)
			{
			case 0:
				DBG("speech off");
				break;
			case 1:
				DBG("speech on");
				break;
			}
			break;
		default:
			CFWEvent.nEventId = Event.nEventId;
			CFWEvent.nParam1 = Event.nParam1;
			CFWEvent.nParam2 = Event.nParam2;
			CFWEvent.nUTI = HIUINT16(Event.nParam3);
			CFWEvent.nType = HIUINT8(Event.nParam3);
			CFWEvent.nFlag = LOUINT8(Event.nParam3);
			GPRS_EventAnalyze(&CFWEvent);
			break;
		}

    }
}

void ANT_TestTask(void *pData)
{
	COS_EVENT Event = { 0 };
	while (1)
	{
		COS_WaitEvent(gSys.TaskID[MONITOR_TASK_ID], &Event, COS_WAIT_FOREVER);
	}
}

void __MainInit(void)
{
//	uint8_t *Temp = (uint8_t *)pal_GetImei(CFW_SIM_0);
//	//OS_GetIMEI(Temp);
//	ReverseBCD(Temp, gSys.IMEI, IMEI_LEN);
//	gSys.IMEI[0] &= 0x0f;
	uint8_t Buf[0x28];
	uint32_t Addr = 0x003FE000;
#ifdef __PLATFORM_8955__


	__ReadFlash(Addr, Buf, 0x28);
	ReverseBCD(Buf + 4, gSys.IMEI, IMEI_LEN);
	gSys.IMEI[0] &= 0x0f;
#else
	uint8_t Temp[IMEI_LEN];
	OS_GetIMEI(Temp);
	ReverseBCD(Temp, gSys.IMEI, IMEI_LEN);
	gSys.IMEI[0] &= 0x0f;
#endif



	InitRBuffer(&gSys.TraceBuf, gSys.TraceData, sizeof(gSys.TraceData), 1);

	//DBG("%02x", XorCheck("CFGCLR,hFF", strlen("CFGCLR,hFF"), 0));
	//加入初始化参数代码
	memset(gSys.TaskID, 0, sizeof(HANDLE) * TASK_ID_MAX);
	gSys.TaskID[MAIN_TASK_ID] = COS_CreateTask(Main_Task, NULL,
			NULL, MMI_TASK_MAX_STACK_SIZE, MMI_TASK_PRIORITY + MAIN_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI Main Task");
	__SetDefaultTaskHandle(gSys.TaskID[MAIN_TASK_ID]);
	DBG("new start %u", OS_GetResetReason());
	Param_Config();
	SYS_PrintInfo();
	GPIO_Config();

	gSys.Var[SYS_TIME] = 0;
	GPRS_Config();
	Monitor_InitCache();
#ifdef __ANT_TEST__

	gSys.TaskID[MONITOR_TASK_ID] = COS_CreateTask(ANT_TestTask, NULL,
				NULL, MMI_TASK_MIN_STACK_SIZE, MMI_TASK_PRIORITY + MONITOR_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI ANT Task");
	DummyCtrl.Param = gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param;
	DummyCtrl.Param[PARAM_UPLOAD_RUN_PERIOD] = 43200;
#else
	//使用哪个监控协议，就初始化哪个平台
#if (__CUST_CODE__ == __CUST_KQ__)
	KQ_Config();
#elif (__CUST_CODE__ == __CUST_LY__ || __CUST_CODE__ == __CUST_LY_IOTDEV__)
	LY_Config();
#elif (__CUST_CODE__ == __CUST_LB__ || __CUST_CODE__ == __CUST_LB_V2__)
	LB_Config();
#elif (__CUST_CODE__ == __CUST_GLEAD__ || __CUST_CODE__ == __CUST_NONE__)
	GL_Config();
#else
	gSys.TaskID[MONITOR_TASK_ID] = COS_CreateTask(ANT_TestTask, NULL,
				NULL, MMI_TASK_MIN_STACK_SIZE, MMI_TASK_PRIORITY + MONITOR_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI ANT Task");
	DummyCtrl.Param = gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param;
	gSys.Monitor = &DummyCtrl;
	DummyCtrl.Param[PARAM_UPLOAD_RUN_PERIOD] = 999999;
#endif
	Monitor_Wakeup();

#endif
	FTP_Config();
	Uart_Config();
	SMS_Config();
#ifdef __NO_GPS__
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	hwp_iomux->pad_GPIO_4_cfg = IOMUX_PAD_GPIO_4_SEL_FUN_GPIO_4_SEL;
	hwp_iomux->pad_GPIO_5_cfg = IOMUX_PAD_GPIO_5_SEL_FUN_GPIO_5_SEL;
#endif

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_configRegs->Alt_mux_select &= ~CFG_REGS_UART2_UART2;
	hwp_configRegs->GPIO_Mode |= (1 << 8)|(1 << 13);
#endif
#else
	GPS_Config();
#endif
	Alarm_Config();
	User_Config();
#ifdef __ANT_TEST__
#else
	Remote_Config();
	NTP_Config();
	LUAT_Config();
#endif
	SYS_PowerStateBot();
	Detect_Config();
	strcpy(Buf, "测试123");
	HexTrace(Buf, strlen(Buf));
	Addr = OS_GB2312ToUCS2(Buf, Buf, strlen(Buf), 0);
	HexTrace(Buf, Addr);
	Addr = OS_UCS2ToGB2312(Buf, Buf, Addr, 1);
	HexTrace(Buf, Addr);
}

void SYS_PowerStateBot(void)
{
	if (gSys.Var[VBAT])
	{
		gSys.Var[VBAT] = (gSys.Var[VBAT] * 9 + OS_GetVbatADC() * 1) / 10;
	}
	else
	{
		gSys.Var[VBAT] = OS_GetVbatADC();
	}

	switch (gSys.State[SYSTEM_STATE])
	{
	case SYSTEM_POWER_STOP:
		if (gSys.Var[VBAT] >= gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_NORMAL_VBAT])
		{
			gSys.State[SYSTEM_STATE] = SYSTEM_POWER_ON;
			DBG("system recovery %u", gSys.Var[VBAT]);
		}
		break;
	case SYSTEM_POWER_LOW:
		if (gSys.Var[VBAT] >= gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_NORMAL_VBAT])
		{
			gSys.State[SYSTEM_STATE] = SYSTEM_POWER_ON;
			DBG("system recovery %u", gSys.Var[VBAT]);
		}
		else if (gSys.Var[VBAT] <= gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_STOP_VBAT])
		{
			gSys.State[SYSTEM_STATE] = SYSTEM_POWER_STOP;
			DBG("system down %u", gSys.Var[VBAT]);
			Monitor_RecordAlarm(ALARM_TYPE_NOPOWER, 0, 0);
		}
		break;
	case SYSTEM_POWER_ON:
		if (gSys.Var[VBAT] <= gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_STOP_VBAT])
		{
			gSys.State[SYSTEM_STATE] = SYSTEM_POWER_STOP;
			DBG("system down %u", gSys.Var[VBAT]);
		}
		else if (gSys.Var[VBAT] <= gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_LOW_VBAT])
		{
			gSys.State[SYSTEM_STATE] = SYSTEM_POWER_LOW;
			DBG("system low %u", gSys.Var[VBAT]);
			Monitor_RecordAlarm(ALARM_TYPE_LOWPOWER, 0, 0);
		}
		break;
	}
}

void SYS_Error(uint8_t Sn, uint8_t Val)
{
	if (Sn < ERROR_MAX)
	{
		if (gSys.Error[Sn] != Val)
		{
			DBG("Error %u %u->%u", Sn, gSys.Error[Sn], Val);
			gSys.Error[Sn] = Val;
			gSys.ErrorCRC32 = __CRC32(gSys.Error, ERROR_MAX, CRC32_START);
		}
	}
}

void SYS_Reset(void)
{
	if (gSys.RMCInfo->LocatStatus)
	{
		Param_Save(PARAM_TYPE_LOCAT);
	}
	OS_SendEvent(gSys.TaskID[MAIN_TASK_ID], EV_MMI_REBOOT, 0, 0, 0);
	DBG("!!!");
}

void SYS_PrintInfo(void)
{
	Date_Union uDate;
	Time_Union uTime;
	uint64_t Tamp;
	int8_t Buf[4][6];
//	char Month[5];
	char *strMon[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	int Day,Year,Mon,Hour,Min,Sec,i;
	CmdParam CP;
	memset(&CP, 0, sizeof(CP));
	memset(Buf, 0, sizeof(Buf));
	CP.param_max_len = 6;
	CP.param_max_num = 4;
	CP.param_str = (int8_t *)Buf;
	CmdParseParam(__DATE__, &CP, ' ');
	Mon = 0;
	for (i = 0; i < 12; i++)
	{
		if (!strcmp(strMon[i], Buf[0]))
		{
			Mon = i + 1;
		}
	}

	if (Buf[1][0])
	{
		Day = strtol(Buf[1], NULL, 10);
		Year = strtol(Buf[2], NULL, 10);
	}
	else
	{
		Day = strtol(Buf[2], NULL, 10);
		Year = strtol(Buf[3], NULL, 10);
	}


	CP.param_num = 0;
	memset(Buf, 0, sizeof(Buf));

	CP.param_str = (int8_t *)Buf;
	CmdParseParam(__TIME__, &CP, ':');
	Hour = strtol(Buf[0], NULL, 10);
	Min = strtol(Buf[1], NULL, 10);
	Sec = strtol(Buf[2], NULL, 10);

	//hwp_configRegs->GPIO_Mode |= (1 << 4)|(1 << 8)|(1 << 13);

	gSys.Var[MAIN_FREQ] = hal_SysGetFreq();
	gSys.Var[SOFTWARE_VERSION] = ( (Year - 2000) * 12 + Mon) * 1000000 + Day * 10000 + Hour * 100 + Min;
	DBG("Version %u Build in %u-%u-%u %u:%u:%u", gSys.Var[SOFTWARE_VERSION], Year, Mon, Day, Hour, Min, Sec);
	if (!gSys.Var[UTC_DATE])
	{

		uDate.Date.Year = Year;
		uDate.Date.Mon = Mon;
		uDate.Date.Day = Day;
		uTime.Time.Hour = Hour;
		uTime.Time.Min = Min;
		uTime.Time.Sec = Sec;
		Tamp = UTC2Tamp(&uDate.Date, &uTime.Time);
		Tamp2UTC(Tamp - 28800, &uDate.Date, &uTime.Time, 0);
		gSys.Var[UTC_DATE] = uDate.dwDate;
		gSys.Var[UTC_TIME] = uTime.dwTime;
	}

}

void SYS_CheckTime(Date_UserDataStruct *Date, Time_UserDataStruct *Time)
{
	HAL_TIM_RTC_TIME_T RTC;
	Date_Union uDate;
	Time_Union uTime;
	uint64_t NewTamp = 0;
	uint64_t SysTamp = 0;
	uDate.dwDate = gSys.Var[UTC_DATE];
	uTime.dwTime = gSys.Var[UTC_TIME];
	NewTamp = UTC2Tamp(Date, Time);
	SysTamp = UTC2Tamp(&uDate.Date, &uTime.Time);
	if (NewTamp > (SysTamp + 1))
	{
		DBG("%u %u", (uint32_t)NewTamp, (uint32_t)SysTamp);
		RTC.year = Date->Year - 2000;
		RTC.month = Date->Mon;
		RTC.day = Date->Day;
		RTC.hour = Time->Hour;
		RTC.min = Time->Min;
		RTC.sec = Time->Sec;
		hal_TimRtcSetTime(&RTC);
	}
}

void SYS_Wakeup(void)
{

    if (!gSys.State[TRACE_STATE])
    {
    	__SetDeepSleep(0);
    	gSys.State[TRACE_STATE] = 1;
    }
    OS_StartTimer(gSys.TaskID[MAIN_TASK_ID], TRACE_TIMER_ID, COS_TIMER_MODE_SINGLE, SYS_TICK/16);
}

void SYS_Debug(const ascii *Fmt, ...)
{
    int8_t uart_buf[512];
    int32_t Len;
    va_list ap;
    SYS_Wakeup();
    va_start (ap, Fmt);
    Len = vsnprintf(uart_buf, sizeof(uart_buf), Fmt, ap);
    va_end (ap);
    sxs_fprintf(_MMI | TNB_ARG(0) | TSTDOUT, uart_buf);

    WriteRBufferForce(&gSys.TraceBuf, uart_buf, Len);
    WriteRBufferForce(&gSys.TraceBuf, "\r\n", 2);
}

void HexTrace(uint8_t *Data, uint32_t Len)
{
    char *uart_buf = COS_MALLOC(Len * 3 + 2);
    uint32_t i,j, Temp;
    j = 0;
    if (!uart_buf)
    	return;
    for (i = 0; i < Len; i++)
    {
    	Temp = Data[i] >> 4;
    	if (Temp < 10 )
    	{
    		uart_buf[j++] = Temp + '0';
    	}
    	else
    	{
    		uart_buf[j++] = Temp + 'A' - 10;
    	}
    	Temp = Data[i] & 0x0f;
    	if (Temp < 10 )
    	{
    		uart_buf[j++] = Temp + '0';
    	}
    	else
    	{
    		uart_buf[j++] = Temp + 'A' - 10;
    	}
    	uart_buf[j++] = ' ';
    }
    uart_buf[j++] = 0;
    SYS_Wakeup();
    sxs_fprintf(_MMI | TNB_ARG(0) | TSTDOUT, uart_buf);

    WriteRBufferForce(&gSys.TraceBuf, uart_buf, strlen(uart_buf));
    WriteRBufferForce(&gSys.TraceBuf, "\r\n", 2);
    COS_FREE(uart_buf);
}

void DecTrace(uint8_t *Data, uint8_t Len)
{
	char *uart_buf = COS_MALLOC(Len * 4 + 2);
    uint8_t i,j, Temp;
    j = 0;
    if (!uart_buf)
    	return ;
    for (i = 0; i < Len; i++)
    {
    	Temp = Data[i] / 100;
    	uart_buf[j++] = Temp + '0';
    	Temp = (Data[i] % 100) / 10;
    	uart_buf[j++] = Temp + '0';
    	Temp = (Data[i] % 10);
    	uart_buf[j++] = Temp + '0';
    	uart_buf[j++] = ' ';
    }
    uart_buf[j++] = 0;
    SYS_Wakeup();
    sxs_fprintf(_MMI | TNB_ARG(0) | TSTDOUT, uart_buf);
    WriteRBufferForce(&gSys.TraceBuf, uart_buf, strlen(uart_buf));
    WriteRBufferForce(&gSys.TraceBuf, "\r\n", 2);
    COS_FREE(uart_buf);
}
