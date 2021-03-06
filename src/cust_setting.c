#include "user.h"

uint8_t Mem_Check(uint8_t *Start, uint8_t Check, uint32_t Len)
{
	uint32_t i;
	for (i = 0; i < Len; i++)
	{
		if (Start[i] != Check)
		{
			return 0;
		}
	}
	return 1;
}

uint8_t Param_Load(uint8_t Type, uint8_t *FlashBuf)
{
	uint32_t Addr1 = PARAM_PID_ADDR + Type * FLASH_SECTOR_LEN * 2;
	uint32_t Addr2 = Addr1 + FLASH_SECTOR_LEN;
	uint32_t i, Pos1, Pos2, Pos3 = 0;
	Param_Byte64Struct *Byte64;
	Param_Byte64Struct Byte64_1;
	Param_Byte64Struct Byte64_2;
	Param_Byte64Struct *ParamBuf = &gSys.nParam[Type];

	if (Type >= PARAM_TYPE_MAX)
	{
		DBG("%d!", Type);
		return 0;
	}

	if (Type != PARAM_TYPE_MAIN)
	{
		Addr1 = PARAM_START_ADDR + (Type - 1) * FLASH_SECTOR_LEN;
		Addr2 = 0;
	}


	__ReadFlash(Addr1, FlashBuf, FLASH_SECTOR_LEN);
	Pos1 = 0;
	for (i = 0; i < FLASH_SECTOR_LEN / sizeof(Param_Byte64Struct); i++)
	{
		Byte64 = (Param_Byte64Struct *)&FlashBuf[i * sizeof(Param_Byte64Struct)];
		if (Byte64->CRC32 != __CRC32((uint8_t *)&Byte64->Data, sizeof(Param_Byte60Union), CRC32_START))
		{
			if (!Mem_Check((uint8_t *)Byte64, 0xff, sizeof(Param_Byte64Struct)))
			{
				DBG("Param %u flash1 %u error", Type, i);
				__EraseSector(Addr1);
				if (Pos3)
				{
					__WriteFlash(Addr1, (uint8_t *)ParamBuf, sizeof(Param_Byte64Struct));
					Pos1 = 1;
				}
				else
				{
					Pos1 = 0;
				}

			}
			break;
		}
		else
		{
			Pos1 = i + 1;
		}
	}

	if (Pos1)
	{
		memcpy(&Byte64_1, &FlashBuf[(Pos1 - 1) * sizeof(Param_Byte64Struct)], sizeof(Param_Byte64Struct));
	}
	Pos2 = 0;
	if (Type == PARAM_TYPE_MAIN)
	{
		__ReadFlash(Addr2, FlashBuf, FLASH_SECTOR_LEN);
		for (i = 0; i < FLASH_SECTOR_LEN / sizeof(Param_Byte64Struct); i++)
		{
			Byte64 = (Param_Byte64Struct *)&FlashBuf[i * sizeof(Param_Byte64Struct)];
			if (Byte64->CRC32 != __CRC32((uint8_t *)&Byte64->Data, sizeof(Param_Byte60Union), CRC32_START))
			{
				if (!Mem_Check((uint8_t *)Byte64, 0xff, sizeof(Param_Byte64Struct)))
				{
					DBG("Param %u flash2 %u error", Type, i);
					__EraseSector(Addr2);
					if (Pos3)
					{
						__WriteFlash(Addr2, (uint8_t *)ParamBuf, sizeof(Param_Byte64Struct));
						Pos1 = 0;
					}
					else
					{
						Pos2 = 0;
					}
				}
				break;
			}
			else
			{
				Pos2 = i + 1;
			}
		}
		if (Pos2)
		{
			memcpy(&Byte64_2, &FlashBuf[(Pos2 - 1) * sizeof(Param_Byte64Struct)], sizeof(Param_Byte64Struct));
		}
	}


	if (Pos1)
	{
		DBG("Param %u last save ok! %u", Type, Pos1);
		memcpy(ParamBuf, &Byte64_1, sizeof(Param_Byte64Struct));
	}
	else if (Pos2 && (Type == PARAM_TYPE_MAIN) )
	{
		DBG("Param %u recovery!", Type);
		memcpy(ParamBuf, &Byte64_2, sizeof(Param_Byte64Struct));
		__EraseSector(Addr1);
		__WriteFlash(Addr1, (uint8_t *)ParamBuf, sizeof(Param_Byte64Struct));

		Pos1 = 1;
		if (Pos2 > 1)
		{
			__EraseSector(Addr2);
			__WriteFlash(Addr2, (uint8_t *)ParamBuf, sizeof(Param_Byte64Struct));
			Pos2 = 1;
		}
	}

	if (Pos1 > (FLASH_SECTOR_LEN / sizeof(Param_Byte64Struct) / 2))
	{
		DBG("Param %u last save too much, recovery!", Type);
		__EraseSector(Addr1);
		__WriteFlash(Addr1, (uint8_t *)ParamBuf, sizeof(Param_Byte64Struct));
		__EraseSector(Addr2);
		__WriteFlash(Addr2, (uint8_t *)ParamBuf, sizeof(Param_Byte64Struct));
	}

	return Pos1 + Pos2 + Pos3;
}

