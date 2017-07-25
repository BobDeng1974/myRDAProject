#include "user.h"
#define NTP_PACK_LEN (48)
#define LI 			(3)	//未同步
#define VN 			(4)	//版本号4
#define MODE 		(3)	//客户端模式
#define STRATUM 	(0) //对本地时钟级别的整体识别
#define POLL 		(6) //有符号整数表示连续信息间的最大间隔
#define PREC 		(236)//有符号整数表示本地时钟精确度
//#define SYNC_PERIOD (600)
#define SYNC_PERIOD (24 * 3600)//24小时同步一次
#define JAN_1970 		(0x83aa7e80) /* 1900年～1970年之间的时间秒数 */
#define NTPFRAC(x)		(4294 * (x) + ((1981 * (x)) >> 11))
#define USEC(x)			(((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))
typedef struct
{
	u8 URL[URL_LEN_MAX];
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
	u32 Param;
	u32 RootDelay;
	u32 RootDispersion;
	u32 ReferenceIdentifier;
	u32 ReferenceTampInt;
	u32 ReferenceTampFine;
	u32 OriginageTampInt;
	u32 OriginageTampFine;
	u32 ReceiveTampInt;
	u32 ReceiveTampFine;
	u32 TransmitTampInt;
	u32 TransmitTampFine;
}NTP_APUStruct;

typedef struct
{
	Net_CtrlStruct Net;
	NTP_APUStruct RxAPU;
	NTP_APUStruct TxAPU;
	u8 IsNTPOK;
	u8 IsNTPError;
}NTP_CtrlStruct;

NTP_CtrlStruct __attribute__((section (".usr_ram"))) NTPCtrl;
s32 NTP_ReceiveAnalyze(void *pData)
{
	u32 RxLen = (u32)pData;
	//u32 FinishLen = 0;
	//u32 TxLen;
	u64 NewTamp;
	//u64 SysTamp;
	u8 *Buf;
	HAL_TIM_RTC_TIME_T RTC;
	Date_Union uDate;
	Time_Union uTime;
//	Date_Union uDateOld;
//	Time_Union uTimeOld;
	if (RxLen != NTP_PACK_LEN)
	{
		Buf = COS_MALLOC(1024);
		while (RxLen)
		{
			RxLen -= OS_SocketReceive(NTPCtrl.Net.SocketID, Buf, 1024, NULL, NULL);
		}
		NTPCtrl.IsNTPError = 1;
		DBG("!");
		COS_FREE(Buf);
		return -1;
	}
	else
	{
		OS_SocketReceive(NTPCtrl.Net.SocketID, (u8 *)&NTPCtrl.RxAPU, NTP_PACK_LEN, NULL, NULL);
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
	u8 i;
	u8 Retry;
	IP_AddrUnion uIP;

	NTPCtrl.TxAPU.Param = htonl((LI << 30)|(VN << 27)|(MODE << 24)|(STRATUM << 16)|(POLL << 8)|(PREC & 0xff));
	while(1)
	{
		NTPCtrl.IsNTPOK = 0;
		NTPCtrl.IsNTPError = 0;
		for (i = 0; i < sizeof(NTP_ServerList)/sizeof(NTP_URLStruct);i++)
		{
			NTPCtrl.Net.To = 70;
			Net_Connect(&NTPCtrl.Net, 0, (u8 *)NTP_ServerList[i].URL);
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
				DBG("IP %u.%u.%u.%u OK", (u32)uIP.u8_addr[0], (u32)uIP.u8_addr[1],
						(u32)uIP.u8_addr[2], (u32)uIP.u8_addr[3]);
				for(Retry = 0; Retry < 3; Retry++)
				{
					Net_Send(&NTPCtrl.Net, (u8 *)&NTPCtrl.TxAPU, NTP_PACK_LEN);
					NTPCtrl.Net.To = 15;
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
				NTPCtrl.Net.To = 15;
				if (NTPCtrl.Net.SocketID != INVALID_SOCKET)
				{
					Net_Disconnect(&NTPCtrl.Net);
				}
			}
		}
		NTPCtrl.Net.To = 15;
		Net_Disconnect(&NTPCtrl.Net);
		NTPCtrl.Net.To = SYNC_PERIOD;
		Net_WaitTime(&NTPCtrl.Net);
	}
}

void NTP_Config(void)
{
	gSys.TaskID[NTP_TASK_ID] = COS_CreateTask(NTP_Task, NULL,
					NULL, MMI_TASK_MIN_STACK_SIZE , MMI_TASK_PRIORITY + NTP_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI NTP Task");
	memset(&NTPCtrl, 0, sizeof(NTPCtrl));
	NTPCtrl.Net.SocketID = INVALID_SOCKET;
	NTPCtrl.Net.TaskID = gSys.TaskID[NTP_TASK_ID];
	NTPCtrl.Net.Channel = GPRS_CH_NTP;
	NTPCtrl.Net.TimerID = NTP_TIMER_ID;
	NTPCtrl.Net.ReceiveFun = NTP_ReceiveAnalyze;
	NTPCtrl.Net.UDPPort = NTP_PORT;

}
