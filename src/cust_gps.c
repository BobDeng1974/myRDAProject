#include "user.h"
#define GPS_SECTOR_LEN_MAX 	(32)
#define GPS_UART hwp_uart2
#define GPS_UART_ID HAL_UART_2

typedef struct
{
	uint8_t AnalyzeBuf[GPS_LEN_MAX + 2];
	uint8_t RxState;
	uint8_t IsWork;
	uint8_t RxBuf[GPS_LEN_MAX];
	uint8_t TxBuf[32];
	uint8_t LocatTime;
	uint8_t RemotePrintTime;
	uint32_t NoDataTime;
	uint32_t NoLocatTime;
	uint32_t SleepTime;
	uint32_t KeepTime;
	uint32_t GPSVaildTime;			//当车辆处于静止状态时，不累计里程
	uint32_t RxPos;
	uint32_t AnalyzeLen;
	uint32_t *Param;
}GPS_CtrlStruct;

GPS_CtrlStruct __attribute__((section (".usr_ram"))) GPSCtrl;
enum
{
	GPS_NORMAL,
	GPS_ACTIVE_BY_VACC,
	GPS_ACTIVE_BY_GS,
};
const int8_t GNMode[GN_OTHER_MODE][2] =
{
		{'G','N'},
		{'G','P'},
		{'B','D'},
};
void GPS_IRQHandle(HAL_UART_IRQ_STATUS_T Status, HAL_UART_ERROR_STATUS_T Error);
int32_t GPS_RMCAnalyze(void *pData)
{
	uint8_t Buf[RMC_SECTOR_MAX][GPS_SECTOR_LEN_MAX];
	CmdParam CP;
	uint8_t i;
	double Speed;
	double Cog;
	uint32_t MileageM;
	RMC_InfoStruct RMC;

	if (GPSCtrl.RemotePrintTime < 180)
	{
		GPSCtrl.RemotePrintTime++;
		DBG("%s", (int8_t *)pData);
	}


	memset(&RMC, 0, sizeof(RMC_InfoStruct));
	memset(Buf, 0, sizeof(Buf));
	CP.param_str = (int8_t *)Buf;
	CP.param_max_num = RMC_SECTOR_MAX;
	CP.param_max_len = 32;
	CP.param_num = 0;
	CmdParseParam(pData, &CP, ',');



	RMC.LocatMode = GN_OTHER_MODE;
	for (i = 0; i < GN_OTHER_MODE; i++)
	{
		if (!memcmp(Buf[RMC_HEAD], GNMode[i], 2))
		{
			RMC.LocatMode = i;
			break;
		}
	}

	RMC.UTCTime.Hour = AsciiToU32(&Buf[RMC_TIME][0], 2);
	if (RMC.UTCTime.Hour > 23)
	{
		DBG("!");
		RMC.UTCTime.Hour = gSys.RMCInfo->UTCTime.Hour;
	}

	RMC.UTCTime.Min = AsciiToU32(&Buf[RMC_TIME][2], 2);
	if (RMC.UTCTime.Min > 59)
	{
		DBG("!");
		RMC.UTCTime.Min = gSys.RMCInfo->UTCTime.Min;
	}

	RMC.UTCTime.Sec = AsciiToU32(&Buf[RMC_TIME][4], 2);
	if (RMC.UTCTime.Sec > 59)
	{
		DBG("!");
		RMC.UTCTime.Sec = gSys.RMCInfo->UTCTime.Sec;
	}

	if (Buf[RMC_DATE][0])
	{
		RMC.UTCDate.Day = AsciiToU32(&Buf[RMC_DATE][0], 2);
		if (RMC.UTCDate.Day > 31)
		{
			DBG("!");
			RMC.UTCDate.Day = gSys.RMCInfo->UTCDate.Day;
		}
		RMC.UTCDate.Mon = AsciiToU32(&Buf[RMC_DATE][2], 2);
		if (RMC.UTCDate.Mon > 12)
		{
			DBG("!");
			RMC.UTCDate.Mon = gSys.RMCInfo->UTCDate.Mon;
		}
		RMC.UTCDate.Year = AsciiToU32(&Buf[RMC_DATE][4], 2);
		if (RMC.UTCDate.Year > 99)
		{
			DBG("!");
			RMC.UTCDate.Year = gSys.RMCInfo->UTCDate.Year;
		}
		RMC.UTCDate.Year += 2000;
	}
	else
	{
		RMC.UTCDate.Year = 2000;
		RMC.UTCDate.Mon = 1;
		RMC.UTCDate.Day = 1;
	}

	if (Buf[RMC_STATUS][0] == 'A' && (Buf[RMC_LOCAT_MODE][0] == 'A' || Buf[RMC_LOCAT_MODE][0] == 'D'))
	{
		RMC.LocatStatus = 1;
	}
	else
	{
		RMC.LocatStatus = 0;
	}

	Speed = AsciiToFloat(Buf[RMC_SPEED]);
	Cog = AsciiToFloat(Buf[RMC_COG]);

	if (Speed > 999)
	{
		Speed = 999;
	}

	if (Cog > 360.0)
	{
		Cog = 360.0;
	}

	if (Buf[RMC_NS][0] == 'N')
	{
		RMC.LatNS = 'N';
	}
	else if (Buf[RMC_NS][0] == 'S')
	{
		RMC.LatNS = 'S';
	}
	else
	{
		RMC.LatNS = gSys.RMCInfo->LatNS;
	}

	if (Buf[RMC_EW][0] == 'W')
	{
		RMC.LgtEW = 'W';
	}
	else if (Buf[RMC_EW][0] == 'E')
	{
		RMC.LgtEW = 'E';
	}
	else
	{
		RMC.LgtEW = gSys.RMCInfo->LgtEW;
	}

	if (IsDigit(Buf[RMC_LAT][0]))
	{
		RMC.LatDegree = AsciiToU32(&Buf[RMC_LAT][0], 2);
		if (RMC.LatDegree > 90)
		{
			DBG("!");
			RMC.LatDegree = gSys.RMCInfo->LatDegree;
			RMC.LatMin = gSys.RMCInfo->LatMin;
		}
		else
		{
			RMC.LatMin = AsciiToU32(&Buf[RMC_LAT][2], 2);
			RMC.LatMin = RMC.LatMin * 10000 + AsciiToU32(&Buf[RMC_LAT][5], 4);
			if (RMC.LatMin > 99999999)
			{
				DBG("!");
				RMC.LatDegree = gSys.RMCInfo->LatDegree;
				RMC.LatMin = gSys.RMCInfo->LatMin;
			}
		}
	}
	else
	{
		RMC.LatDegree = gSys.RMCInfo->LatDegree;
		RMC.LatMin = gSys.RMCInfo->LatMin;
	}

	if (IsDigit(Buf[RMC_LGT][0]))
	{
		RMC.LgtDegree = AsciiToU32(&Buf[RMC_LGT][0], 3);
		if (RMC.LgtDegree > 180)
		{
			DBG("!");
			RMC.LgtDegree = gSys.RMCInfo->LgtDegree;
			RMC.LgtMin = gSys.RMCInfo->LgtMin;
		}
		else
		{
			RMC.LgtMin = AsciiToU32(&Buf[RMC_LGT][3], 2);
			RMC.LgtMin = RMC.LgtMin * 10000 + AsciiToU32(&Buf[RMC_LGT][6], 4);
			if (RMC.LgtMin > 99999999)
			{
				DBG("!");
				RMC.LgtDegree = gSys.RMCInfo->LgtDegree;
				RMC.LgtMin = gSys.RMCInfo->LgtMin;
			}
		}
	}
	else
	{
		RMC.LgtDegree = gSys.RMCInfo->LgtDegree;
		RMC.LgtMin = gSys.RMCInfo->LgtMin;
	}

	RMC.Speed  = Speed * 1000;
	RMC.Cog = Cog * 1000;
	if (RMC.LocatStatus)
	{
		GPSCtrl.NoLocatTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_V_TO];
		GPSCtrl.LocatTime++;
	}
	else
	{
		GPSCtrl.LocatTime = 0;
	}
	memcpy(gSys.RMCInfo, &RMC, sizeof(RMC));
	if ( (gSys.State[GPS_STATE] == GPS_A_STAGE) && (GPSCtrl.GPSVaildTime > gSys.Var[SYS_TIME]))//当车辆处于行驶状态时，累计里程
	{
		MileageM = Speed * 1.852 / 3600;
		gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageM += MileageM;
		if (gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageM > 1000)
		{
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageKM += gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageM / 1000;
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageM = gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageM % 1000;
		}

#if (__CUST_CODE__ == __CUST_LY__)
		if (gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageKM > 99999)
		{
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageKM = 0;
		}
#else
		if (gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageKM > 999999)
		{
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageKM = 0;
		}
#endif
	}
	Locat_CacheSave();


