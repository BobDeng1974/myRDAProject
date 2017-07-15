#include "user.h"
//使用MQTT协议
#define REMOTE_URL			"app.mqlinks.com"
#define REMOTE_PORT			(1883)
#define MQTT_USER			"mylinks"
#define MQTT_PASSWORD		"mylinks_20160915"
#define MQTT_STATE_TOPIC	"/mqtt/state"
#define MQTT_PUB_TOPIC		"/mqtt/dev/"
#define MQTT_SUB_TOPIC		"/mqtt/user/"
#define MQTT_SEND_TO		(30)
#define MQTT_KEEP_TO		(900)
#define MQTT_TOPIC_LEN_MAX	(128)
enum
{
	REMOTE_STATE_DBG_CONNECT,
	REMOTE_STATE_DBG_MQTT_CONNECT,
	REMOTE_STATE_DBG_MQTT_WAIT_START,
	REMOTE_STATE_DBG_MQTT_RUN,
	REMOTE_STATE_DBG_MQTT_STOP,
	REMOTE_STATE_DBG_MQTT_DISCONNECT,
	REMOTE_STATE_DBG_DISCONNECT,
	REMOTE_STATE_IDLE,
};

typedef struct
{
	Net_CtrlStruct Net;
	MQTT_HeadStruct Rxhead;
	u8 RecBuf[MONITOR_RXBUF_LEN / 2];
	u8 SendBuf[MONITOR_TXBUF_LEN];
	u8 TempBuf[MONITOR_TXBUF_LEN];
	u8 Payload[MONITOR_TXBUF_LEN];
	u8 BackBuf[MONITOR_TXBUF_LEN];
	u32 BackLen;
	Buffer_Struct TxBuf;
	Buffer_Struct PayloadBuf;
	u32 KeepTime;
	u16 PackID;
	u8 IMEIStr[20];
	u8 User[32];
	u8 Password[32];
	u8 StateTopic[MQTT_TOPIC_LEN_MAX/2];
	u8 WillMsg[MQTT_TOPIC_LEN_MAX];
	u8 PubTopic[MQTT_TOPIC_LEN_MAX];
	u8 SubTopic[MQTT_TOPIC_LEN_MAX];
	u8 OnlineType;
	u8 State;
	u8 RxPublishWaitFlag;
}Remote_CtrlStruct;

Remote_CtrlStruct __attribute__((section (".usr_ram"))) RemoteCtrl;

void Remote_PublishAnalyze(void)
{
	u8 *Payload = NULL;
	u32 PayloadLen;

	RemoteCtrl.Rxhead.Cmd = 0;
	Payload = MQTT_DecodeMsg(&RemoteCtrl.Rxhead, MQTT_TOPIC_LEN_MAX, &PayloadLen,
			RemoteCtrl.BackBuf, RemoteCtrl.BackLen);
	RemoteCtrl.PayloadBuf.Pos = 0;
	if (Payload != INVALID_HANDLE_VALUE)
	{
		if (Payload && PayloadLen)
		{
			memcpy(RemoteCtrl.PayloadBuf.Data, Payload, PayloadLen);
			RemoteCtrl.PayloadBuf.Pos = PayloadLen;
		}
	}
	RemoteCtrl.BackLen = 0;
}

s32 Remote_ReceiveAnalyze(void *pData)
{
	u32 RxLen = (u32)pData;
	u32 FinishLen = 0;
	u32 TxLen;
	u8 *Payload = NULL;
	u32 PayloadLen;

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
//		__Trace("MQTT: Rx %d", FinishLen);
//		__HexTrace(RemoteCtrl.RecBuf, FinishLen);
		RemoteCtrl.RecBuf[FinishLen] = 0;
		RemoteCtrl.Rxhead.Cmd = 0;
		Payload = MQTT_DecodeMsg(&RemoteCtrl.Rxhead, MQTT_TOPIC_LEN_MAX, &PayloadLen, RemoteCtrl.RecBuf, FinishLen);
		RemoteCtrl.PayloadBuf.Pos = 0;
		if (Payload != INVALID_HANDLE_VALUE)
		{
			if (RemoteCtrl.Rxhead.Cmd == MQTT_CMD_PUBLISH)
			{
				if (RemoteCtrl.RxPublishWaitFlag)
				{
					__Trace("back!");
					memcpy(RemoteCtrl.BackBuf, RemoteCtrl.RecBuf, FinishLen);
					RemoteCtrl.BackLen = FinishLen;
					return 0;
				}
			}
			if (Payload && PayloadLen)
			{
				memcpy(RemoteCtrl.PayloadBuf.Data, Payload, PayloadLen);
				RemoteCtrl.PayloadBuf.Pos = PayloadLen;
			}
		}
	}
	return 0;
}


