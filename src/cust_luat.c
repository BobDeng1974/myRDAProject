#include "user.h"
#define LUAT_LBS_URL		"bs.openluat.com"
#define LUAT_LBS_PORT		(12411)
#define LUAT_LBS_REQ_LOCAT	(0x02)
#define LUAT_UPGRADE_URL	"firmware.openluat.com"
#define LUAT_UPGRADE_PORT	(12410)
typedef struct
{
	Net_CtrlStruct Net;
	LBS_LocatInfoStruct LBSLocat;
	uint32_t UpgradeID;
	uint16_t UpgradeLastLen;
	uint16_t UpgradeRxLen;
	uint16_t UpgradeIndexMax;
	uint8_t UpgradeState;

	uint8_t StartLBS;
	uint8_t LBSFinish;
	uint8_t LBSOK;
	uint8_t RepeatLBS;
	uint8_t TempBuf[MONITOR_TXBUF_LEN];
	uint8_t RxBuf[MONITOR_TXBUF_LEN];
	uint8_t IMEI[8];
	uint8_t IMEIStr[16];
}LUAT_CtrlStruct;

#define LUAT_PRODUCTKEY "V0iboxbLGfHJgqjFVLgMZ4AamkOxSslK"
#define LUAT_APIKEY "O65uoZPlLK6FcSmU"
#define LUAT_APISECRET "1VtpEOQ1HDPHKaoCDkdA7HBfwq9ghpE10RRESUsmf6xoeiBA3gl0HXmvimJzwilP"

LUAT_CtrlStruct __attribute__((section (".usr_ram"))) LUATCtrl;


int32_t LUAT_ReceiveAnalyze(void *pData)
{
	int32_t Error;
	CFW_TCPIP_SOCKET_ADDR From;
	INT32 FromLen;
	int i;
	uint32_t RxLen = (uint32_t)pData;
	memset(&From, 0, sizeof(From));
	DBG("%u", RxLen);
	if (RxLen > 1200)
	{
		DBG("too much data");
		do
		{
			Error = (int32_t)OS_SocketReceive(LUATCtrl.Net.SocketID, LUATCtrl.RxBuf, 1024, &From, &FromLen);

		}while(Error > 0);

	}
	else
	{
		DBG("%d %u", LUATCtrl.Net.SocketID, RxLen);
		Error = (int32_t)OS_SocketReceive(LUATCtrl.Net.SocketID, LUATCtrl.RxBuf, RxLen, &From, &FromLen);
		if (Error < 0)
		{
			LUATCtrl.UpgradeRxLen = 0;
			return -1;
		}
		else
		{
			RxLen = Error;
		}
		DBG("%d", Error);
		if (LUATCtrl.UpgradeState)
		{
			LUATCtrl.UpgradeRxLen = RxLen;
		}
		else
		{
			if (LUATCtrl.RxBuf[0])
			{
				DBG("fail %02x", LUATCtrl.RxBuf[0]);
			}
			else
			{
				LUATCtrl.RxBuf[RxLen] = 0;
				ReverseBCD(LUATCtrl.RxBuf + 1, LUATCtrl.RxBuf + 1, 10);
				if ( (LUATCtrl.RxBuf[1] == 0xff) || (LUATCtrl.RxBuf[6] == 0xff) )
				{
					HexTrace(LUATCtrl.RxBuf, 11);
				}
				else
				{
					for (i = 1; i <= 10; i++)
					{
						if ( ((LUATCtrl.RxBuf[i] & 0xf0) >> 4) >= 10 )
						{
							LUATCtrl.RxBuf[i] &= 0x0f;
						}

						if ( ((LUATCtrl.RxBuf[i] & 0x0f) >> 0) >= 10 )
						{
							LUATCtrl.RxBuf[i] &= 0xf0;
						}
					}
					LUATCtrl.LBSLocat.Lat = BCDToInt(LUATCtrl.RxBuf + 1, 5);
					LUATCtrl.LBSLocat.Lgt = BCDToInt(LUATCtrl.RxBuf + 6, 5);
					LUATCtrl.LBSOK = 1;
				}
			}
			LUATCtrl.LBSFinish = 1;
		}

	}


	return 0;
}

