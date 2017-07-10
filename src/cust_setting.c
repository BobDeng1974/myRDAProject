#include "user.h"

u8 Mem_Check(u8 *Start, u8 Check, u32 Len)
{
	u32 i;
	for (i = 0; i < Len; i++)
	{
		if (Start[i] != Check)
		{
			return 0;
		}
	}
	return 1;
}

u8 Param_Load(u8 Type, u8 *FlashBuf)
{
	u32 Addr1 = PARAM_PID_ADDR + Type * FLASH_SECTOR_LEN * 2;
	u32 Addr2 = Addr1 + FLASH_SECTOR_LEN;
	u32 i, Pos1, Pos2, Pos3;
	Param_Byte64Struct *Byte64;
	Param_Byte64Struct Byte64_1;
	Param_Byte64Struct Byte64_2;
	Param_Byte64Struct *ParamBuf = &gSys.nParam[Type];
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
		if (Byte64->CRC32 != __CRC32((u8 *)&Byte64->Data, sizeof(Param_Byte60Union), CRC32_START))
		{
			if (!Mem_Check((u8 *)Byte64, 0xff, sizeof(Param_Byte64Struct)))
			{
				DBG("Param %d flash1 %d error", Type, i);
				__EraseSector(Addr1);
				if (Pos3)
				{
					__WriteFlash(Addr1, (u8 *)ParamBuf, sizeof(Param_Byte64Struct));
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
			if (Byte64->CRC32 != __CRC32((u8 *)&Byte64->Data, sizeof(Param_Byte60Union), CRC32_START))
			{
				if (!Mem_Check((u8 *)Byte64, 0xff, sizeof(Param_Byte64Struct)))
				{
					DBG("Param %d flash2 %d error", Type, i);
					__EraseSector(Addr2);
					if (Pos3)
					{
						__WriteFlash(Addr2, (u8 *)ParamBuf, sizeof(Param_Byte64Struct));
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
		DBG("Param %d last save ok! %d", Type, Pos1);
		memcpy(ParamBuf, &Byte64_1, sizeof(Param_Byte64Struct));
	}
	else if (Pos2 && (Type == PARAM_TYPE_MAIN) )
	{
		DBG("Param %d recovery!", Type);
		memcpy(ParamBuf, &Byte64_2, sizeof(Param_Byte64Struct));
		__EraseSector(Addr1);
		__WriteFlash(Addr1, (u8 *)ParamBuf, sizeof(Param_Byte64Struct));

		Pos1 = 1;
		if (Pos2 > 1)
		{
			__EraseSector(Addr2);
			__WriteFlash(Addr2, (u8 *)ParamBuf, sizeof(Param_Byte64Struct));
			Pos2 = 1;
		}
	}

	if (Pos1 > (FLASH_SECTOR_LEN / sizeof(Param_Byte64Struct) / 2))
	{
		DBG("Param %d last save too much, recovery!", Type);
		__EraseSector(Addr1);
		__WriteFlash(Addr1, (u8 *)ParamBuf, sizeof(Param_Byte64Struct));
		__EraseSector(Addr2);
		__WriteFlash(Addr2, (u8 *)ParamBuf, sizeof(Param_Byte64Struct));
	}

	return Pos1 + Pos2 + Pos3;
}

void Param_Config(void)
{
	IP_AddrUnion uIP;
	u8 i;
	u8 *Buf = (u8 *)gSys.FlashBuf;
	Param_Byte64Struct *Param;
	u32 Result;
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
	DBG("%d %d %d %d.%d.%d.%d %s %d %d", Param->Data.MainInfo.UID[0], Param->Data.MainInfo.UID[1], Param->Data.MainInfo.UID[2],
			uIP.u8_addr[0], uIP.u8_addr[1], uIP.u8_addr[2], uIP.u8_addr[3], Param->Data.MainInfo.MainURL, Param->Data.MainInfo.TCPPort, Param->Data.MainInfo.UDPPort);

	if (!Param_Load(PARAM_TYPE_SYS, Buf))
	{
		DBG("%d no data", PARAM_TYPE_SYS);
		Param = &gSys.nParam[PARAM_TYPE_SYS];
		memset(Param, 0, sizeof(Param_Byte64Struct));
		Param->Data.ParamDW.Param[PARAM_DETECT_PERIOD] = 8;
#if (__CUST_CODE__ == __CUST_KQ__)
		Param->Data.ParamDW.Param[PARAM_SENSOR_EN] = 0;
		Param->Data.ParamDW.Param[PARAM_COM_BR] = HAL_UART_BAUD_RATE_115200;
		Param->Data.ParamDW.Param[PARAM_STOP_VBAT] = 2600;
		Param->Data.ParamDW.Param[PARAM_LOW_VBAT] = 3700;
		Param->Data.ParamDW.Param[PARAM_GPS_BR] = HAL_UART_BAUD_RATE_9600;
		Param->Data.ParamDW.Param[PARAM_NORMAL_VBAT] = 3900;
		Param->Data.ParamDW.Param[PARAM_SMS_ALARM] = 0;
		Param->Data.ParamDW.Param[PARAM_CALL_AUTO_GET] = 0;
		Param->Data.ParamDW.Param[PARAM_SIM_TO] = 120;
		Param->Data.ParamDW.Param[PARAM_GPRS_TO] = 120;

#elif (__CUST_CODE__ == __CUST_LY__)
		Param->Data.ParamDW.Param[PARAM_SENSOR_EN] = 1;
		Param->Data.ParamDW.Param[PARAM_COM_BR] = HAL_UART_BAUD_RATE_9600;
		Param->Data.ParamDW.Param[PARAM_GPS_BR] = HAL_UART_BAUD_RATE_9600;
		Param->Data.ParamDW.Param[PARAM_STOP_VBAT] = 3400;
		Param->Data.ParamDW.Param[PARAM_LOW_VBAT] = 3700;
		Param->Data.ParamDW.Param[PARAM_NORMAL_VBAT] = 3900;
		Param->Data.ParamDW.Param[PARAM_SMS_ALARM] = 0;
		Param->Data.ParamDW.Param[PARAM_CALL_AUTO_GET] = 0;
		Param->Data.ParamDW.Param[PARAM_SIM_TO] = 10;
		Param->Data.ParamDW.Param[PARAM_GPRS_TO] = 60;

#elif (__CUST_CODE__ == __CUST_GLEAD__)
		Param->Data.ParamDW.Param[PARAM_SENSOR_EN] = 1;
		Param->Data.ParamDW.Param[PARAM_COM_BR] = HAL_UART_BAUD_RATE_115200;
		Param->Data.ParamDW.Param[PARAM_STOP_VBAT] = 3600;
		Param->Data.ParamDW.Param[PARAM_LOW_VBAT] = 3700;
		Param->Data.ParamDW.Param[PARAM_GPS_BR] = HAL_UART_BAUD_RATE_9600;
		Param->Data.ParamDW.Param[PARAM_NORMAL_VBAT] = 3900;
		Param->Data.ParamDW.Param[PARAM_SMS_ALARM] = 0;
		Param->Data.ParamDW.Param[PARAM_CALL_AUTO_GET] = 0;
		Param->Data.ParamDW.Param[PARAM_SIM_TO] = 10;
		Param->Data.ParamDW.Param[PARAM_GPRS_TO] = 60;

#elif (__CUST_CODE__ == __CUST_LB__)
		Param->Data.ParamDW.Param[PARAM_SENSOR_EN] = 1;
		Param->Data.ParamDW.Param[PARAM_COM_BR] = HAL_UART_BAUD_RATE_9600;
		Param->Data.ParamDW.Param[PARAM_STOP_VBAT] = 3600;
		Param->Data.ParamDW.Param[PARAM_LOW_VBAT] = 3700;
		Param->Data.ParamDW.Param[PARAM_GPS_BR] = HAL_UART_BAUD_RATE_9600;
		Param->Data.ParamDW.Param[PARAM_NORMAL_VBAT] = 3900;
		Param->Data.ParamDW.Param[PARAM_SMS_ALARM] = 0;
		Param->Data.ParamDW.Param[PARAM_CALL_AUTO_GET] = 0;
		Param->Data.ParamDW.Param[PARAM_SIM_TO] = 10;
		Param->Data.ParamDW.Param[PARAM_GPRS_TO] = 60;
#elif (__CUST_CODE__ == __CUST_LY_IOTDEV__)
		Param->Data.ParamDW.Param[PARAM_DETECT_PERIOD] = 4;
		Param->Data.ParamDW.Param[PARAM_SENSOR_EN] = 1;
		Param->Data.ParamDW.Param[PARAM_COM_BR] = HAL_UART_BAUD_RATE_115200;
		Param->Data.ParamDW.Param[PARAM_STOP_VBAT] = 3600;
		Param->Data.ParamDW.Param[PARAM_LOW_VBAT] = 3700;
		Param->Data.ParamDW.Param[PARAM_GPS_BR] = HAL_UART_BAUD_RATE_9600;
		Param->Data.ParamDW.Param[PARAM_NORMAL_VBAT] = 3900;
		Param->Data.ParamDW.Param[PARAM_SMS_ALARM] = 0;
		Param->Data.ParamDW.Param[PARAM_CALL_AUTO_GET] = 0;
		Param->Data.ParamDW.Param[PARAM_SIM_TO] = 10;
		Param->Data.ParamDW.Param[PARAM_GPRS_TO] = 60;
#endif

#ifdef __MINI_SYSTEM__
		Param->Data.ParamDW.Param[PARAM_SENSOR_EN] = 0;
		Param->Data.ParamDW.Param[PARAM_DETECT_PERIOD] = 1;
#endif
		Param->CRC32 = __CRC32((u8 *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}
	Param = &gSys.nParam[PARAM_TYPE_SYS];
	if ( (Param->Data.ParamDW.Param[PARAM_DETECT_PERIOD] > 64) || (Param->Data.ParamDW.Param[PARAM_DETECT_PERIOD] == 0) )
	{
		Param->Data.ParamDW.Param[PARAM_DETECT_PERIOD] = 8;
	}

	if (!Param_Load(PARAM_TYPE_GPS, Buf))
	{
		DBG("%d no data", PARAM_TYPE_GPS);
		Param = &gSys.nParam[PARAM_TYPE_GPS];
		memset(Param, 0, sizeof(Param_Byte64Struct));
#if (__CUST_CODE__ == __CUST_KQ__)
		Param->Data.ParamDW.Param[PARAM_GS_WAKEUP_GPS] = 10;
		Param->Data.ParamDW.Param[PARAM_VACC_WAKEUP_GPS] = 1;
		Param->Data.ParamDW.Param[PARAM_GPS_NODATA_TO] = 3;
		Param->Data.ParamDW.Param[PARAM_GPS_V_TO] = 180;
		Param->Data.ParamDW.Param[PARAM_GPS_KEEP_TO] = 120;
		Param->Data.ParamDW.Param[PARAM_GPS_SLEEP_TO] = 180;//为0表示GPS不自动唤醒
		Param->Data.ParamDW.Param[PARAM_AGPS_EN] = 1;
		Param->Data.ParamDW.Param[PARAM_GPS_ONLY_ONCE] = 0;
#elif (__CUST_CODE__ == __CUST_LY__)
		Param->Data.ParamDW.Param[PARAM_GS_WAKEUP_GPS] = 10;
		Param->Data.ParamDW.Param[PARAM_VACC_WAKEUP_GPS] = 1;
		Param->Data.ParamDW.Param[PARAM_GPS_NODATA_TO] = 3;
		Param->Data.ParamDW.Param[PARAM_GPS_V_TO] = 180;
		Param->Data.ParamDW.Param[PARAM_GPS_KEEP_TO] = 120;
		Param->Data.ParamDW.Param[PARAM_GPS_SLEEP_TO] = 0;//为0表示GPS不自动唤醒
		Param->Data.ParamDW.Param[PARAM_AGPS_EN] = 1;
		Param->Data.ParamDW.Param[PARAM_GPS_ONLY_ONCE] = 0;
#elif (__CUST_CODE__ == __CUST_GLEAD__)
		Param->Data.ParamDW.Param[PARAM_GS_WAKEUP_GPS] = 10;
		Param->Data.ParamDW.Param[PARAM_VACC_WAKEUP_GPS] = 1;
		Param->Data.ParamDW.Param[PARAM_GPS_NODATA_TO] = 3;
		Param->Data.ParamDW.Param[PARAM_GPS_V_TO] = 180;
		Param->Data.ParamDW.Param[PARAM_GPS_KEEP_TO] = 120;
		Param->Data.ParamDW.Param[PARAM_GPS_SLEEP_TO] = 3600;//为0表示GPS不自动唤醒
		Param->Data.ParamDW.Param[PARAM_AGPS_EN] = 1;
		Param->Data.ParamDW.Param[PARAM_GPS_ONLY_ONCE] = 0;
#elif (__CUST_CODE__ == __CUST_LB__)
		Param->Data.ParamDW.Param[PARAM_GS_WAKEUP_GPS] = 10;
		Param->Data.ParamDW.Param[PARAM_VACC_WAKEUP_GPS] = 0;
		Param->Data.ParamDW.Param[PARAM_GPS_NODATA_TO] = 3;
		Param->Data.ParamDW.Param[PARAM_GPS_V_TO] = 180;
		Param->Data.ParamDW.Param[PARAM_GPS_KEEP_TO] = 120;
		Param->Data.ParamDW.Param[PARAM_GPS_SLEEP_TO] = 0;//为0表示GPS不自动唤醒
		Param->Data.ParamDW.Param[PARAM_AGPS_EN] = 1;
		Param->Data.ParamDW.Param[PARAM_GPS_ONLY_ONCE] = 0;
#elif (__CUST_CODE__ == __CUST_LY_IOTDEV__)
		Param->Data.ParamDW.Param[PARAM_GS_WAKEUP_GPS] = 0;
		Param->Data.ParamDW.Param[PARAM_VACC_WAKEUP_GPS] = 0;
		Param->Data.ParamDW.Param[PARAM_GPS_NODATA_TO] = 5;
		Param->Data.ParamDW.Param[PARAM_GPS_V_TO] = 180;
		Param->Data.ParamDW.Param[PARAM_GPS_KEEP_TO] = 1;
		Param->Data.ParamDW.Param[PARAM_GPS_SLEEP_TO] = 0;//为0表示GPS不自动唤醒
		Param->Data.ParamDW.Param[PARAM_AGPS_EN] = 0;
		Param->Data.ParamDW.Param[PARAM_GPS_ONLY_ONCE] = 0;
#endif

#ifdef __MINI_SYSTEM__
		Param->Data.ParamDW.Param[PARAM_GPS_KEEP_TO] = 1;
#endif
		Param->CRC32 = __CRC32((u8 *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}

	if (!Param_Load(PARAM_TYPE_MONITOR, Buf))
	{
		DBG("%d no data", PARAM_TYPE_MONITOR);
		Param = &gSys.nParam[PARAM_TYPE_MONITOR];
		memset(Param, 0, sizeof(Param_Byte64Struct));
#if (__CUST_CODE__ == __CUST_KQ__)
		Param->Data.ParamDW.Param[PARAM_GS_WAKEUP_MONITOR] = 0;//为0表示VACC唤醒
		Param->Data.ParamDW.Param[PARAM_GS_JUDGE_RUN] = 0;//不为0则表示在不骑行的时候，降低发送频率
		Param->Data.ParamDW.Param[PARAM_UPLOAD_RUN_PERIOD] = 60;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_STOP_PERIOD] = 300;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_HEART_PERIOD] = 50;
		Param->Data.ParamDW.Param[PARAM_MONITOR_NET_TO] = 65;//系统TCP超时62秒，所有设置为65秒
		Param->Data.ParamDW.Param[PARAM_MONITOR_KEEP_TO] = 0;//为0表示永远在线
		Param->Data.ParamDW.Param[PARAM_MONITOR_SLEEP_TO] = 0;//为0表示休眠期间，不周期性启动发送数据
		Param->Data.ParamDW.Param[PARAM_MONITOR_RECONNECT_MAX] = 8;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ADD_MILEAGE] = 0;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ACC_UPLOAD] = 0;
#elif (__CUST_CODE__ == __CUST_LY__)
		Param->Data.ParamDW.Param[PARAM_GS_WAKEUP_MONITOR] = 0;//为0表示VACC唤醒
		Param->Data.ParamDW.Param[PARAM_GS_JUDGE_RUN] = 10;//不为0则表示在不骑行的时候，降低发送频率
		Param->Data.ParamDW.Param[PARAM_UPLOAD_RUN_PERIOD] = 30;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_STOP_PERIOD] = 600;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_HEART_PERIOD] = 50;
		Param->Data.ParamDW.Param[PARAM_MONITOR_NET_TO] = 65;//系统TCP超时62秒，所有设置为65秒
		Param->Data.ParamDW.Param[PARAM_MONITOR_KEEP_TO] = 0;//为0表示永远在线
		Param->Data.ParamDW.Param[PARAM_MONITOR_SLEEP_TO] = 0;//为0表示休眠期间，不周期性启动发送数据
		Param->Data.ParamDW.Param[PARAM_MONITOR_RECONNECT_MAX] = 4;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ADD_MILEAGE] = 1;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ACC_UPLOAD] = 1;
#elif (__CUST_CODE__ == __CUST_GLEAD__)
		Param->Data.ParamDW.Param[PARAM_GS_WAKEUP_MONITOR] = 0;//为0表示VACC唤醒
		Param->Data.ParamDW.Param[PARAM_GS_JUDGE_RUN] = 10;//不为0则表示在不骑行的时候，降低发送频率
		Param->Data.ParamDW.Param[PARAM_UPLOAD_RUN_PERIOD] = 30;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_STOP_PERIOD] = 99999999;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_HEART_PERIOD] = 120;
		Param->Data.ParamDW.Param[PARAM_MONITOR_NET_TO] = 65;//系统TCP超时62秒，所有设置为65秒
		Param->Data.ParamDW.Param[PARAM_MONITOR_KEEP_TO] = 0;//为0表示永远在线
		Param->Data.ParamDW.Param[PARAM_MONITOR_SLEEP_TO] = 3600 * 6;//为0表示休眠期间，不周期性启动发送数据
		Param->Data.ParamDW.Param[PARAM_MONITOR_RECONNECT_MAX] = 8;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ADD_MILEAGE] = 1;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ACC_UPLOAD] = 1;