//	DBG("%u %u-%u-%u %u:%u:%u %02x %u %c %u %u %c %u %u %u %u\r\n",
//			gSys.RMCPos, gSys.RMCInfo[gSys.RMCPos].UTCDate.Year,
//			gSys.RMCInfo[gSys.RMCPos].UTCDate.Mon, gSys.RMCInfo[gSys.RMCPos].UTCDate.Day,
//			gSys.RMCInfo[gSys.RMCPos].UTCTime.Hour, gSys.RMCInfo[gSys.RMCPos].UTCTime.Min,
//			gSys.RMCInfo[gSys.RMCPos].UTCTime.Sec, gSys.RMCInfo[gSys.RMCPos].LocatMode,
//			gSys.RMCInfo[gSys.RMCPos].LocatStatus, gSys.RMCInfo[gSys.RMCPos].LatNS,
//			gSys.RMCInfo[gSys.RMCPos].LatDegree, gSys.RMCInfo[gSys.RMCPos].LatMin,
//			gSys.RMCInfo[gSys.RMCPos].LgtEW, gSys.RMCInfo[gSys.RMCPos].LgtDegree,
//			gSys.RMCInfo[gSys.RMCPos].LgtMin, gSys.RMCInfo[gSys.RMCPos].Speed, gSys.RMCInfo[gSys.RMCPos].Cog);
//	memcpy(&RMC->OrgFormat[0], &Buf[RMC_LAT][0], 4);
//	memcpy(&RMC->OrgFormat[4], &Buf[RMC_LAT][5], 4);
//	memcpy(&RMC->OrgFormat[8], &Buf[RMC_LGT][0], 5);
//	memcpy(&RMC->OrgFormat[13], &Buf[RMC_LGT][6], 4);
//	RMC->OrgFormat[17]=0;
	return 0;
}

