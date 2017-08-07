#ifndef __CUST_FTP_H__
#define __CUST_FTP_H__

#define FTP_FINISH_OK 		(0x00000001)
#define FTP_FINISH_FAIL		(0x00000002)
typedef struct
{
	uint8_t Url[128];
	uint8_t Path[128];
	uint8_t User[32];
	uint8_t Pwd[32];
	uint8_t *Buf;
	uint32_t IP;
	uint16_t Port;
}FTP_CmdStruct;

typedef struct
{
	FTP_CmdStruct Cmd;
	Net_CtrlStruct CmdCtrl;
	Net_CtrlStruct DataCtrl;
	uint32_t DownloadLen;
	uint32_t DownloadPos;
	HANDLE TaskID;
	uint8_t SendBuf[64];
	uint8_t State;
	uint8_t IsDataOnline;
}FTP_CtrlStruct;

void FTP_Config(void);
int32_t FTP_UpgradeStart(FTP_CmdStruct *Cmd, HANDLE CBTaskID);
int32_t FTP_StartCmd(int8_t *CmdStr, uint8_t *Buf);
#endif
