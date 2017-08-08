#include "user.h"
#define LUAT_LBS_URL		"bs.openluat.com"
#define LUAT_LBS_PORT		(12411)
#define LUAT_LBS_REQ_LOCAT	(0x02)
typedef struct
{
	Net_CtrlStruct Net;
	LBS_LocatInfoStruct LBSLocat;
	uint8_t StartLBS;
	uint8_t LBSFinish;
	uint8_t LBSOK;
	uint8_t RepeatLBS;
	uint8_t TempBuf[MONITOR_TXBUF_LEN];
	uint8_t IMEI[8];
}LUAT_CtrlStruct;

#define LUAT_PRODUCTKEY "V0iboxbLGfHJgqjFVLgMZ4AamkOxSslK"
#define LUAT_APIKEY "O65uoZPlLK6FcSmU"
#define LUAT_APISECRET "1VtpEOQ1HDPHKaoCDkdA7HBfwq9ghpE10RRESUsmf6xoeiBA3gl0HXmvimJzwilP"

LUAT_CtrlStruct __attribute__((section (".usr_ram"))) LUATCtrl;


int32_t LUAT_ReceiveAnalyze(void *pData)
{
	uint8_t *Temp;
	CFW_TCPIP_SOCKET_ADDR From;
	INT32 FromLen;
	int i;
	uint32_t RxLen = (uint32_t)pData;
	DBG("%u", RxLen);
	Temp = COS_MALLOC(1024);
	if (RxLen > 266)
	{
		DBG("!");
		while (RxLen)
		{
			RxLen -= OS_SocketReceive(LUATCtrl.Net.SocketID, Temp, 1024, &From, &FromLen);
		}

	}
	else
	{
		OS_SocketReceive(LUATCtrl.Net.SocketID, Temp, RxLen, &From, &FromLen);
		if (Temp[0])
		{
			DBG("fail %02x", Temp[0]);
		}
		else
		{
			Temp[RxLen] = 0;
			ReverseBCD(Temp + 1, Temp + 1, 10);
			if ( (Temp[1] == 0xff) || (Temp[6] == 0xff) )
			{
				HexTrace(Temp, 11);
			}
			else
			{
				for (i = 1; i <= 10; i++)
				{
					if ( ((Temp[i] & 0xf0) >> 4) >= 10 )
					{
						Temp[i] &= 0x0f;
					}

					if ( ((Temp[i] & 0x0f) >> 0) >= 10 )
					{
						Temp[i] &= 0xf0;
					}
				}
				LUATCtrl.LBSLocat.Lat = BCDToInt(Temp + 1, 5);
				LUATCtrl.LBSLocat.Lgt = BCDToInt(Temp + 6, 5);
				LUATCtrl.LBSOK = 1;
			}

		}
	}
	COS_FREE(Temp);
	LUATCtrl.LBSFinish = 1;
	return 0;
}

void LUAT_LBSTx(void)
{
	uint32_t Pos = 0;
	int8_t IMEIStr[16];
	uint16_t MCC = BCDToInt(gSys.IMSI, 2);
	uint8_t MNC = BCDToInt(gSys.IMSI + 2, 1);
	uint8_t i,j;
	if (MNC == 4)
	{
		MNC = 0;
	}
	if (!LUATCtrl.IMEI[0])
	{
		snprintf(IMEIStr, 16, "%01x%02x%02x%02x%02x%02x%02x%02x", gSys.IMEI[0], gSys.IMEI[1],
				gSys.IMEI[2], gSys.IMEI[3],gSys.IMEI[4], gSys.IMEI[5],gSys.IMEI[6], gSys.IMEI[7]);
		AsciiToGsmBcd(IMEIStr, 15, LUATCtrl.IMEI);
		HexTrace(LUATCtrl.IMEI, 8);
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

void LUAT_Task(void *pData)
{
	IP_AddrUnion uIP;
	uint8_t Retry;
	LUATCtrl.StartLBS = 1;
	while(1)
	{
		if (LUATCtrl.StartLBS)
		{
			LUATCtrl.StartLBS = 0;
			if (!gSys.IMEI[0])
			{
				goto LUAT_LBS_FINISH;
			}

			LUATCtrl.LBSFinish = 0;
			LUATCtrl.LBSOK = 0;
			LUATCtrl.Net.LocalPort = UDP_LUAT_LBS_PORT;
			if (LUATCtrl.Net.SocketID != INVALID_SOCKET)
			{
				LUATCtrl.Net.To = 15;
				Net_Disconnect(&LUATCtrl.Net);
			}
			LUATCtrl.Net.TCPPort = 0;
			LUATCtrl.Net.UDPPort = 12411;
			LUATCtrl.Net.To = 70;
			Net_Connect(&LUATCtrl.Net, 0, LUAT_LBS_URL);
			if (LUATCtrl.Net.Result == NET_RES_CONNECT_OK)
			{
				uIP.u32_addr = LUATCtrl.Net.IPAddr.s_addr;
				DBG("IP %u.%u.%u.%u OK", (uint32_t)uIP.u8_addr[0], (uint32_t)uIP.u8_addr[1],
						(uint32_t)uIP.u8_addr[2], (uint32_t)uIP.u8_addr[3]);
			}
			for (Retry = 0; Retry < 3; Retry++)
			{
				LUATCtrl.LBSLocat.uDate.dwDate = gSys.Var[UTC_DATE];
				LUATCtrl.LBSLocat.uTime.dwTime = gSys.Var[UTC_TIME];
				LUAT_LBSTx();
				LUATCtrl.Net.To = 15;
				Net_WaitEvent(&LUATCtrl.Net);
				if (LUATCtrl.LBSFinish)
				{
					break;
				}
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
			if (!gSys.RMCInfo->LatDegree || !gSys.RMCInfo->LgtDegree || gSys.Error[NO_LOCAT_ERROR])
			{
				gSys.RMCInfo->LatDegree = gSys.LBSLocat.Lat / 10000000;
				gSys.RMCInfo->LatMin = (gSys.LBSLocat.Lat % 10000000) / 100;
				gSys.RMCInfo->LgtDegree = gSys.LBSLocat.Lgt / 10000000;
				gSys.RMCInfo->LgtDegree = (gSys.LBSLocat.Lgt % 10000000) / 100;
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
			LUATCtrl.Net.To = 3600;
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
					NULL, MMI_TASK_MAX_STACK_SIZE , MMI_TASK_PRIORITY + LUAT_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI Luat Task");
	LUATCtrl.Net.SocketID = INVALID_SOCKET;
	LUATCtrl.Net.TaskID = gSys.TaskID[LUAT_TASK_ID];
	LUATCtrl.Net.Channel = GPRS_CH_LUAT;
	LUATCtrl.Net.TimerID = LUAT_TIMER_ID;
	LUATCtrl.Net.ReceiveFun = LUAT_ReceiveAnalyze;
}