void LUAT_LBSTx(void)
{
	uint32_t Pos = 0;

	uint16_t MCC = BCDToInt(gSys.IMSI, 2);
	uint8_t MNC = BCDToInt(gSys.IMSI + 2, 1);
	uint8_t i,j;
	if (MNC == 4)
	{
		MNC = 0;
	}

	MCC = htons(MCC);
	LUATCtrl.TempBuf[Pos++] = strlen(LUAT_PRODUCTKEY);
	memcpy(LUATCtrl.TempBuf + Pos, LUAT_PRODUCTKEY, LUATCtrl.TempBuf[0]);
	Pos += LUATCtrl.TempBuf[0];
	LUATCtrl.TempBuf[Pos++] = 0;
	memcpy(LUATCtrl.TempBuf + Pos, LUATCtrl.IMEI, 8);
	Pos += 8;
	LUATCtrl.TempBuf[Pos++] = gSys.LBSInfo.LACNum;
	for(i = 0; i < gSys.LBSInfo.LACNum; i++)
	{
		memcpy(&LUATCtrl.TempBuf[Pos], gSys.LBSInfo.LAC[i].LAC_BE, 2);
		Pos += 2;
		memcpy(&LUATCtrl.TempBuf[Pos], &MCC, 2);
		Pos += 2;
		LUATCtrl.TempBuf[Pos++] = MNC;
		for (j = 0; j < gSys.LBSInfo.LAC[i].CINum; j++)
		{
			memcpy(&LUATCtrl.TempBuf[Pos], &gSys.LBSInfo.LAC[i].CI[j], 3);
			if (!j)
			{
				LUATCtrl.TempBuf[Pos] |= (gSys.LBSInfo.LAC[i].CINum - 1) << 5;
			}
			Pos += 3;
		}
	}
	//DBG("%d", Pos);
	//__HexTrace(LUATCtrl.TempBuf, Pos);
	Net_Send(&LUATCtrl.Net, LUATCtrl.TempBuf, Pos);
}

int32_t LUAT_UpgradeTx(uint8_t nIndex, uint32_t ID)
{
	int8_t IMEIStr[16];
	int32_t TxLen;
	if (!LUATCtrl.IMEI[0])
	{
		snprintf(IMEIStr, 16, "%01x%02x%02x%02x%02x%02x%02x%02x", gSys.IMEI[0], gSys.IMEI[1],
				gSys.IMEI[2], gSys.IMEI[3],gSys.IMEI[4], gSys.IMEI[5],gSys.IMEI[6], gSys.IMEI[7]);
		AsciiToGsmBcd(IMEIStr, 15, LUATCtrl.IMEI);
		HexTrace(LUATCtrl.IMEI, 8);
	}
	if (!nIndex)
	{
		TxLen = sprintf(LUATCtrl.TempBuf, "0,%s,%s,,%08x,%u.0.%u",
				LUAT_PRODUCTKEY, LUATCtrl.IMEIStr, __GetMainVersion(),gSys.Var[SOFTWARE_VERSION]/1000000, gSys.Var[SOFTWARE_VERSION]%1000000);
	}
	else
	{
		TxLen = sprintf(LUATCtrl.TempBuf, "0,%s,%s,,%08x,%u.0.%u,Get%u,%u",
				LUAT_PRODUCTKEY, LUATCtrl.IMEIStr, __GetMainVersion(),gSys.Var[SOFTWARE_VERSION]/1000000, gSys.Var[SOFTWARE_VERSION]%1000000,
				nIndex, ID);
	}

	if (TxLen > 0)
	{
		DBG("%d,%s",TxLen, LUATCtrl.TempBuf);
		Net_Send(&LUATCtrl.Net, LUATCtrl.TempBuf, TxLen);
	}
	else
	{
		DBG("!");
	}
	return TxLen;
}