void Param_Config(void)
{
	IP_AddrUnion uIP;
	uint8_t i;
	uint8_t *Buf = (uint8_t *)gSys.FlashBuf;
	Param_Byte64Struct *Param;
	//uint32_t Result;
	Param = &gSys.nParam[PARAM_TYPE_MAIN];
	if (!Param_Load(PARAM_TYPE_MAIN, Buf))
	{
		Param->Data.MainInfo.UID[0] = 0;
		Param->Data.MainInfo.UID[1] = 0;
		Param->Data.MainInfo.UID[2] = 0;
		uIP.u8_addr[0] = __CUST_IP_ADDR1__;
		uIP.u8_addr[1] = __CUST_IP_ADDR2__;
		uIP.u8_addr[2] = __CUST_IP_ADDR3__;
		uIP.u8_addr[3] = __CUST_IP_ADDR4__;
		Param->Data.MainInfo.MainIP = uIP.u32_addr;
		strcpy(Param->Data.MainInfo.MainURL, __CUST_URL__);
		Param->Data.MainInfo.UDPPort = __CUST_UDP_PORT__;
		Param->Data.MainInfo.TCPPort = __CUST_TCP_PORT__;
		Param->Data.MainInfo.CustCode = __CUST_CODE__;
	}
	else
	{
		if (Param->Data.MainInfo.CustCode != __CUST_CODE__)
		{
			uIP.u8_addr[0] = __CUST_IP_ADDR1__;
			uIP.u8_addr[1] = __CUST_IP_ADDR2__;
			uIP.u8_addr[2] = __CUST_IP_ADDR3__;
			uIP.u8_addr[3] = __CUST_IP_ADDR4__;
			Param->Data.MainInfo.MainIP = uIP.u32_addr;
			strcpy(Param->Data.MainInfo.MainURL, __CUST_URL__);
			Param->Data.MainInfo.UDPPort = __CUST_UDP_PORT__;
			Param->Data.MainInfo.TCPPort = __CUST_TCP_PORT__;
			Param->Data.MainInfo.CustCode = __CUST_CODE__;
			Param_Save(PARAM_TYPE_MAIN);
			for (i = PARAM_TYPE_SYS; i < PARAM_TYPE_MAX; i++)
			{
				Param_Format(i);
			}
			SYS_Reset();
		}
	}
	uIP.u32_addr = Param->Data.MainInfo.MainIP;
	DBG("%u %u %u %u.%u.%u.%u %s %u %u", Param->Data.MainInfo.UID[0], Param->Data.MainInfo.UID[1], Param->Data.MainInfo.UID[2],
			uIP.u8_addr[0], uIP.u8_addr[1], uIP.u8_addr[2], uIP.u8_addr[3], Param->Data.MainInfo.MainURL, Param->Data.MainInfo.TCPPort, Param->Data.MainInfo.UDPPort);

	if (!Param_Load(PARAM_TYPE_SYS, Buf))
	{
		DBG("%u no data", PARAM_TYPE_SYS);
		Param = &gSys.nParam[PARAM_TYPE_SYS];
		memset(Param, 0, sizeof(Param_Byte64Struct));
		Param->Data.ParamDW.Param[PARAM_DETECT_PERIOD] = 8;
		Param->Data.ParamDW.Param[PARAM_STOP_VBAT] = 3400;
		Param->Data.ParamDW.Param[PARAM_LOW_VBAT] = 3600;
		Param->Data.ParamDW.Param[PARAM_NORMAL_VBAT] = 3900;
		Param->Data.ParamDW.Param[PARAM_COM_BR] = HAL_UART_BAUD_RATE_9600;
		Param->Data.ParamDW.Param[PARAM_GPS_BR] = HAL_UART_BAUD_RATE_9600;
		Param->Data.ParamDW.Param[PARAM_SMS_ALARM] = 0;
		Param->Data.ParamDW.Param[PARAM_CALL_AUTO_GET] = 0;
		Param->Data.ParamDW.Param[PARAM_SIM_TO] = 10;
		Param->Data.ParamDW.Param[PARAM_GPRS_TO] = 60;
#if (__CUST_CODE__ == __CUST_KQ__)
		Param->Data.ParamDW.Param[PARAM_COM_BR] = HAL_UART_BAUD_RATE_115200;
		Param->Data.ParamDW.Param[PARAM_STOP_VBAT] = 2600;
		Param->Data.ParamDW.Param[PARAM_SIM_TO] = 120;
		Param->Data.ParamDW.Param[PARAM_GPRS_TO] = 120;
#elif (__CUST_CODE__ == __CUST_LY__)

#elif (__CUST_CODE__ == __CUST_GLEAD__)

#elif (__CUST_CODE__ == __CUST_LB_V3__ || __CUST_CODE__ == __CUST_LB_V2__)

#elif (__CUST_CODE__ == __CUST_LY_IOTDEV__)

#endif

#ifdef __MINI_SYSTEM__
		Param->Data.ParamDW.Param[PARAM_DETECT_PERIOD] = 1;
#endif
#ifdef __ANT_TEST__
		Param->Data.ParamDW.Param[PARAM_CALL_AUTO_GET] = 1;
#endif
		Param->CRC32 = __CRC32((uint8_t *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}
	Param = &gSys.nParam[PARAM_TYPE_SYS];
	if ( (Param->Data.ParamDW.Param[PARAM_DETECT_PERIOD] > 64) || (Param->Data.ParamDW.Param[PARAM_DETECT_PERIOD] == 0) )
	{
		Param->Data.ParamDW.Param[PARAM_DETECT_PERIOD] = 8;
	}

	if (!Param_Load(PARAM_TYPE_GPS, Buf))
	{
		DBG("%u no data", PARAM_TYPE_GPS);
		Param = &gSys.nParam[PARAM_TYPE_GPS];
		memset(Param, 0, sizeof(Param_Byte64Struct));
		Param->Data.ParamDW.Param[PARAM_GS_WAKEUP_GPS] = 10;
		Param->Data.ParamDW.Param[PARAM_VACC_WAKEUP_GPS] = 1;
		Param->Data.ParamDW.Param[PARAM_GPS_NODATA_TO] = 3;
		Param->Data.ParamDW.Param[PARAM_GPS_V_TO] = 150;
		Param->Data.ParamDW.Param[PARAM_GPS_KEEP_TO] = 120;
		Param->Data.ParamDW.Param[PARAM_GPS_SLEEP_TO] = 0;//为0表示GPS不自动唤醒
		Param->Data.ParamDW.Param[PARAM_AGPS_EN] = 1;
		Param->Data.ParamDW.Param[PARAM_GPS_ONLY_ONCE] = 0;
#if (__CUST_CODE__ == __CUST_KQ__)
		Param->Data.ParamDW.Param[PARAM_GPS_SLEEP_TO] = 180;//为0表示GPS不自动唤醒
		Param->Data.ParamDW.Param[PARAM_GPS_ONLY_ONCE] = 1;
#elif (__CUST_CODE__ == __CUST_LY__)

#elif (__CUST_CODE__ == __CUST_GLEAD__)
		Param->Data.ParamDW.Param[PARAM_GPS_SLEEP_TO] = 4 * 3600;//为0表示GPS不自动唤醒
#elif (__CUST_CODE__ == __CUST_LB_V3__ || __CUST_CODE__ == __CUST_LB_V2__)
#elif (__CUST_CODE__ == __CUST_LY_IOTDEV__)
		Param->Data.ParamDW.Param[PARAM_AGPS_EN] = 0;
#endif

#ifdef __MINI_SYSTEM__
		Param->Data.ParamDW.Param[PARAM_GPS_KEEP_TO] = 1;
#endif
		Param->CRC32 = __CRC32((uint8_t *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}

	if (!Param_Load(PARAM_TYPE_MONITOR, Buf))
	{
		DBG("%u no data", PARAM_TYPE_MONITOR);
		Param = &gSys.nParam[PARAM_TYPE_MONITOR];
		memset(Param, 0, sizeof(Param_Byte64Struct));

		Param->Data.ParamDW.Param[PARAM_GS_WAKEUP_MONITOR] = 0;//为0表示VACC唤醒
		Param->Data.ParamDW.Param[PARAM_GS_JUDGE_RUN] = 10;//为0则表示ACC开时，始终发送
		Param->Data.ParamDW.Param[PARAM_UPLOAD_RUN_PERIOD] = 30;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_STOP_PERIOD] = 12 * 3600;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_HEART_PERIOD] = 180;
		Param->Data.ParamDW.Param[PARAM_MONITOR_NET_TO] = 65;//系统TCP超时62秒，所有设置为65秒
		Param->Data.ParamDW.Param[PARAM_MONITOR_KEEP_TO] = 125;//为0表示永远在线
		Param->Data.ParamDW.Param[PARAM_MONITOR_SLEEP_TO] = 0;//为0表示休眠期间，不周期性启动发送数据
		Param->Data.ParamDW.Param[PARAM_MONITOR_RECONNECT_MAX] = 8;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ADD_MILEAGE] = 1;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ACC_UPLOAD] = 1;
#if (__CUST_CODE__ == __CUST_KQ__)

		Param->Data.ParamDW.Param[PARAM_GS_JUDGE_RUN] = 0;//不为0则表示在不骑行的时候，降低发送频率
		Param->Data.ParamDW.Param[PARAM_UPLOAD_RUN_PERIOD] = 60;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_STOP_PERIOD] = 300;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_HEART_PERIOD] = 50;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ADD_MILEAGE] = 0;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ACC_UPLOAD] = 0;

