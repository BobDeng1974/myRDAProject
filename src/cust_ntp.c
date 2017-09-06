#include "user.h"
#define NTP_PACK_LEN (48)
#define LI 			(3)	//未同步
#define VN 			(4)	//版本号4
#define MODE 		(3)	//客户端模式
#define STRATUM 	(0) //对本地时钟级别的整体识别
#define POLL 		(6) //有符号整数表示连续信息间的最大间隔
#define PREC 		(236)//有符号整数表示本地时钟精确度
//#define SYNC_PERIOD (600)
#define SYNC_PERIOD (12 * 3600)//12小时同步一次
#define JAN_1970 		(0x83aa7e80) /* 1900年～1970年之间的时间秒数 */
#define NTPFRAC(x)		(4294 * (x) + ((1981 * (x)) >> 11))
#define USEC(x)			(((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))
typedef struct
{
	uint8_t URL[URL_LEN_MAX];
}NTP_URLStruct;
#define NTP_PORT	(123)

const NTP_URLStruct NTP_ServerList[] =
{
		{"ntp1.aliyun.com"},
		{"ntp2.aliyun.com"},
		{"ntp3.aliyun.com"},
		{"ntp4.aliyun.com"},
		{"ntp5.aliyun.com"},
		{"ntp7.aliyun.com"},
		{"ntp6.aliyun.com"},
		{"s2c.time.edu.cn"},
		{"194.109.22.18"},
		{"210.72.145.44"},
};



typedef struct
{
	uint32_t Param;
	uint32_t RootDelay;
	uint32_t RootDispersion;
	uint32_t ReferenceIdentifier;
	uint32_t ReferenceTampInt;
	uint32_t ReferenceTampFine;
	uint32_t OriginageTampInt;
	uint32_t OriginageTampFine;
	uint32_t ReceiveTampInt;
	uint32_t ReceiveTampFine;
	uint32_t TransmitTampInt;
	uint32_t TransmitTampFine;
}NTP_APUStruct;

typedef struct
{
	Net_CtrlStruct Net;
	NTP_APUStruct RxAPU;
	NTP_APUStruct TxAPU;
	uint8_t IsNTPOK;
	uint8_t IsNTPError;
	uint8_t IsNeedNtp;
}NTP_CtrlStruct;

NTP_CtrlStruct __attribute__((section (".usr_ram"))) NTPCtrl;
int32_t NTP_ReceiveAnalyze(void *pData)
{
	uint32_t RxLen = (uint32_t)pData;
	int32_t Error;
	//uint32_t FinishLen = 0;
	//uint32_t TxLen;
	CFW_TCPIP_SOCKET_ADDR From;
	INT32 FromLen;
	uint64_t NewTamp;
	//uint64_t SysTamp;
	uint8_t *Buf;
	HAL_TIM_RTC_TIME_T RTC;
	Date_Union uDate;
	Time_Union uTime;
//	Date_Union uDateOld;
//	Time_Union uTimeOld;
	if (RxLen != NTP_PACK_LEN)
	{
		Buf = COS_MALLOC(1024);
		do
		{
			Error = (int32_t)OS_SocketReceive(NTPCtrl.Net.SocketID, Buf, 1024, &From, &FromLen);
		}while(Error > 0);
		NTPCtrl.IsNTPError = 1;
		DBG("!");
		COS_FREE(Buf);
		return -1;
	}
	else
	{

		Error = (int32_t)OS_SocketReceive(NTPCtrl.Net.SocketID, (uint8_t *)&NTPCtrl.RxAPU, NTP_PACK_LEN, &From, &FromLen);
		DBG("%d", Error);
		if (Error <= 0)
		{
			NTPCtrl.IsNTPOK = 1;
			return -1;
		}
		NewTamp = htonl(NTPCtrl.RxAPU.TransmitTampInt) - JAN_1970;
		Tamp2UTC(NewTamp, &uDate.Date, &uTime.Time, 0);
//		uDateOld.dwDate = gSys.Var[UTC_DATE];
//		uTimeOld.dwTime = gSys.Var[UTC_TIME];
//		SysTamp = UTC2Tamp(&uDateOld.Date, &uTimeOld.Time);
		if (gSys.State[GPS_STATE] != GPS_A_STAGE)
		{
			DBG("%u %u %u %u:%u:%u", uDate.Date.Year, uDate.Date.Mon, uDate.Date.Day,
					uTime.Time.Hour, uTime.Time.Min, uTime.Time.Sec);
			RTC.year = uDate.Date.Year - 2000;
			RTC.month = uDate.Date.Mon;
			RTC.day = uDate.Date.Day;
			RTC.hour = uTime.Time.Hour;
			RTC.min = uTime.Time.Min;
			RTC.sec = uTime.Time.Sec;
			hal_TimRtcSetTime(&RTC);
		}
		NTPCtrl.IsNTPOK = 1;
	}
	return 0;
}

