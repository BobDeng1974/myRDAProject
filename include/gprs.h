#ifndef __CUST_GPRS_H__
#define __CUST_GPRS_H__

#define GPRS_RESEND_DATA_LEN	1024
#define OA_MAX_SOCK_ADDR_LEN           (64)
#define GPRS_CHN_UNICOM_MNC1		(0x01)
#define GPRS_CHN_UNICOM_MNC2		(0x06)
#define GPRS_CHN_UNICOM_MNC3		(0x07)
enum
{
	GPRS_IDLE,
	GPRS_ATTACHING,
	GPRS_PDP_ACTING,
	GPRS_RUN,
	GPRS_RESTART,


	GPRS_CH_MAIN_MONITOR = 0,
	GPRS_CH_FTP_CMD,
	GPRS_CH_FTP_DATA,
	GPRS_CH_REMOTE,
	GPRS_CH_NTP,
	GPRS_CH_LUAT,
	GPRS_CH_MAX,

    NET_STATE_OFFLINE = 0,
	NET_STATE_CONNECT,
	NET_STATE_ONLINE,
	NET_STATE_DISCONNECT,
};

typedef struct
{
	s8 Url[URL_LEN_MAX + 8];
	HANDLE TaskID;
	SOCKET Socket;
}GPRS_ChannelStruct;

typedef struct
{
	u8 ID[4];
}Cell_InfoStruct;

typedef union
{
	Cell_InfoStruct CellInfo;
	u32 CellID;
}Cell_InfoUnion;

typedef struct
{
	u32 *Param;
	u32 To;
	GPRS_ChannelStruct Data[GPRS_CH_MAX];
}GPRS_CtrlStruct;

void GPRS_Config(void);
void GPRS_EventAnalyze(CFW_EVENT *Event);
void GPRS_MonitorTask(void);
void GPRS_GetHostResult(s8 *HostName, u32 IP);

s32 GPRS_RegDNS(u8 Channel, u8 *Url);
void GPRS_RegChannel(u8 Channel, HANDLE TaskID);
void GPRS_RegSocket(u8 Channel, SOCKET Socket);
void GPRS_ResetSocket(u8 Channel);
HANDLE GPRS_GetTaskFromSocketID(SOCKET SocketID);
u8 RssiToCSQ(u8 nRssi);

#endif
