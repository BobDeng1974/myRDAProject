#include "user.h"
#define MAX_NUM_OF_PARAM  8
#define MAX_LEN_OF_PARAM  32
#define FTP_WAIT_DATA_DELAY (70)
#define FTP_WAIT_CMD_RET_DELAY (70)
enum
{
	FTP_STATE_NONE = 0,
	FTP_STATE_NET_DATA,
	FTP_STATE_NET_CTRL,
	FTP_STATE_LOGINON_USER,
	FTP_STATE_LOGINON_PASS,
	FTP_STATE_TYPE,
	FTP_STATE_FILESIZE,
	FTP_STATE_PASV_MOD,
	FTP_STATE_FILE_REST,
	FTP_STATE_DLFILE_REQ,
	FTP_STATE_TRANSFER_GOING,
	FTP_STATE_QUIT_LOGIN,
	FTP_STATE_ERROR,



	/*FTP protocol command*/
	FTP_RET_CMD_DATA_CONNECT_ALREADY    = 125, /*Data connection already open; Transfer starting.*/
	FTP_RET_CMD_CONNECTED               = 220, /*service ready*/
	FTP_RET_CMD_REQ_PW                  = 331,
	FTP_RET_CMD_LOGGEDON                = 230,
	FTP_RET_CMD_CWD                     = 250,
	FTP_RET_CMD_TYPE                    = 200,
	FTP_RET_CMD_PASVMOD                 = 227,
	FTP_RET_CMD_PWD						= 257,
	FTP_RET_CMD_DATA_PIPE_ACCEPT        = 150,
	FTP_RET_CMD_TRANSFER_OK             = 226,
	FTP_RET_CMD_FILESIZE                = 213,
	FTP_RET_CMD_REST                    = 350,
	FTP_RET_CMD_GOODBYE                 = 221,

	/*error*/
	/*421 is the most common error*/
	FTP_RET_CMD_SERVER_LOCKED           = 421, /**/
	FTP_RET_CMD_CONNECT_TIMEOUT         = 421, /*Connection timed out*/
	FTP_RET_CMD_BE_TICKED               = 421, /*be ticked*/

	FTP_RET_CMD_USER_PW_INCORRECT       = 530, /*Login or password incorrect*/
	FTP_RET_CMD_FILE_NOT_FIND           = 550, /*File not found*/
	FTP_RET_CMD_CLOSE_CONNECT           = 426, /*Connection closed; transfer aborted.*/
//	FTP_CMD_NONE = 0,
//	FTP_CMD_START,
//	FTP_CMD_SUCCESS,
//	FTP_CMD_TIMEROUT,
//	FTP_CMD_FAIL,
};
FTP_CtrlStruct __attribute__((section (".usr_ram"))) FTPCtrl;

void FTP_Finish(u32 Result)
{
	DBG("%x", Result);
	if (FTPCtrl.TaskID != gSys.TaskID[USER_TASK_ID])
	{
		DBG("%x", FTPCtrl.TaskID);
	}
	OS_SendEvent(FTPCtrl.TaskID, EV_MMI_FTP_FINISH, Result, 0, 0);
	OS_SendEvent(gSys.TaskID[FTP_DATA_TASK_ID], EV_MMI_FTP_FINISH, Result, 0, 0);
}