int32_t GPS_GSVAnalyze(void *pData)
{
	uint8_t Buf[GSV_SECTOR_MAX][GPS_SECTOR_LEN_MAX];
	CmdParam CP;
	GSV_InfoStruct *GSV = &gSys.GSVInfo;
	uint8_t IsBD, Temp, i;
	memset(Buf, 0, sizeof(Buf));

	if (GPSCtrl.RemotePrintTime < 180)
	{
		DBG("%s", (int8_t *)pData);
	}

	CP.param_str = (int8_t *)Buf;
	CP.param_max_num = GSV_SECTOR_MAX;
	CP.param_max_len = 32;
	CP.param_num = 0;
	CmdParseParam(pData, &CP, ',');
	if (Buf[GSV_HEAD][0] == 'B')
	{
		IsBD = 1;
	}
	else
	{
		IsBD = 0;
	}

	if (!IsBD && (Buf[GSV_MSG_NO][0] == '1'))
	{
//		DecTrace(&gSys.GSVInfo.PRN[0][0], gSys.GSVInfo.Pos[0]);
//		DecTrace(&gSys.GSVInfo.CN[0][0], gSys.GSVInfo.Pos[0]);
//		DecTrace(&gSys.GSVInfo.PRN[1][0], gSys.GSVInfo.Pos[1]);
//		DecTrace(&gSys.GSVInfo.CN[1][0], gSys.GSVInfo.Pos[1]);
		gSys.GSVInfoSave = gSys.GSVInfo;
		GSV->Pos[0] = 0;
		GSV->Pos[1] = 0;
	}

	for (i = 0;i < 4; i++)
	{
		Temp = AsciiToU32(&Buf[GSV_PRN1 + i * 4][0], 3);
		if (Temp)
		{
			GSV->PRN[IsBD][GSV->Pos[IsBD]] = Temp;
			Temp = AsciiToU32(&Buf[GSV_CN1 + i * 4][0], 2);
			if (Temp)
			{
				GSV->CN[IsBD][GSV->Pos[IsBD]] = Temp;
				GSV->Pos[IsBD]++;
			}

		}
		else
		{
			break;
		}
	}
	return 0;
}