#elif (__CUST_CODE__ == __CUST_LY__)

		Param->Data.ParamDW.Param[PARAM_UPLOAD_RUN_PERIOD] = 30;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_STOP_PERIOD] = 600;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_HEART_PERIOD] = 50;
		Param->Data.ParamDW.Param[PARAM_MONITOR_RECONNECT_MAX] = 4;

#elif (__CUST_CODE__ == __CUST_GLEAD__)
		Param->Data.ParamDW.Param[PARAM_UPLOAD_HEART_PERIOD] = 150;
		Param->Data.ParamDW.Param[PARAM_MONITOR_KEEP_TO] = 0;//为0表示永远在线
		Param->Data.ParamDW.Param[PARAM_MONITOR_SLEEP_TO] = 0;//为0表示休眠期间，不周期性启动发送数据
		Param->Data.ParamDW.Param[PARAM_MONITOR_ACC_UPLOAD] = 1;
#elif (__CUST_CODE__ == __CUST_LB_V3__ || __CUST_CODE__ == __CUST_LB_V2__)
		Param->Data.ParamDW.Param[PARAM_GS_JUDGE_RUN] = 10;//不为0则表示在不骑行的时候，降低发送频率
		Param->Data.ParamDW.Param[PARAM_UPLOAD_RUN_PERIOD] = 15;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_HEART_PERIOD] = 180;