s32 FTP_CtrlReceive(void *pData)
{
	u8 *Buf = COS_MALLOC(MAX_NUM_OF_PARAM * MAX_LEN_OF_PARAM);
	u8 *RxBuf = COS_MALLOC(MAX_NUM_OF_PARAM * MAX_LEN_OF_PARAM + 4);
	u8 CmdRetString[4];
	u32 RxLen = (u32)pData;
	u32 Ret, i;
	u16 Port;
	s8 *Start;
	IP_AddrUnion uIP;
	CmdParam Param;
	//DBG("Receive %d", RxLen);
	OS_SocketReceive(FTPCtrl.CmdCtrl.SocketID, RxBuf, RxLen, NULL, NULL);
	RxBuf[RxLen] = 0;
	//DBG("%s", RxBuf);
	if (IsDigitStr(RxBuf, 3))
	{
		memcpy(CmdRetString, RxBuf, 3);
		CmdRetString[3] = 0;
		Ret = strtol(CmdRetString, NULL, 10);
		DBG("%d", Ret);
		switch(Ret)
		{
		case FTP_RET_CMD_CONNECTED:
			FTPCtrl.State = FTP_STATE_LOGINON_USER;
			break;
		case FTP_RET_CMD_REQ_PW:
			FTPCtrl.State = FTP_STATE_LOGINON_PASS;
			break;
		case FTP_RET_CMD_LOGGEDON:
			FTPCtrl.State = FTP_STATE_TYPE;
			break;
		case FTP_RET_CMD_TYPE:
			FTPCtrl.State = FTP_STATE_FILESIZE;
			break;
		case FTP_RET_CMD_FILESIZE:
			FTPCtrl.DownloadLen = strtol((s8 *)(RxBuf + 4), NULL, 10);
			DBG("download file len %d", FTPCtrl.DownloadLen);
			FTPCtrl.State = FTP_STATE_PASV_MOD;
			break;

		case FTP_RET_CMD_PASVMOD:

			/*get service port for passive mode*/
			Param.param_max_num = MAX_NUM_OF_PARAM;
			Param.param_max_len = MAX_LEN_OF_PARAM;
			Param.param_num = 0;
			Param.param_str = Buf;
			memset(Buf, 0, MAX_NUM_OF_PARAM * MAX_LEN_OF_PARAM);
			Start = strchr((s8 *)(RxBuf + 4), '(');
			CmdParseParam((s8*)Start + 1, &Param, ',');
			for(i = 0; i < 4; i++)
			{
				uIP.u8_addr[i] = strtol((s8 *)&Buf[i * MAX_LEN_OF_PARAM], NULL, 10);
			}
			Port = strtol((s8 *)&Buf[4 * MAX_LEN_OF_PARAM], NULL, 10);
			Port = Port * 256 + strtol((s8 *)&Buf[5 * MAX_LEN_OF_PARAM], NULL, 10);
			DBG("ftp data point %d.%d.%d.%d %d", uIP.u8_addr[0], uIP.u8_addr[1], uIP.u8_addr[2],
					uIP.u8_addr[3], Port);
			FTPCtrl.DataCtrl.IPAddr.s_addr = uIP.u32_addr;
			FTPCtrl.DataCtrl.TCPPort = Port;
			OS_SendEvent(gSys.TaskID[FTP_DATA_TASK_ID], EV_MMI_FTP_DATA_START, 0, 0, 0);
			break;
		case FTP_RET_CMD_REST:
			FTPCtrl.State = FTP_STATE_DLFILE_REQ;
			break;
		case FTP_RET_CMD_GOODBYE:
			FTPCtrl.State = FTP_STATE_QUIT_LOGIN;
			break;
		case FTP_RET_CMD_DATA_PIPE_ACCEPT:
		case FTP_RET_CMD_DATA_CONNECT_ALREADY:
            /*wait for the first block of data.*/
			FTPCtrl.State =  FTP_STATE_TRANSFER_GOING;
        	break;
//		case FTP_RET_CMD_BE_TICKED:
//
//		case FTP_RET_CMD_USER_PW_INCORRECT:
//
//		case FTP_RET_CMD_FILE_NOT_FIND:
//
//		break;

		default:
			FTPCtrl.State = FTP_STATE_ERROR;
			break;
		}
	}
	else
	{
		DBG("!");
		FTPCtrl.State = FTP_STATE_ERROR;
	}
	COS_FREE(Buf);
}

