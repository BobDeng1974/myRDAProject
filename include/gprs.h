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
	int8_t Url[URL_LEN_MAX + 8];
	HANDLE TaskID;
	SOCKET Socket;
}GPRS_ChannelStruct;

typedef struct
{
	uint8_t ID[4];
}Cell_InfoStruct;

typedef union
{
	Cell_InfoStruct CellInfo;
	uint32_t CellID;
}Cell_InfoUnion;

typedef struct
{
	uint8_t CSQ;
	uint8_t CI_BE[2];
}LBS_CIStruct;

typedef struct
{
	uint8_t LAC_BE[2];
	uint8_t MCC[2];
	uint8_t MNC[1];
	uint8_t CINum;
	LBS_CIStruct CI[7];
}LBS_LACStruct;

typedef struct
{
	uint8_t LACNum;
	LBS_LACStruct LAC[7];
}LBS_InfoStruct;

typedef struct
{
	uint32_t *Param;
	uint32_t To;
	GPRS_ChannelStruct Data[GPRS_CH_MAX];
}GPRS_CtrlStruct;

void GPRS_Config(void);
void GPRS_EventAnalyze(CFW_EVENT *Event);
void GPRS_MonitorTask(void);
void GPRS_GetHostResult(int8_t *HostName, uint32_t IP);
void GPRS_Restart(void);
int32_t GPRS_RegDNS(uint8_t Channel, uint8_t *Url);
void GPRS_RegChannel(uint8_t Channel, HANDLE TaskID);
void GPRS_RegSocket(uint8_t Channel, SOCKET Socket);
void GPRS_ResetSocket(uint8_t Channel);
HANDLE GPRS_GetTaskFromSocketID(SOCKET SocketID);
uint8_t RssiToCSQ(uint8_t nRssi);

#endif