#elif (__CUST_CODE__ == __CUST_LB__)
		Param->Data.ParamDW.Param[PARAM_GS_WAKEUP_MONITOR] = 0;//为0表示VACC唤醒
		Param->Data.ParamDW.Param[PARAM_GS_JUDGE_RUN] = 10;//不为0则表示在不骑行的时候，降低发送频率
		Param->Data.ParamDW.Param[PARAM_UPLOAD_RUN_PERIOD] = 20;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_STOP_PERIOD] = 99999999;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_HEART_PERIOD] = 180;
		Param->Data.ParamDW.Param[PARAM_MONITOR_NET_TO] = 65;//系统TCP超时62秒，所有设置为65秒
		Param->Data.ParamDW.Param[PARAM_MONITOR_KEEP_TO] = 0;//为0表示永远在线
		Param->Data.ParamDW.Param[PARAM_MONITOR_SLEEP_TO] = 0;//为0表示休眠期间，不周期性启动发送数据
		Param->Data.ParamDW.Param[PARAM_MONITOR_RECONNECT_MAX] = 8;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ADD_MILEAGE] = 1;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ACC_UPLOAD] = 1;
#elif (__CUST_CODE__ == __CUST_LY_IOTDEV__)
		Param->Data.ParamDW.Param[PARAM_GS_WAKEUP_MONITOR] = 0;//为0表示VACC唤醒
		Param->Data.ParamDW.Param[PARAM_GS_JUDGE_RUN] = 10;//不为0则表示在不骑行的时候，降低发送频率
		Param->Data.ParamDW.Param[PARAM_UPLOAD_RUN_PERIOD] = 30;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_STOP_PERIOD] = 600;
		Param->Data.ParamDW.Param[PARAM_UPLOAD_HEART_PERIOD] = 50;
		Param->Data.ParamDW.Param[PARAM_MONITOR_NET_TO] = 65;//系统TCP超时62秒，所有设置为65秒
		Param->Data.ParamDW.Param[PARAM_MONITOR_KEEP_TO] = 0;//为0表示永远在线
		Param->Data.ParamDW.Param[PARAM_MONITOR_SLEEP_TO] = 0;//为0表示休眠期间，不周期性启动发送数据
		Param->Data.ParamDW.Param[PARAM_MONITOR_RECONNECT_MAX] = 4;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ADD_MILEAGE] = 1;
		Param->Data.ParamDW.Param[PARAM_MONITOR_ACC_UPLOAD] = 1;