void LUAT_Upgrade(void)
{
	uint32_t FileLen;
	int32_t Error = 0;
	uint16_t Retry, nIndex, RxIndex;
	int8_t Buf[4][32];
	CmdParam CP;
	memset(&CP, 0, sizeof(CP));
	memset(Buf, 0, sizeof(Buf));
	CP.param_max_len = 32;
	CP.param_max_num = 4;
	CP.param_str = (int8_t *)Buf;

	LUATCtrl.UpgradeState = 1;
	LUATCtrl.UpgradeID = 0xffffffff;
	Error = 1;
	for (Retry = 0; Retry < 3; Retry++)
	{
		LUAT_UpgradeTx(0,0);
		LUATCtrl.UpgradeRxLen = 0;
		LUATCtrl.Net.To = 30;
		Net_WaitReceive(&LUATCtrl.Net);
		if (LUATCtrl.Net.Result == NET_RES_UPLOAD)
		{
			if (LUATCtrl.UpgradeRxLen < 256)
			{
				DBG("%d",LUATCtrl.UpgradeRxLen);
				LUATCtrl.RxBuf[LUATCtrl.UpgradeRxLen] = 0;
				CmdParseParam(LUATCtrl.RxBuf, &CP, ',');
				if (!strcmp("LUAUPDATE", Buf[0]))
				{
					LUATCtrl.UpgradeID = strtol(Buf[1], NULL, 10);
					LUATCtrl.UpgradeIndexMax = strtol(Buf[2], NULL, 10);
					LUATCtrl.UpgradeLastLen = strtol(Buf[3], NULL, 10);
					DBG("%d,%d,%d", LUATCtrl.UpgradeID, LUATCtrl.UpgradeIndexMax, LUATCtrl.UpgradeLastLen);
					Error = 0;
					break;
				}
				else
				{
					DBG("no use response");
					break;
				}
			}
		}
	}
	if (Error)
	{
		DBG("!");
		return ;
	}

	if (1022 == LUATCtrl.UpgradeLastLen)
	{
		FileLen = LUATCtrl.UpgradeIndexMax * 1022;

	}
	else
	{
		FileLen = (LUATCtrl.UpgradeIndexMax - 1) * 1022 + LUATCtrl.UpgradeLastLen;
	}
	DBG("%u", FileLen);
	__FileSet(FileLen);
	nIndex = 1;
	while (nIndex <= LUATCtrl.UpgradeIndexMax)
	{
		Error = 1;
		for (Retry = 0; Retry < 3; Retry++)
		{
			LUAT_UpgradeTx(nIndex, LUATCtrl.UpgradeID);
			LUATCtrl.UpgradeRxLen = 0;
			LUATCtrl.Net.To = 30;
			Net_WaitReceive(&LUATCtrl.Net);
			if (LUATCtrl.Net.Result == NET_RES_UPLOAD)
			{
				if (nIndex == LUATCtrl.UpgradeIndexMax)
				{
					if (LUATCtrl.UpgradeRxLen != (LUATCtrl.UpgradeLastLen + 2))
					{
						DBG("%u", LUATCtrl.UpgradeRxLen);
						continue;
					}
				}
				else
				{
					if (LUATCtrl.UpgradeRxLen != 1024)
					{
						DBG("%u", LUATCtrl.UpgradeRxLen);
						continue;
					}
				}
				RxIndex = LUATCtrl.RxBuf[0];
				RxIndex = RxIndex * 256 + LUATCtrl.RxBuf[1];

				if (nIndex == RxIndex)
				{
					Error = 0;
					__WriteFile(LUATCtrl.RxBuf + 2, LUATCtrl.UpgradeRxLen - 2);
					break;
				}
				else
				{
					continue;
				}
			}
		}
		if (Error)
		{
			DBG("!");
			return ;
		}
		nIndex++;
	}
	if (__UpgradeVaildCheck())
	{
		DBG("file ok!");
		SYS_Reset();
	}
	else
	{
		DBG("file error!");
	}
	return;
}