#elif (__CUST_CODE__ == __CUST_LY_IOTDEV__)

		Param->Data.ParamDW.Param[PARAM_UPLOAD_RUN_PERIOD] = 30;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_STOP_PERIOD] = 600;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_HEART_PERIOD] = 50;
		Param->Data.ParamDW.Param[PARAM_MONITOR_RECONNECT_MAX] = 4;

#endif
		Param->CRC32 = __CRC32((uint8_t *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}

	if (!Param_Load(PARAM_TYPE_ALARM1, Buf))
	{
		DBG("%u no data", PARAM_TYPE_ALARM1);
		Param = &gSys.nParam[PARAM_TYPE_ALARM1];
		memset(Param, 0, sizeof(Param_Byte64Struct));
#if (__CUST_CODE__ == __CUST_LY__ )
		Param->Data.ParamDW.Param[PARAM_VACC_CTRL_ALARM] = 1;
		Param->Data.ParamDW.Param[PARAM_ALARM_ON_DELAY] = 0;
		Param->Data.ParamDW.Param[PARAM_CRASH_GS] = 15;
		Param->Data.ParamDW.Param[PARAM_CRASH_JUDGE_TO] = 10;
		Param->Data.ParamDW.Param[PARAM_CRASH_JUDGE_CNT] = 6;
		Param->Data.ParamDW.Param[PARAM_CRASH_ALARM_WAIT_TO] = 0;
		Param->Data.ParamDW.Param[PARAM_CRASH_ALARM_FLUSH_TO] = 300;
		Param->Data.ParamDW.Param[PARAM_CRASH_ALARM_REPEAT] = 1;
		Param->Data.ParamDW.Param[PARAM_CRASH_WAKEUP_MOVE] = 0;
		Param->Data.ParamDW.Param[PARAM_MOVE_RANGE] = 500;
		Param->Data.ParamDW.Param[PARAM_MOVE_JUDGE_TO] = 0;
		Param->Data.ParamDW.Param[PARAM_MOVE_ALARM_FLUSH_TO] = 0;
		Param->Data.ParamDW.Param[PARAM_MOVE_ALARM_REPEAT] = 1;//绿源要求连续报警
#elif (__CUST_CODE__ == __CUST_LY_IOTDEV__)
		Param->Data.ParamDW.Param[PARAM_VACC_CTRL_ALARM] = 1;
		Param->Data.ParamDW.Param[PARAM_ALARM_ON_DELAY] = 0;
		Param->Data.ParamDW.Param[PARAM_CRASH_GS] = 15;
		Param->Data.ParamDW.Param[PARAM_CRASH_JUDGE_TO] = 10;
		Param->Data.ParamDW.Param[PARAM_CRASH_JUDGE_CNT] = 6;
		Param->Data.ParamDW.Param[PARAM_CRASH_ALARM_WAIT_TO] = 0;
		Param->Data.ParamDW.Param[PARAM_CRASH_ALARM_FLUSH_TO] = 300;
		Param->Data.ParamDW.Param[PARAM_CRASH_ALARM_REPEAT] = 1;
		Param->Data.ParamDW.Param[PARAM_CRASH_WAKEUP_MOVE] = 0;
		Param->Data.ParamDW.Param[PARAM_MOVE_RANGE] = 0;
		Param->Data.ParamDW.Param[PARAM_MOVE_JUDGE_TO] = 0;
		Param->Data.ParamDW.Param[PARAM_MOVE_ALARM_FLUSH_TO] = 0;
		Param->Data.ParamDW.Param[PARAM_MOVE_ALARM_REPEAT] = 0;
#else
		Param->Data.ParamDW.Param[PARAM_VACC_CTRL_ALARM] = 1;
		Param->Data.ParamDW.Param[PARAM_ALARM_ON_DELAY] = 30;
		Param->Data.ParamDW.Param[PARAM_CRASH_GS] = 15;
		Param->Data.ParamDW.Param[PARAM_CRASH_JUDGE_TO] = 5;
		Param->Data.ParamDW.Param[PARAM_CRASH_JUDGE_CNT] = 2;
		Param->Data.ParamDW.Param[PARAM_CRASH_ALARM_WAIT_TO] = 0;
		Param->Data.ParamDW.Param[PARAM_CRASH_ALARM_FLUSH_TO] = 120;
		Param->Data.ParamDW.Param[PARAM_CRASH_ALARM_REPEAT] = 0;
		Param->Data.ParamDW.Param[PARAM_CRASH_WAKEUP_MOVE] = 1;
		Param->Data.ParamDW.Param[PARAM_MOVE_RANGE] = 20;
		Param->Data.ParamDW.Param[PARAM_MOVE_JUDGE_TO] = 120;
		Param->Data.ParamDW.Param[PARAM_MOVE_ALARM_FLUSH_TO] = 120;
		Param->Data.ParamDW.Param[PARAM_MOVE_ALARM_REPEAT] = 0;
#endif
		Param->CRC32 = __CRC32((uint8_t *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}

	if (!Param_Load(PARAM_TYPE_ALARM2, Buf))
	{
		DBG("%u no data", PARAM_TYPE_ALARM2);
		Param = &gSys.nParam[PARAM_TYPE_ALARM2];
		memset(Param, 0, sizeof(Param_Byte64Struct));
		Param->Data.ParamDW.Param[PARAM_VACC_WAKEUP_CUTLINE_TO] = 1;
		Param->Data.ParamDW.Param[PARAM_CUTLINE_ALARM_DELAY] = 2;
		Param->Data.ParamDW.Param[PARAM_CUTLINE_ALARM_FLUSH_TO] = 0;
		Param->Data.ParamDW.Param[PARAM_OVERSPEED_ALARM_VAL] = 0;
		Param->Data.ParamDW.Param[PARAM_OVERSPEED_ALARM_DELAY] = 0;
		Param->Data.ParamDW.Param[PARAM_ALARM_ENABLE] = 1;
		Param->Data.ParamDW.Param[PARAM_LOCK_CAR] = 0;
#if (__CUST_CODE__ == __CUST_LY__)
		Param->Data.ParamDW.Param[PARAM_CUTLINE_ALARM_DELAY] = 5;
#elif (__CUST_CODE__ == __CUST_LY_IOTDEV__)
		Param->Data.ParamDW.Param[PARAM_CUTLINE_ALARM_DELAY] = 5;
#elif (__CUST_CODE__ == __CUST_GLEAD__)
		Param->Data.ParamDW.Param[PARAM_VACC_WAKEUP_CUTLINE_TO] = 5;
#endif
		Param->CRC32 = __CRC32((uint8_t *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}

	if (!Param_Load(PARAM_TYPE_APN, Buf))
	{
		DBG("%u no data", PARAM_TYPE_APN);
		Param = &gSys.nParam[PARAM_TYPE_APN];
		memset(Param, 0, sizeof(Param_Byte64Struct));
		Param->CRC32 = __CRC32((uint8_t *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}


	Param = &gSys.nParam[PARAM_TYPE_LOCAT];
	if (Param->CRC32 != __CRC32((uint8_t *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START))
	{
		if (!Param_Load(PARAM_TYPE_LOCAT, Buf))
		{
			DBG("%u no data", PARAM_TYPE_LOCAT);
			memset(Param, 0, sizeof(Param_Byte64Struct));
		}
	}
	else
	{
		if (gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LatDegree +
				gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LatMin +
				gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LgtDegree +
				gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LgtMin)
		{

		}
		else
		{
			if (gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LocatStatus)
			{

			}
			else
			{
				if (!Param_Load(PARAM_TYPE_LOCAT, Buf))
				{
					DBG("%u no data", PARAM_TYPE_LOCAT);
					memset(Param, 0, sizeof(Param_Byte64Struct));

				}
			}
		}
	}
	if (gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LatDegree +
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LatMin +
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LgtDegree +
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LgtMin)
	{

	}
	else
	{
		if (gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LocatStatus)
		{
			DBG("locat in 0,0!");
		}
		else
		{
			DBG("no locat, and 0,0, use default");
			Param->Data.LocatInfo.RMCSave.LatDegree = __CUST_LAT_DEGREE__;
			Param->Data.LocatInfo.RMCSave.LatMin = __CUST_LAT_MIN__;
			Param->Data.LocatInfo.RMCSave.LatNS = __CUST_LAT_NS__;
			Param->Data.LocatInfo.RMCSave.LgtDegree = __CUST_LGT_DEGREE__;
			Param->Data.LocatInfo.RMCSave.LgtMin = __CUST_LGT_MIN__;
			Param->Data.LocatInfo.RMCSave.LgtEW = __CUST_LGT_EW__;
			Param->CRC32 = __CRC32((uint8_t *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
		}
	}

	gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LocatStatus = 0;
	DBG("%u %u %u %u, %u, %u", gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LatDegree,
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LatMin,
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LgtDegree,
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LgtMin,
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageKM, gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageM);

	if (!Param_Load(PARAM_TYPE_UPGRADE, Buf))
	{
		DBG("%u no data", PARAM_TYPE_UPGRADE);
		Param = &gSys.nParam[PARAM_TYPE_UPGRADE];
		memset(Param, 0, sizeof(Param_Byte64Struct));
		Param->CRC32 = __CRC32((uint8_t *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}

	if (!Param_Load(PARAM_TYPE_NUMBER, Buf))
	{
		DBG("%u no data, init", PARAM_TYPE_NUMBER);
		Param = &gSys.nParam[PARAM_TYPE_NUMBER];
		memset(Param, 0, sizeof(Param_Byte64Struct));
	}
	Param = &gSys.nParam[PARAM_TYPE_NUMBER];
	//HexTrace(Param->Data.Number.Phone[0].Num, 8);

	if (!Param_Load(PARAM_TYPE_USER, Buf))
	{
		DBG("%u no data", PARAM_TYPE_USER);
		Param = &gSys.nParam[PARAM_TYPE_USER];
		memset(Param, 0, sizeof(Param_Byte64Struct));
#if (__CUST_CODE__ == __CUST_KQ__)

#endif
	}

	gSys.RMCInfo = &gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave;
	gSys.RMCInfo->LocatStatus = 0;
	gSys.RMCInfo->Speed = 0;
	gSys.RMCInfo->Cog = 0;
	Locat_CacheSave();
	memset(&gSys.LocalIP, 0, sizeof(IP_AddrUnion));
	memset(&gSys.DNS, 0, sizeof(gSys.DNS));
	memset(&gSys.GSVInfo, 0, sizeof(GSV_InfoStruct));
	memset(&gSys.GSVInfoSave, 0, sizeof(GSV_InfoStruct));
	memset(gSys.Var, 0, VAR_MAX * 4);
	memset(gSys.State, 0, STATE_MAX);
	memset(&gSys.CurrentCell, 0, sizeof(CFW_TSM_CURR_CELL_INFO));
	memset(&gSys.NearbyCell, 0, sizeof(CFW_TSM_ALL_NEBCELL_INFO));
	memset(gSys.IMSI, 0, IMSI_LEN);
	memset(gSys.ICCID, 0, ICCID_LEN);
	if (gSys.ErrorCRC32 != __CRC32(gSys.Error, ERROR_MAX, CRC32_START))
	{
		memset(gSys.Error, 0, sizeof(gSys.Error));
		gSys.uDateSave.dwDate = 0;
		gSys.uTimeSave.dwTime = 0;
	}
	else
	{
		gSys.Var[UTC_DATE] = gSys.uDateSave.dwDate;
		gSys.Var[UTC_TIME] = gSys.uTimeSave.dwTime;
		gSys.State[REBOOT_STATE] = 1;
	}

	if (gSys.Error[LOW_POWER_ERROR])
	{
		gSys.State[SYSTEM_STATE] = SYSTEM_POWER_LOW;
	}
	else
	{
		gSys.State[SYSTEM_STATE] = SYSTEM_POWER_ON;
	}
}

int32_t Param_Save(uint8_t Type)
{
	uint32_t Addr1 = PARAM_PID_ADDR + Type * FLASH_SECTOR_LEN * 2;
	uint32_t Addr2 = Addr1 + FLASH_SECTOR_LEN;
	uint32_t i, Pos1, Pos2;
	uint8_t *Buf = (uint8_t *)gSys.FlashBuf;
	if (gSys.Var[VBAT] < gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_LOW_VBAT])
	{
		return -1;
	}
	gSys.nParam[Type].CRC32 = __CRC32((uint8_t *)&gSys.nParam[Type].Data, sizeof(Param_Byte60Union), CRC32_START);
	if (Type != PARAM_TYPE_MAIN)
	{
		Addr1 = PARAM_START_ADDR + (Type - 1) * FLASH_SECTOR_LEN;
		Addr2 = 0;
	}
	__ReadFlash(Addr1, Buf, FLASH_SECTOR_LEN);
	Pos1 = 0;
	for (i = 0; i < FLASH_SECTOR_LEN / sizeof(Param_Byte64Struct); i++)
	{
		if (Mem_Check(&Buf[i * sizeof(Param_Byte64Struct)], 0xff, sizeof(Param_Byte64Struct)))
		{
			//DBG("Param %u save in flash1 %u", Type, i);
			Pos1 = i + 1;
			break;
		}
	}

	if (Type == PARAM_TYPE_MAIN)
	{
		__ReadFlash(Addr2, Buf, FLASH_SECTOR_LEN);
		Pos2 = 0;
		for (i = 0; i < FLASH_SECTOR_LEN / sizeof(Param_Byte64Struct); i++)
		{
			if (Mem_Check(&Buf[i * sizeof(Param_Byte64Struct)], 0xff, sizeof(Param_Byte64Struct)))
			{
				//DBG("Param %u save in flash2 %u", Type, i);
				Pos2 = i + 1;
				break;
			}
		}
	}
	else
	{
		Pos2 = 1;
	}

	if (!Pos1 || !Pos2)
	{
		DBG("Param %u save flash no free rom, please reboot", Type);
		return -1;
	}
	DBG("%u %u %u", Type, Pos1, Pos2);
	Addr1 = Addr1 + (Pos1 - 1) * sizeof(Param_Byte64Struct);
	__WriteFlash(Addr1, (uint8_t *)&gSys.nParam[Type], sizeof(Param_Byte64Struct));
	if (Type == PARAM_TYPE_MAIN)
	{
		Addr2 = Addr2 + (Pos2 - 1) * sizeof(Param_Byte64Struct);
		__WriteFlash(Addr2, (uint8_t *)&gSys.nParam[Type], sizeof(Param_Byte64Struct));
	}
	return 0;
}

int32_t Param_Format(uint8_t Type)
{
	uint32_t Addr1 = PARAM_PID_ADDR + Type * FLASH_SECTOR_LEN * 2;
	uint32_t Addr2 = Addr1 + FLASH_SECTOR_LEN;
	uint32_t i, Pos1, Pos2;
	uint8_t *Buf = (uint8_t *)gSys.FlashBuf;
	if (gSys.Var[VBAT] < gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_LOW_VBAT])
	{
		return -1;
	}
	gSys.nParam[Type].CRC32 = __CRC32((uint8_t *)&gSys.nParam[Type].Data, sizeof(Param_Byte60Union), CRC32_START) + 1;
	if (Type != PARAM_TYPE_MAIN)
	{
		Addr1 = PARAM_START_ADDR + (Type - 1) * FLASH_SECTOR_LEN;
		Addr2 = 0;
	}
	__ReadFlash(Addr1, Buf, FLASH_SECTOR_LEN);
	Pos1 = 0;
	for (i = 0; i < FLASH_SECTOR_LEN / sizeof(Param_Byte64Struct); i++)
	{
		if (Mem_Check(&Buf[i * sizeof(Param_Byte64Struct)], 0xff, sizeof(Param_Byte64Struct)))
		{
			//DBG("Param %u save in flash1 %u", Type, i);
			Pos1 = i + 1;
			break;
		}
	}
	if (Type == PARAM_TYPE_MAIN)
	{
		__ReadFlash(Addr2, Buf, FLASH_SECTOR_LEN);
		Pos2 = 0;
		for (i = 0; i < FLASH_SECTOR_LEN / sizeof(Param_Byte64Struct); i++)
		{
			if (Mem_Check(&Buf[i * sizeof(Param_Byte64Struct)], 0xff, sizeof(Param_Byte64Struct)))
			{
				//DBG("Param %u save in flash2 %u", Type, i);
				Pos2 = i + 1;
				break;
			}
		}
	}
	else
	{
		Pos2 = 1;
	}


	if (!Pos1 || !Pos2)
	{
		DBG("Param %u save flash no free rom, please reboot", Type);
		return -1;
	}
	DBG("%u %u", Pos1, Pos2);

	Addr1 = Addr1 + (Pos1 - 1) * sizeof(Param_Byte64Struct);
	__WriteFlash(Addr1, (uint8_t *)&gSys.nParam[Type], sizeof(Param_Byte64Struct));
	if (Type == PARAM_TYPE_MAIN)
	{
		Addr2 = Addr2 + (Pos2 - 1) * sizeof(Param_Byte64Struct);
		__WriteFlash(Addr2, (uint8_t *)&gSys.nParam[Type], sizeof(Param_Byte64Struct));
	}
	return 0;
}

void Locat_CacheSave(void)
{
	gSys.nParam[PARAM_TYPE_LOCAT].CRC32 = __CRC32((uint8_t *)&gSys.nParam[PARAM_TYPE_LOCAT].Data, sizeof(Param_Byte60Union), CRC32_START);
}