s32 FTP_CtrlBot(void)
{
	u32 TxLen;
	Net_CtrlStruct *Net = &FTPCtrl.CmdCtrl;
	if (FTPCtrl.Cmd.IP)
	{
		Net_Connect(Net, FTPCtrl.Cmd.IP, NULL);
	}
	else
	{
		Net_Connect(Net, 0, FTPCtrl.Cmd.Url);
	}
	if (Net->Result != NET_RES_CONNECT_OK)
	{
		goto ERROR_END;
	}

	Net->To = 10;
	Net_WaitEvent(Net);
	if (Net->Result != NET_RES_UPLOAD)
	{
		goto ERROR_END;
	}
	if (FTP_STATE_LOGINON_USER != FTPCtrl.State)
	{
		goto ERROR_END;
	}

	Net->To = FTP_WAIT_CMD_RET_DELAY;
	TxLen = sprintf(FTPCtrl.SendBuf, "USER %s\r\n", FTPCtrl.Cmd.User);
	DBG("%s", FTPCtrl.SendBuf);
	Net_Send(Net, FTPCtrl.SendBuf, TxLen);
	if (Net->Result != NET_RES_SEND_OK)
	{
		goto ERROR_END;
	}
	Net_WaitEvent(Net);
	if (Net->Result != NET_RES_UPLOAD)
	{
		goto ERROR_END;
	}
	if (FTP_STATE_LOGINON_PASS != FTPCtrl.State)
	{
		goto ERROR_END;
	}

	TxLen = sprintf(FTPCtrl.SendBuf, "PASS %s\r\n", FTPCtrl.Cmd.Pwd);
	DBG("%s", FTPCtrl.SendBuf);
	Net_Send(Net, FTPCtrl.SendBuf, TxLen);
	if (Net->Result != NET_RES_SEND_OK)
	{
		goto ERROR_END;
	}
	Net_WaitEvent(Net);
	if (Net->Result != NET_RES_UPLOAD)
	{
		goto ERROR_END;
	}
	if (FTP_STATE_TYPE != FTPCtrl.State)
	{
		goto ERROR_END;
	}

	TxLen = sprintf(FTPCtrl.SendBuf, "TYPE I\r\n");
	DBG("%s", FTPCtrl.SendBuf);
	Net_Send(Net, FTPCtrl.SendBuf, TxLen);
	if (Net->Result != NET_RES_SEND_OK)
	{
		goto ERROR_END;
	}
	Net_WaitEvent(Net);
	if (Net->Result != NET_RES_UPLOAD)
	{
		goto ERROR_END;
	}
	if (FTP_STATE_FILESIZE != FTPCtrl.State)
	{
		goto ERROR_END;
	}

	TxLen = sprintf(FTPCtrl.SendBuf, "SIZE %s\r\n", FTPCtrl.Cmd.Path);
	DBG("%s", FTPCtrl.SendBuf);
	Net_Send(Net, FTPCtrl.SendBuf, TxLen);
	if (Net->Result != NET_RES_SEND_OK)
	{
		goto ERROR_END;
	}
	Net_WaitEvent(Net);
	if (Net->Result != NET_RES_UPLOAD)
	{
		goto ERROR_END;
	}
	if (FTP_STATE_PASV_MOD != FTPCtrl.State)
	{
		goto ERROR_END;
	}

	TxLen = sprintf(FTPCtrl.SendBuf, "PASV\r\n");
	DBG("%s", FTPCtrl.SendBuf);
	Net_Send(Net, FTPCtrl.SendBuf, TxLen);
	if (Net->Result != NET_RES_SEND_OK)
	{
		goto ERROR_END;
	}
	Net_WaitEvent(Net);
	if (Net->Result != NET_RES_UPLOAD)
	{
		goto ERROR_END;
	}
	Net_WaitSpecialEvent(Net, EV_MMI_FTP_DATA_CONNECT);

	if (Net->Result != NET_RES_EVENT)
	{
		goto ERROR_END;
	}
	if (!FTPCtrl.IsDataOnline)
	{
		goto ERROR_END;
	}
	TxLen = sprintf(FTPCtrl.SendBuf, "REST %d\r\n", FTPCtrl.DownloadPos);
	DBG("%s", FTPCtrl.SendBuf);
	Net_Send(Net, FTPCtrl.SendBuf, TxLen);
	if (Net->Result != NET_RES_SEND_OK)
	{
		goto ERROR_END;
	}
	Net_WaitEvent(Net);
	if (Net->Result != NET_RES_UPLOAD)
	{
		goto ERROR_END;
	}
	if (FTP_STATE_DLFILE_REQ != FTPCtrl.State)
	{
		DBG("%d", FTPCtrl.State);
		goto ERROR_END;
	}

	TxLen = sprintf(FTPCtrl.SendBuf, "RETR %s\r\n", FTPCtrl.Cmd.Path);
	DBG("%s", FTPCtrl.SendBuf);
	Net_Send(Net, FTPCtrl.SendBuf, TxLen);
	if (Net->Result != NET_RES_SEND_OK)
	{
		DBG("%d", FTPCtrl.State);
		goto ERROR_END;
	}

	while (1)
	{
		Net_WaitSpecialEvent(Net, EV_MMI_FTP_DATA_FINISH);
		if (FTPCtrl.DownloadPos >= FTPCtrl.DownloadLen)
		{
			break;
		}
		if (Net->Result != NET_RES_UPLOAD)
		{
			goto ERROR_END;
		}
		if (FTP_STATE_ERROR == FTPCtrl.State)
		{
			DBG("%d", FTPCtrl.State);
			goto ERROR_END;
		}
	}
	TxLen = sprintf(FTPCtrl.SendBuf, "QUIT\r\n");
	DBG("%s", FTPCtrl.SendBuf);
	Net_Send(Net, FTPCtrl.SendBuf, TxLen);
	if (Net->Result != NET_RES_SEND_OK)
	{
		goto ERROR_END;
	}
	Net_WaitEvent(Net);
	if (Net->Result != NET_RES_UPLOAD)
	{
		goto ERROR_END;
	}
//	if (FTP_STATE_QUIT_LOGIN!= FTPCtrl.State)
//	{
//		DBG("%d", FTPCtrl.State);
//		goto ERROR_END;
//	}
	Net_Disconnect(Net);
	if (FTPCtrl.IsDataOnline)
	{
		OS_SendEvent(gSys.TaskID[FTP_CTRL_TASK_ID], EV_MMI_FTP_DATA_STOP, 0, 0 ,0);
	}
	return 0;

ERROR_END:
	Net_Disconnect(Net);
	if (FTPCtrl.IsDataOnline)
	{
		OS_SendEvent(gSys.TaskID[FTP_CTRL_TASK_ID], EV_MMI_FTP_DATA_STOP, 0, 0 ,0);
	}

	return -1;
}