const StrFunStruct GPSStrFun[] =
{
	{
		"RMC",
		GPS_RMCAnalyze,
	},
	{
		"GSV",
		GPS_GSVAnalyze,
	},
};

void GPS_SendCmd(uint32_t Len)
{
	OS_UartDMASend(HAL_IFC_UART2_TX, GPSCtrl.TxBuf, Len);
}

void GPS_Wakeup(uint32_t BR)
{
	HAL_UART_CFG_T uartCfg;
	HAL_UART_IRQ_STATUS_T mask =
	{
		.txModemStatus          = 0,
		.rxDataAvailable        = 1,
		.txDataNeeded           = 0,
		.rxTimeout              = 0,
		.rxLineErr              = 0,
		.txDmaDone              = 1,
		.rxDmaDone              = 0,
		.rxDmaTimeout           = 0,
		.DTR_Rise				= 0,
		.DTR_Fall				= 0,
	};
	DBG("!");
	uartCfg.afc = HAL_UART_AFC_MODE_DISABLE;
	uartCfg.data = HAL_UART_8_DATA_BITS;
	uartCfg.irda = HAL_UART_IRDA_MODE_DISABLE;
	uartCfg.parity = HAL_UART_NO_PARITY;
	uartCfg.stop = HAL_UART_1_STOP_BIT;
	uartCfg.rx_mode = HAL_UART_TRANSFERT_MODE_DIRECT_IRQ;
	uartCfg.tx_mode = HAL_UART_TRANSFERT_MODE_DMA_IRQ;
	uartCfg.tx_trigger = HAL_UART_TX_TRIG_EMPTY;
	uartCfg.rx_trigger = 2;
	uartCfg.rate = BR;
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	hwp_iomux->pad_GPIO_4_cfg = IOMUX_PAD_GPIO_4_SEL_FUN_UART2_RXD_SEL;
	hwp_iomux->pad_GPIO_5_cfg = IOMUX_PAD_GPIO_5_SEL_FUN_UART2_TXD_SEL;
#endif

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_configRegs->Alt_mux_select |= CFG_REGS_UART2_UART2;
	hwp_configRegs->GPIO_Mode &= ~(1 << 8)|(1 << 13);
#endif

	OS_UartClose(GPS_UART_ID);
	OS_UartOpen(GPS_UART_ID, &uartCfg, mask, GPS_IRQHandle);
	GPS_UART->CMD_Set = UART_RX_FIFO_RESET|UART_TX_FIFO_RESET;
	GPS_UART->status = UART_ENABLE;

	GPSCtrl.IsWork = 1;
	GPIO_Write(GPS_POWER_PIN, 1);

	GPSCtrl.NoDataTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_NODATA_TO];
	GPSCtrl.NoLocatTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_V_TO];
	GPSCtrl.KeepTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_KEEP_TO];
	GPSCtrl.GPSVaildTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_KEEP_TO];
	GPSCtrl.LocatTime = 0;
	gSys.State[GPS_STATE] = GPS_V_STAGE;
	Led_Flush(LED_TYPE_GPS, LED_FLUSH_SLOW);
	GPSCtrl.AnalyzeLen = 0;
	GPSCtrl.RxPos = 0;
	GPSCtrl.RxState = 0;
}

void GPS_Sleep(void)
{
	DBG("!");
	GPSCtrl.RemotePrintTime = 0xff;
	GPSCtrl.SleepTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_SLEEP_TO];
	gSys.State[GPS_STATE] = GPS_STOP;
	gSys.RMCInfo->LocatStatus = 0;
	Led_Flush(LED_TYPE_GPS, LED_OFF);
	GPIO_Write(GPS_POWER_PIN, 0);
	OS_UartClose(GPS_UART_ID);

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	hwp_iomux->pad_GPIO_4_cfg = IOMUX_PAD_GPIO_4_SEL_FUN_GPIO_4_SEL;
	hwp_iomux->pad_GPIO_5_cfg = IOMUX_PAD_GPIO_5_SEL_FUN_GPIO_5_SEL;
