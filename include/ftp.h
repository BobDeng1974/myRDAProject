#ifndef __CUST_FTP_H__
#define __CUST_FTP_H__

#define FTP_FINISH_OK 		(0x00000001)
#define FTP_FINISH_FAIL		(0x00000002)
typedef struct
{
	u8 Url[128];
	u8 Path[128];
	u8 User[32];
	u8 Pwd[32];
	u8 *Buf;
	u32 IP;
	u16 Port;
}FTP_CmdStruct;

typedef struct
{
	FTP_CmdStruct Cmd;
	Net_CtrlStruct CmdCtrl;
	Net_CtrlStruct DataCtrl;
	u32 DownloadLen;
	u32 DownloadPos;
	HANDLE TaskID;
	u8 SendBuf[64];
	u8 State;
	u8 IsDataOnline;
}FTP_CtrlStruct;

void FTP_Config(void);
s32 FTP_UpgradeStart(FTP_CmdStruct *Cmd, HANDLE CBTaskID);
s32 FTP_StartCmd(s8 *CmdStr, u8 *Buf);
#endif