void FTP_CtrlTask(void *pData)
{
	COS_EVENT Event;
	IP_AddrUnion uIP;
	u8 Retry;
	DBG("Task start!");
	while (1)
	{
		COS_WaitEvent(gSys.TaskID[FTP_CTRL_TASK_ID], &Event, COS_WAIT_FOREVER);
		if (EV_MMI_FTP_START != Event.nEventId)
		{
			continue;
		}

		if (FTPCtrl.Cmd.IP)
		{
			uIP.u32_addr = FTPCtrl.Cmd.IP;
			DBG("%d.%d.%d.%d %d %s %s %s", uIP.u8_addr[0], uIP.u8_addr[1], uIP.u8_addr[2], uIP.u8_addr[3],
					FTPCtrl.Cmd.Port, FTPCtrl.Cmd.Path, FTPCtrl.Cmd.User, FTPCtrl.Cmd.Pwd);
		}
		else
		{
			DBG("%s %d %s %s %s", FTPCtrl.Cmd.Url,
					FTPCtrl.Cmd.Port, FTPCtrl.Cmd.Path, FTPCtrl.Cmd.User, FTPCtrl.Cmd.Pwd);
		}

		FTPCtrl.CmdCtrl.ReceiveFun = FTP_CtrlReceive;
		FTPCtrl.CmdCtrl.TCPPort = FTPCtrl.Cmd.Port;
		FTPCtrl.State = FTP_STATE_NET_CTRL;
		FTPCtrl.CmdCtrl.To = FTP_WAIT_CMD_RET_DELAY;
		FTPCtrl.DownloadLen = 0;
		FTPCtrl.DownloadPos = 0;
		for (Retry = 0; Retry < 3; Retry++)
		{
			if (FTP_CtrlBot() >= 0)
			{
				break;
			}
		}
		if (Retry >= 3)
		{
			FTP_Finish(FTP_FINISH_FAIL);
		}
		else
		{
			FTP_Finish(FTP_FINISH_OK);
		}
		FTPCtrl.State = FTP_STATE_NONE;
	}
}