#endif

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_configRegs->Alt_mux_select &= ~CFG_REGS_UART2_UART2;
	hwp_configRegs->GPIO_Mode |= (1 << 8)|(1 << 13);
#endif
}

void GPS_StateCheck(void)
{
	uint8_t IsGSAct = 0;
	uint8_t IsAct = 0;
	IO_ValueUnion IO;
	if (GPSCtrl.Param[PARAM_GS_WAKEUP_GPS])
	{
		if (gSys.Var[GSENSOR_ALARM_VAL] >= G_POWER(GPSCtrl.Param[PARAM_GS_WAKEUP_GPS]))
		{
			IsGSAct = 1;
			IsAct = 1;
		}
	}
	else
	{
		IsGSAct = 1;
	}

	if (GPSCtrl.Param[PARAM_VACC_WAKEUP_GPS])
	{
		IO.Val = gSys.Var[IO_VAL];
		if (IO.IOVal.VACC)
		{
			IsAct = 1;
		}
	}
	else if (!GPSCtrl.Param[PARAM_GS_WAKEUP_GPS])
	{
		IsAct = 1;
	}

#ifdef __GPS_TEST__
	IsAct = 1;
#endif
	if (GPSCtrl.IsWork)
	{
		if (gSys.Error[SIM_ERROR] || (SYSTEM_POWER_STOP == gSys.State[SYSTEM_STATE]))
		{
			DBG("sim or power error, sleep!");
			GPS_Sleep();
			return ;
		}

		if (gSys.Var[SYS_TIME] > GPSCtrl.NoDataTime)
		{
			DBG("%usec no data, reboot!", GPSCtrl.Param[PARAM_GPS_NODATA_TO]);
			SYS_Error(GPS_ERROR, 1);
			GPSCtrl.NoDataTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_NODATA_TO];
			GPSCtrl.NoLocatTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_V_TO];
			OS_SendEvent(gSys.TaskID[GPS_TASK_ID], EV_MMI_GPS_REBOOT, 0, 0, 0);
			return ;
		}
		if (IsAct)
		{
			GPSCtrl.KeepTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_KEEP_TO];
		}
		if(IsGSAct)
		{
			GPSCtrl.GPSVaildTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_KEEP_TO];
		}

		if (GPSCtrl.Param[PARAM_GPS_KEEP_TO])
		{
			if (gSys.Var[SYS_TIME] > GPSCtrl.KeepTime)
			{
				DBG("%usec no active, sleep!", GPSCtrl.Param[PARAM_GPS_KEEP_TO]);
				GPSCtrl.IsWork = 0;
				GPS_Sleep();
				return ;
			}
		}
	}


	switch (gSys.State[GPS_STATE])
	{
	case GPS_V_STAGE:
		if (GPSCtrl.Param[PARAM_GPS_V_TO])
		{
			if (gSys.Var[SYS_TIME] > GPSCtrl.NoLocatTime)
			{
				DBG("%usec no locat, reboot!", GPSCtrl.Param[PARAM_GPS_V_TO]);
				OS_SendEvent(gSys.TaskID[GPS_TASK_ID], EV_MMI_GPS_REBOOT, 0, 0, 0);
				GPSCtrl.NoDataTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_NODATA_TO];
				GPSCtrl.NoLocatTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_V_TO];
				return ;

			}
			if (GPSCtrl.LocatTime >= 3)
			{
				GPSCtrl.LocatTime = 3;
				gSys.State[GPS_STATE] = GPS_A_STAGE;
				Led_Flush(LED_TYPE_GPS, LED_ON);
				SYS_CheckTime(&gSys.RMCInfo->UTCDate, &gSys.RMCInfo->UTCTime);
				DBG("locat!");
				if (!gSys.State[FIRST_LOCAT_STATE])
				{
					gSys.State[FIRST_LOCAT_STATE] = 1;
					Param_Save(PARAM_TYPE_LOCAT);
					Monitor_RecordData();
					Monitor_Upload();
					if (GPSCtrl.Param[PARAM_GPS_ONLY_ONCE])
					{
						GPSCtrl.KeepTime = gSys.Var[SYS_TIME];
					}
				}
			}
		}
		break;
	case GPS_A_STAGE:
		GPSCtrl.NoLocatTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_V_TO];
		if (!gSys.RMCInfo->LocatStatus)
		{
			gSys.State[GPS_STATE] = GPS_V_STAGE;
			GPSCtrl.LocatTime = 0;
			Led_Flush(LED_TYPE_GPS, LED_FLUSH_SLOW);
			DBG("no locat!");
		}
		break;
	default:
		GPSCtrl.IsWork = 0;
		if (SYSTEM_POWER_STOP == gSys.State[SYSTEM_STATE])
		{
			GPSCtrl.SleepTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_SLEEP_TO];
			return;
		}

		if (IsAct && !(gSys.Error[SIM_ERROR] || (SYSTEM_POWER_STOP == gSys.State[SYSTEM_STATE])))
		{
			DBG("GPS active");
			GPS_Wakeup(gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_GPS_BR]);
		}
		else
		{
			if (GPSCtrl.Param[PARAM_GPS_SLEEP_TO])
			{
				if (gSys.Var[SYS_TIME] > GPSCtrl.SleepTime)
				{
					DBG("GPS sleep %usec wakeup ", GPSCtrl.Param[PARAM_GPS_SLEEP_TO]);
					GPS_Wakeup(gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_GPS_BR]);
				}
			}
		}
		break;
	}
}