void LUAT_Task(void *pData)
{
	IP_AddrUnion uIP;
	uint8_t Retry;
	uint32_t UpgradeTime = 0;
	if (!gSys.IMEI[0] || (gSys.IMEI[0] == 0x03))
	{
		//goto LUAT_LBS_FINISH;
		LUATCtrl.IMEI[0] = 0x68;
		LUATCtrl.IMEI[1] = 0x92;
		LUATCtrl.IMEI[2] = 0x19;
		LUATCtrl.IMEI[3] = 0x52;
		LUATCtrl.IMEI[4] = 0x28;
		LUATCtrl.IMEI[5] = 0x54;
		LUATCtrl.IMEI[6] = 0x79;
		LUATCtrl.IMEI[7] = 0xf7;
		strcpy(LUATCtrl.IMEIStr, "862991258245977");
	}
	else
	{
		snprintf(LUATCtrl.IMEIStr, 16, "%01x%02x%02x%02x%02x%02x%02x%02x", gSys.IMEI[0], gSys.IMEI[1],
						gSys.IMEI[2], gSys.IMEI[3],gSys.IMEI[4], gSys.IMEI[5],gSys.IMEI[6], gSys.IMEI[7]);
		AsciiToGsmBcd(LUATCtrl.IMEIStr, 15, LUATCtrl.IMEI);

	}
	HexTrace(LUATCtrl.IMEI, 8);
	if (!gSys.RMCInfo->LatDegree || !gSys.RMCInfo->LgtDegree)
	{
		LUATCtrl.StartLBS = 1;
	}
	while(1)
	{
		if (!UpgradeTime || (gSys.Var[SYS_TIME] > UpgradeTime))
		{
			if (LUATCtrl.Net.SocketID != INVALID_SOCKET)
			{
				LUATCtrl.Net.To = 15;
				Net_Disconnect(&LUATCtrl.Net);
			}
			LUATCtrl.Net.TCPPort = 0;
			LUATCtrl.Net.UDPPort = LUAT_UPGRADE_PORT;
			LUATCtrl.Net.To = 70;
			Net_Connect(&LUATCtrl.Net, 0, LUAT_UPGRADE_URL);
			if (LUATCtrl.Net.Result == NET_RES_CONNECT_OK)
			{
				uIP.u32_addr = LUATCtrl.Net.IPAddr.s_addr;
				DBG("IP %u.%u.%u.%u OK", (uint32_t)uIP.u8_addr[0], (uint32_t)uIP.u8_addr[1],
						(uint32_t)uIP.u8_addr[2], (uint32_t)uIP.u8_addr[3]);
				LUAT_Upgrade();
			}
			else
			{
				DBG("luat upgrade fail!");
			}
		}
		UpgradeTime = gSys.Var[SYS_TIME] + 24 * 3600;
		LUATCtrl.UpgradeState = 0;
		if (LUATCtrl.StartLBS)
		{
			LUATCtrl.StartLBS = 0;
			LUATCtrl.LBSFinish = 0;
			LUATCtrl.LBSOK = 0;

			if (LUATCtrl.Net.SocketID != INVALID_SOCKET)
			{
				LUATCtrl.Net.To = 15;
				Net_Disconnect(&LUATCtrl.Net);
			}
			LUATCtrl.Net.TCPPort = 0;
			LUATCtrl.Net.UDPPort = LUAT_LBS_PORT;
			LUATCtrl.Net.To = 70;
			Net_Connect(&LUATCtrl.Net, 0, LUAT_LBS_URL);
			if (LUATCtrl.Net.Result == NET_RES_CONNECT_OK)
			{
				uIP.u32_addr = LUATCtrl.Net.IPAddr.s_addr;
				DBG("IP %u.%u.%u.%u OK", (uint32_t)uIP.u8_addr[0], (uint32_t)uIP.u8_addr[1],
						(uint32_t)uIP.u8_addr[2], (uint32_t)uIP.u8_addr[3]);

				for (Retry = 0; Retry < 3; Retry++)
				{
					LUATCtrl.LBSLocat.uDate.dwDate = gSys.Var[UTC_DATE];
					LUATCtrl.LBSLocat.uTime.dwTime = gSys.Var[UTC_TIME];
					LUAT_LBSTx();
					LUATCtrl.Net.To = 15;
					Net_WaitReceive(&LUATCtrl.Net);
					if (LUATCtrl.LBSFinish)
					{
						break;
					}
				}
			}
			else
			{
				goto LUAT_LBS_FINISH;
			}
		}

		if (LUATCtrl.LBSOK)
		{
			gSys.LBSLocat = LUATCtrl.LBSLocat;
			DBG("%d-%d-%d %d:%d:%d %u.%u %u.%u", gSys.LBSLocat.uDate.Date.Year,
					gSys.LBSLocat.uDate.Date.Mon, gSys.LBSLocat.uDate.Date.Day,
					gSys.LBSLocat.uTime.Time.Hour, gSys.LBSLocat.uTime.Time.Min,
					gSys.LBSLocat.uTime.Time.Sec, gSys.LBSLocat.Lat / 10000000,
					(gSys.LBSLocat.Lat % 10000000) / 100, gSys.LBSLocat.Lgt / 10000000,
					(gSys.LBSLocat.Lgt % 10000000) / 100);
#ifdef __LBS_AUTO__
			if (!gSys.RMCInfo->LatDegree || !gSys.RMCInfo->LgtDegree || gSys.Error[NO_LOCAT_ERROR])
#else
			if (!gSys.RMCInfo->LatDegree || !gSys.RMCInfo->LgtDegree )
#endif
			{
				gSys.RMCInfo->LatDegree = gSys.LBSLocat.Lat / 10000000;
				gSys.RMCInfo->LatMin = ( (gSys.LBSLocat.Lat % 10000000) * 60 ) / 1000;
				gSys.RMCInfo->LgtDegree = gSys.LBSLocat.Lgt / 10000000;
				gSys.RMCInfo->LgtMin = ( (gSys.LBSLocat.Lgt % 10000000) * 60 ) / 1000;
				Locat_CacheSave();
			}

		}
LUAT_LBS_FINISH:
		if (LUATCtrl.Net.SocketID != INVALID_SOCKET)
		{
			LUATCtrl.Net.To = 15;
			Net_Disconnect(&LUATCtrl.Net);
		}
		if (LUATCtrl.RepeatLBS)
		{
			LUATCtrl.StartLBS = 1;
			LUATCtrl.Net.To = 5;
			Net_WaitSpecialEvent(&LUATCtrl.Net, EV_MMI_START_LBS);
		}
		else
		{
			LUATCtrl.Net.To = 3600 * 24;
			Net_WaitSpecialEvent(&LUATCtrl.Net, EV_MMI_START_LBS);
		}

	}
}

void LUAT_StartLBS(uint8_t IsRepeat)
{
	LUATCtrl.StartLBS = 1;
	LUATCtrl.RepeatLBS = IsRepeat;
	OS_SendEvent(gSys.TaskID[LUAT_TASK_ID], EV_MMI_START_LBS, 0, 0, 0);
}

void LUAT_Config(void)
{
	gSys.TaskID[LUAT_TASK_ID] = COS_CreateTask(LUAT_Task, NULL,
					NULL, MMI_TASK_MIN_STACK_SIZE , MMI_TASK_PRIORITY + LUAT_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI Luat Task");
	LUATCtrl.Net.SocketID = INVALID_SOCKET;
	LUATCtrl.Net.TaskID = gSys.TaskID[LUAT_TASK_ID];
	LUATCtrl.Net.Channel = GPRS_CH_LUAT;
	LUATCtrl.Net.TimerID = LUAT_TIMER_ID;
	LUATCtrl.Net.ReceiveFun = LUAT_ReceiveAnalyze;
	LUATCtrl.Net.LocalPort = UDP_LUAT_PORT;
}