s32 FTP_DataReceive(void *pData)
{
	u32 RxLen = (u32)pData;
	OS_SocketReceive(FTPCtrl.DataCtrl.SocketID, FTPCtrl.Cmd.Buf + FTPCtrl.DownloadPos, RxLen, NULL, NULL);
	FTPCtrl.DownloadPos += RxLen;
	DBG("%d", FTPCtrl.DownloadPos);
}

void FTP_DataTask(void *pData)
{
	COS_EVENT Event;
	IP_AddrUnion uIP;
	Net_CtrlStruct *Net = &FTPCtrl.DataCtrl;
	u32 RxLen;
	u8 Retry;
	u8 ErrorOut;
	DBG("Task start!");
	while (1)
	{
		FTPCtrl.IsDataOnline = 0;
		COS_WaitEvent(gSys.TaskID[FTP_DATA_TASK_ID], &Event, COS_WAIT_FOREVER);
		if (Event.nEventId != EV_MMI_FTP_DATA_START)
		{
			continue;
		}
		Net->To = FTP_WAIT_DATA_DELAY;
		FTPCtrl.DataCtrl.ReceiveFun = FTP_DataReceive;
		for(Retry = 0; Retry < 3; Retry++)
		{
			Net_Connect(Net, Net->IPAddr.s_addr, NULL);
			if (Net->Result != NET_RES_CONNECT_OK)
			{
				FTPCtrl.State = FTP_STATE_ERROR;
			}
			else
			{
				FTPCtrl.State = FTP_STATE_FILE_REST;
			}

			if (FTP_STATE_ERROR == FTPCtrl.State)
			{
				Net_Disconnect(Net);
				continue;
			}
			FTPCtrl.IsDataOnline = 1;
			OS_SendEvent(gSys.TaskID[FTP_CTRL_TASK_ID], EV_MMI_FTP_DATA_CONNECT, 0, 0, 0);
			ErrorOut = 0;
			while (!ErrorOut)
			{
				Net_WaitSpecialEvent(Net, EV_MMI_FTP_DATA_STOP);
				switch(Net->Result)
				{
				case NET_RES_EVENT:
					break;
				case NET_RES_TO:
				case NET_RES_ERROR:
					Net_Disconnect(Net);
					ErrorOut = 1;
					break;
				}
				if (FTPCtrl.DownloadPos >= FTPCtrl.DownloadLen)
				{
					DBG("FTP Data OK!");
					Net_Disconnect(Net);
					FTPCtrl.IsDataOnline = 0;
					FTPCtrl.State = FTP_STATE_QUIT_LOGIN;
					OS_SendEvent(gSys.TaskID[FTP_CTRL_TASK_ID], EV_MMI_FTP_DATA_FINISH, 0, 0, 0);
					ErrorOut = 1;

					break;
				}
			}
			if (FTPCtrl.DownloadPos >= FTPCtrl.DownloadLen)
			{
				DBG("FTP Data Bot Finish!");
				Retry = 0;
				break;
			}
		}
		if (Retry > 3)
		{
			DBG("FTP Data Error!");
			FTPCtrl.State = FTP_STATE_ERROR;
			OS_SendEvent(gSys.TaskID[FTP_CTRL_TASK_ID], EV_MMI_FTP_DATA_FINISH, 0, 0, 0);
			FTPCtrl.IsDataOnline = 0;
		}
	}
}