void GPS_IRQHandle(HAL_UART_IRQ_STATUS_T Status, HAL_UART_ERROR_STATUS_T Error)
{
	uint8_t Temp;
	uint8_t i;
	if (Status.rxDataAvailable)
	{
		i = GPS_UART->status & UART_RX_FIFO_LEVEL_MASK;
		while (i)
		{
			Temp = GPS_UART->rxtx_buffer;
			GPS_Receive(NULL, Temp);
			i--;
		}
	}
	if (Status.txDmaDone)
	{

	}
	GPS_UART->status = UART_ENABLE;
	hwp_uart->CMD_Set = UART_RI;
}

void GPS_Print(void)
{
	uint8_t PrintGPSBuf[GPS_LEN_MAX + 4];
	memcpy(&PrintGPSBuf[1], GPSCtrl.AnalyzeBuf, GPSCtrl.AnalyzeLen);
	PrintGPSBuf[0] = '$';
	PrintGPSBuf[GPSCtrl.AnalyzeLen + 1] = '\r';
	PrintGPSBuf[GPSCtrl.AnalyzeLen + 2] = '\n';
	COM_TxReq(PrintGPSBuf, GPSCtrl.AnalyzeLen + 3);
}

void GPS_RemotePrint(void)
{
	GPSCtrl.RemotePrintTime = 0;
}

