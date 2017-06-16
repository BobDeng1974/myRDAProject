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
	GPRS_PDP_DEACTING,
	GPRS_DETACHING,

	GPRS_CH_MAIN_MONITOR = 0,
	GPRS_CH_FTP_CMD,
	GPRS_CH_FTP_DATA,
	GPRS_CH_REMOTE,
	GPRS_CH_MAX,

    NET_STATE_OFFLINE = 0,
	NET_STATE_CONNECT,
	NET_STATE_ONLINE,
	NET_STATE_DISCONNECT,
};

typedef struct
{
	s8 Url[URL_LEN_MAX];
	HANDLE TaskID;
}URL_ReqStruct;

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
	RBuffer UrlBuf;
	URL_ReqStruct UrlData[GPRS_CH_MAX];
	HANDLE IndTaskID[GPRS_CH_MAX];
	SOCKET IndSocketID[GPRS_CH_MAX];
	URL_ReqStruct CurUrl;
	u8 GetHostBusy;
}GPRS_CtrlStruct;

void GPRS_Config(void);
void GPRS_EventAnalyze(CFW_EVENT *Event);
void GPRS_MonitorTask(void *pData);
void GPRS_GetHostBot(void);
void GPRS_GetHostResult(u32 IP);
void GPRS_GetHostReq(u8 *Url, u32 TaskID);
void GPRS_RegChannel(u8 Channel, s8 SocketID, HANDLE TaskID);
HANDLE GPRS_GetTaskFromSocketID(s8 SocketID);
u8 RssiToCSQ(u8 nRssi);
#endif