void FTP_Config(void)
{
	gSys.TaskID[FTP_CTRL_TASK_ID] = COS_CreateTask(FTP_CtrlTask, NULL, NULL, MMI_TASK_MAX_STACK_SIZE/2, MMI_TASK_PRIORITY + FTP_CTRL_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI FTP Ctrl Task");
	gSys.TaskID[FTP_DATA_TASK_ID] = COS_CreateTask(FTP_DataTask, NULL, NULL, MMI_TASK_MIN_STACK_SIZE, MMI_TASK_PRIORITY + FTP_DATA_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI FTP Data Task");
	FTPCtrl.State = FTP_STATE_NONE;
}

s32 FTP_UpgradeStart(FTP_CmdStruct *Cmd, HANDLE CBTaskID)
{
	if (FTP_STATE_NONE != FTPCtrl.State)
	{
		return -1;
	}
	memcpy(&FTPCtrl.Cmd, Cmd, sizeof(FTP_CmdStruct));
	memset(&FTPCtrl.CmdCtrl, 0, sizeof(Net_CtrlStruct));
	memset(&FTPCtrl.DataCtrl, 0, sizeof(Net_CtrlStruct));
	FTPCtrl.TaskID = CBTaskID;
	FTPCtrl.CmdCtrl.TaskID = gSys.TaskID[FTP_CTRL_TASK_ID];
	FTPCtrl.CmdCtrl.TimerID = FTP_CTRL_TIMER_ID;
	FTPCtrl.CmdCtrl.Channel = GPRS_CH_FTP_CMD;
	FTPCtrl.CmdCtrl.SocketID = INVALID_SOCKET;
	FTPCtrl.DataCtrl.TaskID = gSys.TaskID[FTP_DATA_TASK_ID];
	FTPCtrl.DataCtrl.TimerID = FTP_DATA_TIMER_ID;
	FTPCtrl.DataCtrl.Channel = GPRS_CH_FTP_DATA;
	FTPCtrl.DataCtrl.SocketID = INVALID_SOCKET;
	OS_SendEvent(gSys.TaskID[FTP_CTRL_TASK_ID], EV_MMI_FTP_START, 0, 0, 0);
}

s32 FTP_StartCmd(s8 *CmdStr, u8 *Buf)
{
	FTP_CmdStruct Cmd;
	u32 Start, End;
	s32 IP;

	memset(&Cmd, 0, sizeof(FTP_CmdStruct));
	if (!memcmp(CmdStr, "//", 2)) // Ìø¹ý"//"
	{
		Start = CmdStr + 2;
	}
	End = strchr(Start, '/');
	if (End)
	{
		memcpy(Cmd.Url, Start, End - Start);
	}
	else
	{
		return 0;
	}

	Start = End + 1;
	End = strchr(Start, ':');
	if (End)
	{
		memcpy(Cmd.Path, Start, End - Start);
	}
	else
	{
		return 0;
	}

	Start = End + 1;
	End = strchr(Start, '@');
	if (End)
	{
		*(u8 *)End = 0;
		Cmd.Port = strtoul(Start, NULL, 10);
	}
	else
	{
		return 0;
	}

	Start = End + 1;
	End = strchr(Start, ':');
	if (End)
	{
		memcpy(Cmd.User, Start, End - Start);
	}
	else
	{
		return 0;
	}
	Start = End + 1;
	strcpy(Cmd.Pwd, Start);
	DBG("%s %s %d %s %s", Cmd.Url, Cmd.Path, Cmd.Port, Cmd.User, Cmd.Pwd);
	IP = inet_addr(Cmd.Url);
	if (IP != -1)
	{
		Cmd.IP = IP;
	}
	Cmd.Buf = Buf;
	FTP_UpgradeStart(&Cmd, gSys.TaskID[USER_TASK_ID]);
}