void GPS_Task(void *pData)
{
	COS_EVENT Event = { 0 };
	DBG("Task start! %u %u %u %u %u %u", GPSCtrl.Param[PARAM_GS_WAKEUP_GPS],
			GPSCtrl.Param[PARAM_VACC_WAKEUP_GPS],
			GPSCtrl.Param[PARAM_GPS_NODATA_TO],
			GPSCtrl.Param[PARAM_GPS_V_TO],
			GPSCtrl.Param[PARAM_GPS_KEEP_TO],
			GPSCtrl.Param[PARAM_GPS_SLEEP_TO],
			GPSCtrl.Param[PARAM_AGPS_EN]);
    memset(&Event, 0, sizeof(COS_EVENT));

	GPS_Wakeup(gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_GPS_BR]);
    while(1)
    {
    	COS_WaitEvent(gSys.TaskID[GPS_TASK_ID], &Event, COS_WAIT_FOREVER);
#ifdef __NO_GPS__
    	continue;
#endif
    	switch (Event.nEventId)
    	{
    	case EV_TIMER:
    		DBG("%u", Event.nParam1);
    		OS_StopTimer(gSys.TaskID[GPS_TASK_ID], Event.nParam1);
    		break;
        case EV_MMI_GPS_ANALYZE:
        	if (PRINT_GPS == gSys.State[PRINT_STATE])
        	{
        		GPS_Print();
        	}
        	GPSCtrl.AnalyzeBuf[GPSCtrl.AnalyzeLen] = 0;
        	GPS_Analyze(GPSCtrl.AnalyzeBuf, GPSCtrl.AnalyzeLen);
    //    	strcpy(GPSCtrl.TxBuf, "$CFGCLR,hFF*5B\r\n");
    //    	GPS_SendCmd(16);
        	break;
        case EV_MMI_GPS_REBOOT:
        	if (GPSCtrl.IsWork)
        	{
        		DBG("GPS Reboot!");
        		Led_Flush(LED_TYPE_GPS, LED_FLUSH_SLOW);
				GPIO_Write(GPS_POWER_PIN, 0);
				GPS_UART->ctrl &= ~UART_ENABLE_ENABLE;
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
				hwp_iomux->pad_GPIO_4_cfg = IOMUX_PAD_GPIO_4_SEL_FUN_GPIO_4_SEL;
				hwp_iomux->pad_GPIO_5_cfg = IOMUX_PAD_GPIO_5_SEL_FUN_GPIO_5_SEL;
#endif

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
				hwp_configRegs->Alt_mux_select &= ~CFG_REGS_UART2_UART2;
				hwp_configRegs->GPIO_Mode |= (1 << 8)|(1 << 13);
#endif

				OS_Sleep(SYS_TICK/16);
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
				hwp_iomux->pad_GPIO_4_cfg = IOMUX_PAD_GPIO_4_SEL_FUN_UART2_RXD_SEL;
				hwp_iomux->pad_GPIO_5_cfg = IOMUX_PAD_GPIO_5_SEL_FUN_UART2_TXD_SEL;
#endif

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
				hwp_configRegs->Alt_mux_select |= CFG_REGS_UART2_UART2;
				hwp_configRegs->GPIO_Mode &= ~(1 << 8)|(1 << 13);
#endif
				GPIO_Write(GPS_POWER_PIN, 1);
				GPS_UART->ctrl |= UART_ENABLE_ENABLE;

				gSys.State[GPS_STATE] = GPS_V_STAGE;
				GPSCtrl.NoDataTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_NODATA_TO];
				GPSCtrl.NoLocatTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_V_TO];
				//GPSCtrl.KeepTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_KEEP_TO];
				GPSCtrl.GPSVaildTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_KEEP_TO];
				GPSCtrl.LocatTime = 0;
				GPSCtrl.RxState = 0;
        	}
        	break;
        case EV_MMI_AGPS_FILE_OK:
        	break;
        default:
        	break;
    	}

    }
}