u8 Remote_MQTTSend(u32 TxLen, u8 PrintFlag)
{
	if (PrintFlag)
	{
		__Trace("MQTT: Tx %d", TxLen);
		__HexTrace(RemoteCtrl.SendBuf, TxLen);
	}
	if (!TxLen)
	{
		return 0;
	}
	RemoteCtrl.Net.To = MQTT_SEND_TO;
	Net_Send(&RemoteCtrl.Net, RemoteCtrl.SendBuf, TxLen);
	if (RemoteCtrl.Net.Result != NET_RES_SEND_OK)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

s32 Remote_MQTTConnect(void)
{
	u32 TxLen;
	MQTT_SubscribeStruct Sub;
	TxLen = MQTT_ConnectMsg(&RemoteCtrl.TxBuf, &RemoteCtrl.PayloadBuf,
			MQTT_CONNECT_FLAG_CLEAN|MQTT_CONNECT_FLAG_WILL|MQTT_CONNECT_FLAG_WILLQOS1|MQTT_CONNECT_FLAG_USER|MQTT_CONNECT_FLAG_PASSWD,
			MQTT_KEEP_TO, NULL, RemoteCtrl.StateTopic, RemoteCtrl.User, RemoteCtrl.Password, RemoteCtrl.WillMsg,
			strlen(RemoteCtrl.WillMsg));
	if (!Remote_MQTTSend(TxLen, 0))
	{
		RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}

	RemoteCtrl.Net.To = MQTT_SEND_TO;
	Net_WaitReceive(&RemoteCtrl.Net);
	if (RemoteCtrl.Net.Result != NET_RES_UPLOAD)
	{
		DBG("%02x", RemoteCtrl.Net.Result);
		RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}
	if (RemoteCtrl.Rxhead.Cmd != MQTT_CMD_CONNACK)
	{
		DBG("%02x", RemoteCtrl.Rxhead.Cmd);
		RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}
	if (RemoteCtrl.Rxhead.Data[1])
	{
		DBG("%02x %02x", RemoteCtrl.Rxhead.Data[0], RemoteCtrl.Rxhead.Data[1]);
		RemoteCtrl.State = REMOTE_STATE_DBG_MQTT_DISCONNECT;
		return -1;
	}
	RemoteCtrl.TxBuf.Pos = 0;
	RemoteCtrl.PayloadBuf.Pos = 0;
	RemoteCtrl.PackID++;
	Sub.Char = RemoteCtrl.SubTopic;
	Sub.Qos = MQTT_SUBSCRIBE_QOS2;
	TxLen = MQTT_SubscribeMsg(&RemoteCtrl.TxBuf, &RemoteCtrl.PayloadBuf, RemoteCtrl.PackID, &Sub, 1);

	if (!Remote_MQTTSend(TxLen, 0))
	{
		RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}

	RemoteCtrl.Net.To = MQTT_SEND_TO;
	Net_WaitReceive(&RemoteCtrl.Net);
	if (RemoteCtrl.Net.Result != NET_RES_UPLOAD)
	{
		DBG("%02x", RemoteCtrl.Net.Result);
		RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}
	if (RemoteCtrl.Rxhead.Cmd != MQTT_CMD_SUBACK)
	{
		DBG("%02x", RemoteCtrl.Rxhead.Cmd);
		RemoteCtrl.State = REMOTE_STATE_DBG_MQTT_DISCONNECT;
		return -1;
	}
	if (RemoteCtrl.Rxhead.PackID != RemoteCtrl.PackID)
	{
		DBG("%d %d", (u32)RemoteCtrl.Rxhead.PackID, (u32)RemoteCtrl.PackID);
		RemoteCtrl.State = REMOTE_STATE_DBG_MQTT_DISCONNECT;
		return -1;
	}

	switch (RemoteCtrl.PayloadBuf.Data[0])
	{
	case 0:
	case 1:
	case 2:
		RemoteCtrl.State = REMOTE_STATE_DBG_MQTT_WAIT_START;
		return RemoteCtrl.State;
	default:
		DBG("%02x", RemoteCtrl.Rxhead.Cmd);
		RemoteCtrl.State = REMOTE_STATE_DBG_MQTT_DISCONNECT;
		return -1;
	}
}

s32 Remote_MQTTSub(void)
{
	u32 OutLen, TxLen;
	if (memcmp(RemoteCtrl.Rxhead.Data, RemoteCtrl.SubTopic, strlen(RemoteCtrl.SubTopic)))
	{
		DBG("sub topic error");
		return -1;
	}
	RemoteCtrl.PayloadBuf.Data[RemoteCtrl.PayloadBuf.Pos] = 0;
	//__Trace("%s", RemoteCtrl.PayloadBuf.Data);
	if (!memcmp(RemoteCtrl.PayloadBuf.Data, RemoteCtrl.IMEIStr, 15))
	{
		switch (RemoteCtrl.State)
		{
		case REMOTE_STATE_DBG_MQTT_WAIT_START:
			RemoteCtrl.State = REMOTE_STATE_DBG_MQTT_RUN;
			RemoteCtrl.KeepTime = gSys.Var[SYS_TIME] + MQTT_KEEP_TO;
			break;
		case REMOTE_STATE_DBG_MQTT_RUN:
			RemoteCtrl.State = REMOTE_STATE_DBG_MQTT_STOP;
			break;
		default:
			break;
		}
		goto MQTT_SUB_CTRL;
	}
	LV_SMSAnalyze(RemoteCtrl.PayloadBuf.Data, RemoteCtrl.PayloadBuf.Pos, RemoteCtrl.TempBuf, &OutLen);
	if (OutLen)
	{
		DBG("%s", RemoteCtrl.TempBuf);
	}

MQTT_SUB_CTRL:
	switch ( RemoteCtrl.Rxhead.Flag & (MQTT_MSG_QOS1|MQTT_MSG_QOS2) )
	{
	case 0:
		break;
	case MQTT_MSG_QOS1:
		TxLen = MQTT_PublishCtrlMsg(&RemoteCtrl.TxBuf, MQTT_CMD_PUBACK, RemoteCtrl.Rxhead.PackID);
		RemoteCtrl.PackID = RemoteCtrl.Rxhead.PackID;
		if (!Remote_MQTTSend(TxLen, 0))
		{
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}
		break;
	case MQTT_MSG_QOS2:
		TxLen = MQTT_PublishCtrlMsg(&RemoteCtrl.TxBuf, MQTT_CMD_PUBREC, RemoteCtrl.Rxhead.PackID);

		if (!Remote_MQTTSend(TxLen, 0))
		{
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}
		RemoteCtrl.Net.To = MQTT_SEND_TO;
		Net_WaitReceive(&RemoteCtrl.Net);
		if (RemoteCtrl.Net.Result != NET_RES_UPLOAD)
		{
			DBG("%02x", RemoteCtrl.Net.Result);
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}
		if (RemoteCtrl.Rxhead.Cmd != MQTT_CMD_PUBREL)
		{
			DBG("%02x", RemoteCtrl.Rxhead.Cmd);
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}

		TxLen = MQTT_PublishCtrlMsg(&RemoteCtrl.TxBuf, MQTT_CMD_PUBCOMP, RemoteCtrl.Rxhead.PackID);
		RemoteCtrl.PackID = RemoteCtrl.Rxhead.PackID;
		if (!Remote_MQTTSend(TxLen, 0))
		{
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}

		break;
	default:
		RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
		break;
	}
	return 0;
}

s32 Remote_MQTTPub(u8 * Topic, u8 *PubData, u32 PubLen, u8 Dup, u8 Qos, u8 Retain)
{
	u32 OutLen, TxLen;

	RemoteCtrl.PackID++;

	if (Qos)
	{
		TxLen = MQTT_PublishMsg(&RemoteCtrl.TxBuf, Dup|Qos|Retain, RemoteCtrl.PackID, Topic,
				PubData, PubLen);

	}
	else
	{
		TxLen = MQTT_PublishMsg(&RemoteCtrl.TxBuf, Retain, RemoteCtrl.PackID, Topic,
				PubData, PubLen);
	}

	if (RemoteCtrl.State == REMOTE_STATE_DBG_MQTT_WAIT_START)
	{
		if (!Remote_MQTTSend(TxLen, 0))
		{
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}
	}
	else
	{
		if (!Remote_MQTTSend(TxLen, 0))
		{
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}
	}


	if (Qos)
	{
		goto MQTT_PUB_CTRL;
	}

	return 0;
MQTT_PUB_CTRL:

	RemoteCtrl.Net.To = MQTT_SEND_TO;
	Net_WaitReceive(&RemoteCtrl.Net);
	if (RemoteCtrl.Net.Result != NET_RES_UPLOAD)
	{
		DBG("%02x", RemoteCtrl.Net.Result);
		RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}

	if (Qos == MQTT_MSG_QOS1)
	{
		if (RemoteCtrl.Rxhead.Cmd != MQTT_CMD_PUBACK)
		{
			DBG("%02x", RemoteCtrl.Rxhead.Cmd);
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}

		if (RemoteCtrl.Rxhead.PackID != RemoteCtrl.PackID)
		{
			DBG("%d %d", (u32)RemoteCtrl.Rxhead.PackID, (u32)RemoteCtrl.PackID);
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}
		return 0;
	}
	else if (Qos == MQTT_MSG_QOS2)
	{
		if (RemoteCtrl.Rxhead.Cmd != MQTT_CMD_PUBREC)
		{
			DBG("%02x", RemoteCtrl.Rxhead.Cmd);
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}

		if (RemoteCtrl.Rxhead.PackID != RemoteCtrl.PackID)
		{
			DBG("%d %d", (u32)RemoteCtrl.Rxhead.PackID, (u32)RemoteCtrl.PackID);
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}

		TxLen = MQTT_PublishCtrlMsg(&RemoteCtrl.TxBuf, MQTT_CMD_PUBREL, RemoteCtrl.PackID);
		if (!Remote_MQTTSend(TxLen, 0))
		{
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}

		RemoteCtrl.Net.To = MQTT_SEND_TO;
		Net_WaitReceive(&RemoteCtrl.Net);
		if (RemoteCtrl.Net.Result != NET_RES_UPLOAD)
		{
			DBG("%02x", RemoteCtrl.Net.Result);
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}

		if (RemoteCtrl.Rxhead.Cmd != MQTT_CMD_PUBCOMP)
		{
			DBG("%02x", RemoteCtrl.Rxhead.Cmd);
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}

		if (RemoteCtrl.Rxhead.PackID != RemoteCtrl.PackID)
		{
			DBG("%d %d", (u32)RemoteCtrl.Rxhead.PackID, (u32)RemoteCtrl.PackID);
			RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
			return -1;
		}

	}

	return 1;
}

s32 Remote_MQTTHeart(void)
{
	u32 TxLen;
	TxLen = MQTT_SingleMsg(&RemoteCtrl.TxBuf, MQTT_CMD_PINGREQ);
	if (!Remote_MQTTSend(TxLen, 0))
	{
		RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}

	RemoteCtrl.Net.To = MQTT_SEND_TO;
	Net_WaitReceive(&RemoteCtrl.Net);
	if (RemoteCtrl.Net.Result != NET_RES_UPLOAD)
	{
		DBG("%02x", RemoteCtrl.Net.Result);
		RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}

	if (RemoteCtrl.Rxhead.Cmd != MQTT_CMD_PINGRESP)
	{
		DBG("%02x", RemoteCtrl.Rxhead.Cmd);
		RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}
	return 0;
}

void Remote_Task(void *pData)
{
	u8 ConnectOK = 0;
	IP_AddrUnion uIP;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	u32 TxLen;
	u8 HeatBeat = 0;
	IO_ValueUnion Temp;
	COS_EVENT Event = { 0 };
	RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
	RemoteCtrl.OnlineType = 0;
	RemoteCtrl.TxBuf.Data = RemoteCtrl.SendBuf;
	RemoteCtrl.TxBuf.MaxLen = sizeof(RemoteCtrl.SendBuf);
	RemoteCtrl.PayloadBuf.Data = RemoteCtrl.Payload;
	RemoteCtrl.PayloadBuf.MaxLen = sizeof(RemoteCtrl.Payload);
	strcpy(RemoteCtrl.User,  MQTT_USER);
	strcpy(RemoteCtrl.Password,  MQTT_PASSWORD);
	strcpy(RemoteCtrl.StateTopic, MQTT_STATE_TOPIC);
	sprintf(RemoteCtrl.WillMsg, "%09d,%09d,%09d,%s error offline",
			MainInfo->UID[2], MainInfo->UID[1], MainInfo->UID[0], RemoteCtrl.IMEIStr);
	sprintf(RemoteCtrl.PubTopic, "%s%s", MQTT_PUB_TOPIC, RemoteCtrl.IMEIStr);
	sprintf(RemoteCtrl.SubTopic, "%s%s", MQTT_SUB_TOPIC, RemoteCtrl.IMEIStr);
	while(1)
	{
		switch (RemoteCtrl.State)
		{
		case REMOTE_STATE_DBG_CONNECT:
			RemoteCtrl.Net.To = 70;
			if (RemoteCtrl.Net.SocketID != INVALID_SOCKET)
			{
				Net_Disconnect(&RemoteCtrl.Net);
			}
			Net_Connect(&RemoteCtrl.Net, 0, REMOTE_URL);
			if (RemoteCtrl.Net.Result != NET_RES_CONNECT_OK)
			{
				if (RemoteCtrl.Net.SocketID != INVALID_SOCKET)
				{
					Net_Disconnect(&RemoteCtrl.Net);
				}
				RemoteCtrl.State = REMOTE_STATE_IDLE;
			}
			else
			{
				uIP.u32_addr = RemoteCtrl.Net.IPAddr.s_addr;
				DBG("IP %d.%d.%d.%d OK", (u32)uIP.u8_addr[0], (u32)uIP.u8_addr[1],
						(u32)uIP.u8_addr[2], (u32)uIP.u8_addr[3]);
				RemoteCtrl.State = REMOTE_STATE_DBG_MQTT_CONNECT;
			}
			break;
		case REMOTE_STATE_DBG_MQTT_CONNECT:
			RemoteCtrl.RxPublishWaitFlag = 1;
			Remote_MQTTConnect();
			break;
		case REMOTE_STATE_DBG_MQTT_WAIT_START:
			sprintf(RemoteCtrl.TempBuf, "%09d,%09d,%09d,%s online %d",
					MainInfo->UID[2], MainInfo->UID[1], MainInfo->UID[0], RemoteCtrl.IMEIStr, RemoteCtrl.OnlineType);
			RemoteCtrl.RxPublishWaitFlag = 1;
			if (Remote_MQTTPub(RemoteCtrl.StateTopic, RemoteCtrl.TempBuf, strlen(RemoteCtrl.TempBuf),
					0, MQTT_MSG_QOS2, 0) < 0)
			{
				RemoteCtrl.RxPublishWaitFlag = 0;
				break;
			}

			RemoteCtrl.RxPublishWaitFlag = 0;
			if (RemoteCtrl.BackLen)
			{
				Remote_PublishAnalyze();
				Remote_MQTTSub();

			}
			if (RemoteCtrl.State != REMOTE_STATE_DBG_MQTT_WAIT_START)
			{
				break;
			}
			RemoteCtrl.Net.To = MQTT_SEND_TO * 2;
			Net_WaitReceive(&RemoteCtrl.Net);
			switch (RemoteCtrl.Net.Result)
			{
			case NET_RES_ERROR:
				RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
				break;
			case NET_RES_TO:
				RemoteCtrl.State = REMOTE_STATE_DBG_MQTT_STOP;
				break;
			case NET_RES_UPLOAD:
				if (RemoteCtrl.Rxhead.Cmd == MQTT_CMD_PUBLISH)
				{
					Remote_MQTTSub();
				}
				else
				{
					RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
				}
				break;
			}
			break;

		case REMOTE_STATE_DBG_MQTT_RUN:
			if (RemoteCtrl.KeepTime < gSys.Var[SYS_TIME])
			{
				RemoteCtrl.State = REMOTE_STATE_DBG_MQTT_STOP;
				break;
			}
			if (gSys.TraceBuf.Len)
			{
				HeatBeat = 0;
				RemoteCtrl.RxPublishWaitFlag = 1;
				TxLen = QueryRBuffer(&gSys.TraceBuf, RemoteCtrl.TempBuf, 1340);
				if (Remote_MQTTPub(RemoteCtrl.PubTopic, RemoteCtrl.TempBuf, TxLen, 0, 0, 0) < 0)
				{
					RemoteCtrl.RxPublishWaitFlag = 0;
					break;
				}
				else
				{
					RemoteCtrl.RxPublishWaitFlag = 0;
					DelRBuffer(&gSys.TraceBuf, TxLen);
				}
			}
			else
			{
				RemoteCtrl.Net.To = 1;
				Net_WaitEvent(&RemoteCtrl.Net);
				HeatBeat++;
    			if (RemoteCtrl.Net.Result == NET_RES_ERROR)
    			{
    				RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
    			}
    			else if (RemoteCtrl.Net.Result == NET_RES_UPLOAD)
    			{
    				if (RemoteCtrl.Rxhead.Cmd == MQTT_CMD_PUBLISH)
    				{
    					Remote_MQTTSub();
    				}
    			}
    			if (HeatBeat >= MQTT_SEND_TO)
    			{
    				HeatBeat = 0;
    				Remote_MQTTHeart();
    			}
			}
			break;
		case REMOTE_STATE_DBG_MQTT_STOP:
			sprintf(RemoteCtrl.TempBuf, "%09d,%09d,%09d,%s offline",
					MainInfo->UID[2], MainInfo->UID[1], MainInfo->UID[0], RemoteCtrl.IMEIStr);
			RemoteCtrl.RxPublishWaitFlag = 1;
			Remote_MQTTPub(RemoteCtrl.StateTopic, RemoteCtrl.TempBuf, strlen(RemoteCtrl.TempBuf),
					0, MQTT_MSG_QOS2, 0);
			RemoteCtrl.RxPublishWaitFlag = 0;
			RemoteCtrl.State = REMOTE_STATE_DBG_MQTT_DISCONNECT;
			break;
		case REMOTE_STATE_DBG_MQTT_DISCONNECT:
			TxLen = MQTT_SingleMsg(&RemoteCtrl.TxBuf, MQTT_CMD_DISCONNECT);
			RemoteCtrl.Net.To = MQTT_SEND_TO;
			Net_Send(&RemoteCtrl.Net, RemoteCtrl.SendBuf, TxLen);
			RemoteCtrl.State = REMOTE_STATE_DBG_DISCONNECT;
			break;

		case REMOTE_STATE_DBG_DISCONNECT:

			RemoteCtrl.Net.To = MQTT_SEND_TO;
			if (RemoteCtrl.Net.SocketID != INVALID_SOCKET)
			{
				Net_Disconnect(&RemoteCtrl.Net);
			}
			RemoteCtrl.State = REMOTE_STATE_IDLE;
			break;

		default:
			COS_WaitEvent(gSys.TaskID[REMOTE_TASK_ID], &Event, COS_WAIT_FOREVER);
			if (Event.nEventId == EV_MMI_START_REMOTE)
			{
				RemoteCtrl.State = REMOTE_STATE_DBG_CONNECT;
				RemoteCtrl.OnlineType = Event.nParam1;
			}
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
	sprintf(RemoteCtrl.IMEIStr, "%02x%02x%02x%02x%02x%02x%02x%02x", gSys.IMEI[0], gSys.IMEI[1], gSys.IMEI[2],
			gSys.IMEI[3], gSys.IMEI[4], gSys.IMEI[5], gSys.IMEI[6], gSys.IMEI[7]);
	RemoteCtrl.Rxhead.Data = COS_MALLOC(MQTT_TOPIC_LEN_MAX);

}
