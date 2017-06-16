#include "user.h"

#define REMOTE_URL	"www.bdclw.net"
#define REMOTE_PORT	(9777)
typedef struct
{
	Net_CtrlStruct Net;
	u8 RecBuf[MONITOR_RXBUF_LEN];
	u8 SendBuf[MONITOR_TXBUF_LEN];
}Remote_CtrlStruct;

Remote_CtrlStruct __attribute__((section (".usr_ram"))) RemoteCtrl;
s32 Remote_ReceiveAnalyze(void *pData)
{
	u32 RxLen = (u32)pData;
	u32 FinishLen = 0;
	u32 TxLen;
	while (RxLen)
	{
		if (RxLen > MONITOR_RXBUF_LEN)
		{
			FinishLen = MONITOR_RXBUF_LEN;
		}
		else
		{
			FinishLen = RxLen;
		}

		RxLen -= OS_SocketReceive(RemoteCtrl.Net.SocketID, RemoteCtrl.RecBuf, FinishLen, NULL, NULL);
		RemoteCtrl.RecBuf[FinishLen] = 0;
		LV_SMSAnalyze(RemoteCtrl.RecBuf, FinishLen, RemoteCtrl.SendBuf, &TxLen);
		DBG("%s", RemoteCtrl.SendBuf);
	}
}


void Remote_Task(void *pData)
{
	u8 ConnectOK = 0;
	IP_AddrUnion uIP;
	u8 State = 0;
	u8 LastACC = 1;
	u32 TxLen;
	IO_ValueUnion Temp;
	while(1)
	{
		switch (State)
		{
		case 0:
			RemoteCtrl.Net.To = 70;
			Net_Connect(&RemoteCtrl.Net, 0, REMOTE_URL);
			if (RemoteCtrl.Net.Result != NET_RES_CONNECT_OK)
			{
				if (RemoteCtrl.Net.SocketID != INVALID_SOCKET)
				{
					Net_Disconnect(&RemoteCtrl.Net);
				}
				LastACC = 1;
				State = 2;
			}
			else
			{
				uIP.u32_addr = RemoteCtrl.Net.IPAddr.s_addr;
				DBG("IP %d.%d.%d.%d OK", (u32)uIP.u8_addr[0], (u32)uIP.u8_addr[1],
						(u32)uIP.u8_addr[2], (u32)uIP.u8_addr[3]);
				State = 1;
			}
			break;
		case 1:
			if (gSys.TraceBuf.Len)
			{
				memcpy(RemoteCtrl.SendBuf, gSys.IMEIStr, 20);
				TxLen = QueryRBuffer(&gSys.TraceBuf, RemoteCtrl.SendBuf + 20, 1340);
				RemoteCtrl.Net.To = 70;
				Net_Send(&RemoteCtrl.Net, RemoteCtrl.SendBuf, TxLen + 20);
				if (RemoteCtrl.Net.Result != NET_RES_SEND_OK)
				{
					RemoteCtrl.Net.To = 15;
					if (RemoteCtrl.Net.SocketID != INVALID_SOCKET)
					{
						Net_Disconnect(&RemoteCtrl.Net);
					}
					State = 0;
				}
				else
				{
					DelRBuffer(&gSys.TraceBuf, TxLen);
				}
			}
			else
			{
				RemoteCtrl.Net.To = 1;
				Net_WaitEvent(&RemoteCtrl.Net);
    			if (RemoteCtrl.Net.Result == NET_RES_ERROR)
    			{
    				RemoteCtrl.Net.To = 15;
    				if (RemoteCtrl.Net.SocketID != INVALID_SOCKET)
    				{
    					Net_Disconnect(&RemoteCtrl.Net);
    				}
    				State = 0;
    			}
			}
			break;
		default:
#if (1)
			RemoteCtrl.Net.To = 2;
			Net_WaitTime(&RemoteCtrl.Net);

			if (LastACC)
			{
				Temp.Val = gSys.Var[IO_VAL];
				if (!Temp.IOVal.ACC)
				{
					LastACC = 0;
				}
			}
			else
			{
				Temp.Val = gSys.Var[IO_VAL];
				if (Temp.IOVal.ACC)
				{
    				RemoteCtrl.Net.To = 15;
    				if (RemoteCtrl.Net.SocketID != INVALID_SOCKET)
    				{
    					Net_Disconnect(&RemoteCtrl.Net);
    				}
    				State = 0;
				}
			}
#else
			RemoteCtrl.Net.To = 10;
			Net_WaitTime(&RemoteCtrl.Net);
			RemoteCtrl.Net.To = 15;
			if (RemoteCtrl.Net.SocketID != INVALID_SOCKET)
			{
				Net_Disconnect(&RemoteCtrl.Net);
			}
			State = 0;
#endif
			break;
		}

	}
}

void Remote_Config(void)
{
	gSys.TaskID[REMOTE_TASK_ID] = COS_CreateTask(Remote_Task, NULL,
					NULL, MMI_TASK_MAX_STACK_SIZE , MMI_TASK_PRIORITY + REMOTE_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI Remote Task");
	RemoteCtrl.Net.SocketID = INVALID_SOCKET;
	RemoteCtrl.Net.TaskID = gSys.TaskID[REMOTE_TASK_ID];
	RemoteCtrl.Net.Channel = GPRS_CH_REMOTE;
	RemoteCtrl.Net.TimerID = REMOTE_TIMER_ID;
	RemoteCtrl.Net.ReceiveFun = Remote_ReceiveAnalyze;
	RemoteCtrl.Net.TCPPort = REMOTE_PORT;

}