void NTP_Task(void *pData)
{
	uint8_t i;
	uint8_t Retry;
	IP_AddrUnion uIP;
#ifndef __NTP_ENABLE__
	COS_EVENT Event;
	while(1)
	{
		COS_WaitEvent(gSys.TaskID[NTP_TASK_ID], &Event, COS_WAIT_FOREVER);
	}
#endif
	NTPCtrl.TxAPU.Param = htonl((LI << 30)|(VN << 27)|(MODE << 24)|(STRATUM << 16)|(POLL << 8)|(PREC & 0xff));
	GPRS_RegChannel(NTPCtrl.Net.Channel, NTPCtrl.Net.TaskID);
	Net_WaitGPRSAct(&NTPCtrl.Net);
	while(1)
	{
		if (NTPCtrl.IsNeedNtp)
		{
			NTPCtrl.IsNTPOK = 0;
			NTPCtrl.IsNTPError = 0;
			for (i = 0; i < sizeof(NTP_ServerList)/sizeof(NTP_URLStruct);i++)
			{
				NTPCtrl.Net.To = 70;
				Net_Connect(&NTPCtrl.Net, 0, (uint8_t *)NTP_ServerList[i].URL);
				if (NTPCtrl.Net.Result != NET_RES_CONNECT_OK)
				{
					if (NTPCtrl.Net.SocketID != INVALID_SOCKET)
					{
						Net_Disconnect(&NTPCtrl.Net);
					}
					continue;
				}
				else
				{
					uIP.u32_addr = NTPCtrl.Net.IPAddr.s_addr;
					DBG("IP %u.%u.%u.%u OK", (uint32_t)uIP.u8_addr[0], (uint32_t)uIP.u8_addr[1],
							(uint32_t)uIP.u8_addr[2], (uint32_t)uIP.u8_addr[3]);
					for(Retry = 0; Retry < 5; Retry++)
					{
						Net_Send(&NTPCtrl.Net, (uint8_t *)&NTPCtrl.TxAPU, NTP_PACK_LEN);
						NTPCtrl.Net.To = 10;
						Net_WaitEvent(&NTPCtrl.Net);
						if (NTPCtrl.IsNTPOK)
						{
							DBG("%s get time", NTP_ServerList[i].URL);
							break;
						}
					}
					if (NTPCtrl.IsNTPOK)
					{
						break;
					}

					if (NTPCtrl.Net.SocketID != INVALID_SOCKET)
					{
						NTPCtrl.Net.To = 15;
						Net_Disconnect(&NTPCtrl.Net);
					}
				}
			}
			if (NTPCtrl.Net.SocketID != INVALID_SOCKET)
			{
				NTPCtrl.Net.To = 15;
				Net_Disconnect(&NTPCtrl.Net);
			}
		}
		else
		{
			DBG("utc time already check");
		}

		if (NTPCtrl.Net.SocketID != INVALID_SOCKET)
		{
			NTPCtrl.Net.To = 15;
			Net_Disconnect(&NTPCtrl.Net);
		}

		NTPCtrl.IsNeedNtp = 1;
		NTPCtrl.Net.To = SYNC_PERIOD;
		Net_WaitTime(&NTPCtrl.Net);
	}
}

void NTP_Config(void)
{
	gSys.TaskID[NTP_TASK_ID] = COS_CreateTask(NTP_Task, NULL,
					NULL, MMI_TASK_MAX_STACK_SIZE , MMI_TASK_PRIORITY + NTP_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI NTP Task");
	memset(&NTPCtrl, 0, sizeof(NTPCtrl));
	NTPCtrl.Net.SocketID = INVALID_SOCKET;
	NTPCtrl.Net.TaskID = gSys.TaskID[NTP_TASK_ID];
	NTPCtrl.Net.Channel = GPRS_CH_NTP;
	NTPCtrl.Net.TimerID = NTP_TIMER_ID;
	NTPCtrl.Net.ReceiveFun = NTP_ReceiveAnalyze;
	NTPCtrl.Net.UDPPort = NTP_PORT;
	NTPCtrl.Net.LocalPort = UDP_NTP_PORT;
	NTPCtrl.IsNeedNtp = 1;
}

void NTP_ClearNeedFlag(void)
{
	NTPCtrl.IsNeedNtp = 0;
}