#endif
		Param->CRC32 = __CRC32((u8 *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}

	if (!Param_Load(PARAM_TYPE_ALARM1, Buf))
	{
		DBG("%d no data", PARAM_TYPE_ALARM1);
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
		Param->Data.ParamDW.Param[PARAM_MOVE_RANGE] = 99999999;
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
		Param->CRC32 = __CRC32((u8 *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}

	if (!Param_Load(PARAM_TYPE_ALARM2, Buf))
	{
		DBG("%d no data", PARAM_TYPE_ALARM2);
		Param = &gSys.nParam[PARAM_TYPE_ALARM2];
		memset(Param, 0, sizeof(Param_Byte64Struct));
#if (__CUST_CODE__ == __CUST_LY__)
		Param->Data.ParamDW.Param[PARAM_VACC_WAKEUP_CUTLINE_TO] = 1;
		Param->Data.ParamDW.Param[PARAM_CUTLINE_ALARM_DELAY] = 5;
		Param->Data.ParamDW.Param[PARAM_CUTLINE_ALARM_FLUSH_TO] = 0;
		Param->Data.ParamDW.Param[PARAM_OVERSPEED_ALARM_VAL] = 0;
		Param->Data.ParamDW.Param[PARAM_OVERSPEED_ALARM_DELAY] = 0;
		Param->Data.ParamDW.Param[PARAM_ALARM_ENABLE] = 1;
#elif (__CUST_CODE__ == __CUST_LY_IOTDEV__)
		Param->Data.ParamDW.Param[PARAM_VACC_WAKEUP_CUTLINE_TO] = 1;
		Param->Data.ParamDW.Param[PARAM_CUTLINE_ALARM_DELAY] = 5;
		Param->Data.ParamDW.Param[PARAM_CUTLINE_ALARM_FLUSH_TO] = 0;
		Param->Data.ParamDW.Param[PARAM_OVERSPEED_ALARM_VAL] = 0;
		Param->Data.ParamDW.Param[PARAM_OVERSPEED_ALARM_DELAY] = 0;
		Param->Data.ParamDW.Param[PARAM_ALARM_ENABLE] = 1;
#else
		Param->Data.ParamDW.Param[PARAM_VACC_WAKEUP_CUTLINE_TO] = 0;
		Param->Data.ParamDW.Param[PARAM_CUTLINE_ALARM_DELAY] = 2;
		Param->Data.ParamDW.Param[PARAM_CUTLINE_ALARM_FLUSH_TO] = 0;
		Param->Data.ParamDW.Param[PARAM_OVERSPEED_ALARM_VAL] = 0;
		Param->Data.ParamDW.Param[PARAM_OVERSPEED_ALARM_DELAY] = 0;
		Param->Data.ParamDW.Param[PARAM_ALARM_ENABLE] = 1;
#endif
		Param->CRC32 = __CRC32((u8 *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}

	if (!Param_Load(PARAM_TYPE_APN, Buf))
	{
		DBG("%d no data", PARAM_TYPE_APN);
		Param = &gSys.nParam[PARAM_TYPE_APN];
		memset(Param, 0, sizeof(Param_Byte64Struct));
		Param->CRC32 = __CRC32((u8 *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}


	Param = &gSys.nParam[PARAM_TYPE_LOCAT];
	if (Param->CRC32 != __CRC32((u8 *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START))
	{
		if (!Param_Load(PARAM_TYPE_LOCAT, Buf))
		{
			DBG("%d no data", PARAM_TYPE_LOCAT);
			memset(Param, 0, sizeof(Param_Byte64Struct));
			Param->Data.LocatInfo.RMCSave.LatDegree = __CUST_LAT_DEGREE__;
			Param->Data.LocatInfo.RMCSave.LatMin = __CUST_LAT_MIN__;
			Param->Data.LocatInfo.RMCSave.LatNS = __CUST_LAT_NS__;
			Param->Data.LocatInfo.RMCSave.LgtDegree = __CUST_LGT_DEGREE__;
			Param->Data.LocatInfo.RMCSave.LgtMin = __CUST_LGT_MIN__;
			Param->Data.LocatInfo.RMCSave.LgtEW = __CUST_LGT_EW__;
			Param->CRC32 = __CRC32((u8 *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
		}
	}

	gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LocatStatus = 0;
	DBG("%d %d %d %d, %d, %d", gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LatDegree,
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LatMin,
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LgtDegree,
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave.LgtMin,
			gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageKM, gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageM);

	if (!Param_Load(PARAM_TYPE_UPGRADE, Buf))
	{
		DBG("%d no data", PARAM_TYPE_UPGRADE);
		Param = &gSys.nParam[PARAM_TYPE_UPGRADE];
		memset(Param, 0, sizeof(Param_Byte64Struct));
		Param->CRC32 = __CRC32((u8 *)&Param->Data, sizeof(Param_Byte60Union), CRC32_START);
	}

	if (!Param_Load(PARAM_TYPE_NUMBER, Buf))
	{
		DBG("%d no data, init", PARAM_TYPE_NUMBER);
		Param = &gSys.nParam[PARAM_TYPE_NUMBER];
		memset(Param, 0, sizeof(Param_Byte64Struct));
	}
	Param = &gSys.nParam[PARAM_TYPE_NUMBER];
	__HexTrace(Param->Data.Number.Phone[0].Num, 8);

	if (!Param_Load(PARAM_TYPE_USER, Buf))
	{
		DBG("%d no data", PARAM_TYPE_USER);
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

s32 Param_Save(u8 Type)
{
	u32 Addr1 = PARAM_PID_ADDR + Type * FLASH_SECTOR_LEN * 2;
	u32 Addr2 = Addr1 + FLASH_SECTOR_LEN;
	u32 i, Pos1, Pos2;
	u8 *Buf = (u8 *)gSys.FlashBuf;
	gSys.nParam[Type].CRC32 = __CRC32((u8 *)&gSys.nParam[Type].Data, sizeof(Param_Byte60Union), CRC32_START);
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
			//DBG("Param %d save in flash1 %d", Type, i);
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
				//DBG("Param %d save in flash2 %d", Type, i);
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
		DBG("Param %d save flash no free rom, please reboot", Type);
		return -1;
	}
	DBG("%d %d %d", Type, Pos1, Pos2);
	Addr1 = Addr1 + (Pos1 - 1) * sizeof(Param_Byte64Struct);
	__WriteFlash(Addr1, (u8 *)&gSys.nParam[Type], sizeof(Param_Byte64Struct));
	if (Type == PARAM_TYPE_MAIN)
	{
		Addr2 = Addr2 + (Pos2 - 1) * sizeof(Param_Byte64Struct);
		__WriteFlash(Addr2, (u8 *)&gSys.nParam[Type], sizeof(Param_Byte64Struct));
	}
	return 0;
}

s32 Param_Format(u8 Type)
{
	u32 Addr1 = PARAM_PID_ADDR + Type * FLASH_SECTOR_LEN * 2;
	u32 Addr2 = Addr1 + FLASH_SECTOR_LEN;
	u32 i, Pos1, Pos2;
	u8 *Buf = (u8 *)gSys.FlashBuf;
	gSys.nParam[Type].CRC32 = __CRC32((u8 *)&gSys.nParam[Type].Data, sizeof(Param_Byte60Union), CRC32_START) + 1;
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
			//DBG("Param %d save in flash1 %d", Type, i);
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
				//DBG("Param %d save in flash2 %d", Type, i);
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
		DBG("Param %d save flash no free rom, please reboot", Type);
		return -1;
	}
	DBG("%d %d", Pos1, Pos2);

	Addr1 = Addr1 + (Pos1 - 1) * sizeof(Param_Byte64Struct);
	__WriteFlash(Addr1, (u8 *)&gSys.nParam[Type], sizeof(Param_Byte64Struct));
	if (Type == PARAM_TYPE_MAIN)
	{
		Addr2 = Addr2 + (Pos2 - 1) * sizeof(Param_Byte64Struct);
		__WriteFlash(Addr2, (u8 *)&gSys.nParam[Type], sizeof(Param_Byte64Struct));
	}
	return 0;
}

void Locat_CacheSave(void)
{
	gSys.nParam[PARAM_TYPE_LOCAT].CRC32 = __CRC32((u8 *)&gSys.nParam[PARAM_TYPE_LOCAT].Data, sizeof(Param_Byte60Union), CRC32_START);
}