void GPS_Config(void)
{
	gSys.TaskID[GPS_TASK_ID] = COS_CreateTask(GPS_Task, NULL,
			NULL, MMI_TASK_MIN_STACK_SIZE, MMI_TASK_PRIORITY + GPS_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI GPS Task");
	GPSCtrl.Param = gSys.nParam[PARAM_TYPE_GPS].Data.ParamDW.Param;
	gSys.State[GPS_STATE] = GPS_STOP;
	GPSCtrl.RemotePrintTime = 0xff;
#ifdef __GPS_TEST__
	gSys.State[PRINT_STATE] = PRINT_GPS;
#endif
}

void GPS_Receive(void *pData, uint8_t Data)
{

	switch (GPSCtrl.RxState)
	{
	case 0:
		if (Data == '$')		//发现起始头
		{
			GPSCtrl.RxState = 1;
			GPSCtrl.RxPos = 0;
		}
		break;
	case 1:
		if (Data == '$')		//重新发现起始头
		{
			GPSCtrl.RxPos = 0;
		}
		else if (Data == '\r')	//发现结尾
		{
			memcpy(GPSCtrl.AnalyzeBuf, GPSCtrl.RxBuf, GPSCtrl.RxPos);
			GPSCtrl.AnalyzeBuf[GPSCtrl.RxPos] = 0;
			GPSCtrl.AnalyzeLen = GPSCtrl.RxPos;
			//DBG("%s", GPSCtrl.AnalyzeBuf);
			GPSCtrl.RxState = 0;
			GPSCtrl.RxPos = 0;
			OS_SendEvent(gSys.TaskID[GPS_TASK_ID], EV_MMI_GPS_ANALYZE, 0, 0, 0);
	    	SYS_Error(GPS_ERROR, 0);
	    	GPSCtrl.NoDataTime = gSys.Var[SYS_TIME] + GPSCtrl.Param[PARAM_GPS_NODATA_TO];
		}
		else if (Data == '\n' || Data == 0 || Data >= 0x80)
		{
			DBG("gps char error %02x", Data);
			GPSCtrl.RxState = 0;
		}
		else
		{
			GPSCtrl.RxBuf[GPSCtrl.RxPos] = Data;
			GPSCtrl.RxPos++;
			if (GPSCtrl.RxPos >= GPS_LEN_MAX)
			{
				DBG("gps len error %u", GPSCtrl.RxPos);
				GPSCtrl.RxState = 0;
			}
		}
		break;
	}
}

void GPS_Analyze(int8_t *Data, uint32_t Len)
{
	uint32_t i;
	uint8_t Check, CheckSum = 0;

	AsciiToHex(Data + Len - 2, 1, &Check);

	CheckSum = XorCheck(Data, Len - 3, 0);
	if (CheckSum != Check)
	{
		Data[Len] = 0;
		DBG("%s, %x %x", Data, Check, CheckSum);
	}
	Data[Len - 3] = 0;
	for (i = 0; i < sizeof(GPSStrFun)/sizeof(StrFunStruct);i++)
	{
		if (!memcmp(GPSCtrl.AnalyzeBuf + 2, GPSStrFun[i].Cmd, 3))
		{
			if ((uint32_t)GPSStrFun[i].Func > (FLASH_BASE + USER_CODE_START))
			{
				GPSStrFun[i].Func(Data);
			}
		}
	}
}

static uint8_t outOfChina(double lat, double lon)
{
	if (lon < 72.004 || lon > 137.8347)
    	return 1;
	if (lat < 0.8293 || lat > 55.8271)
		return 1;
	return 0;
}

static double transformlat(double x, double y)
{
	double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(x);
	double pi = 3.14159265358979324;
	ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
	ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
	ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi / 30.0)) * 2.0 / 3.0;
	return ret;
}

static double transformlon(double x, double y)
{
	double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(x);
	double pi = 3.14159265358979324;
	ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
	ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
	ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
	return ret;
}

void WGS84ToGCJ02(double wglat, double wglgt, double *china_lat, double *china_lgt)
{
	double pi = 3.14159265358979324;
    double a = 6378245.0;
    double ee = 0.00669342162296594323;
    double dLat;
    double dLon;
    double radLat;
    double magic;
    double sqrtMagic;
    if (outOfChina(wglat, wglgt)) {
        *china_lat = wglat;
        *china_lgt = wglgt;
        return;
    }
    dLat = transformlat(wglgt - 105.0, wglat - 35.0);
    dLon = transformlon(wglgt - 105.0, wglat - 35.0);
    radLat = wglat / 180.0 * pi;
    magic = sin(radLat);

    magic = 1 - ee * magic * magic;
    sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
	dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
	*china_lat = wglat + dLat;
	*china_lgt = wglgt + dLon;
}


static double rad(double d)
{
   return d * 3.1415926535898 / 180.0;
}

double GPS_Distance(double lat1, double lat2, double lgt1, double lgt2)
{
	double radLat1 = rad(lat1);
	double radLat2 = rad(lat2);
	double a = (radLat1 > radLat2)?(radLat1 - radLat2):(radLat2 - radLat1);
	double b = (lgt1 > lgt2)?(rad(lgt1) - rad(lgt2)):(rad(lgt2) - rad(lgt1));
	double EARTH_RADIUS = 6378.137;
	double s = 2 * asin(sqrt(pow(sin(a/2),2) + cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));
	s = s * EARTH_RADIUS * 1000;
	return s;
}
