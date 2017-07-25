#include "user.h"
//使用MQTT协议
#define REMOTE_URL			"www.bdclw.net"
#define REMOTE_PORT			(1883)
#define MQTT_USER			"rda8955"
#define MQTT_PASSWORD		"rda8955"
#define MQTT_PUB_TOPIC		"dbg/g/"
#define MQTT_SUB_TOPIC		"dbg/u/"
#define MQTT_SEND_TO		(30)
#define MQTT_KEEP_TO		(900)
#define MQTT_TOPIC_LEN_MAX	(128)
#define __MQTT_DEBUG__

#ifdef __MQTT_DEBUG__
#define MQTT(X, Y...)	__Trace("%s %d:"X, __FUNCTION__, __LINE__, ##Y)
#else
#define MQTT(X, Y...)
#endif
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

	MQTT_SUB_STATE_IDLE = 0,
	MQTT_SUB_STATE_ACK,
	MQTT_SUB_STATE_REC,
	MQTT_SUB_STATE_REL,
	MQTT_SUB_STATE_COMP,
	MQTT_PUB_STATE_IDLE = 0,
	MQTT_PUB_STATE_ACK,
	MQTT_PUB_STATE_REC,
	MQTT_PUB_STATE_REL,
	MQTT_PUB_STATE_COMP,

};

typedef struct
{
	Net_CtrlStruct Net;
	MQTT_HeadStruct Rxhead;
	u8 RecBuf[MONITOR_TXBUF_LEN];
	u8 SendBuf[MONITOR_TXBUF_LEN];
	u8 TempBuf[MONITOR_TXBUF_LEN];
	u8 Payload[MONITOR_TXBUF_LEN];
	Buffer_Struct TxBuf;
	Buffer_Struct PayloadBuf;
	u32 OnlineKeepTime;
	u16 PackID;
	u16 SubPackID;
	u16 PubPackID;
	u8 IMEIStr[20];
	u8 User[32];
	u8 Password[32];
	u8 WillMsg[MQTT_TOPIC_LEN_MAX];
	u8 PubTopic[MQTT_TOPIC_LEN_MAX];
	u8 SubTopic[MQTT_TOPIC_LEN_MAX];
	u8 OnlineType;
	u8 State;
	u8 SubState;
	u8 PubState;
	u8 RxFlag;
	u8 IsHeart;
}Remote_CtrlStruct;

Remote_CtrlStruct __attribute__((section (".usr_ram"))) RDCtrl;

s32 Remote_ReceiveAnalyze(void *pData)
{
	u32 RxLen = (u32)pData;
	u32 FinishLen = 0;
	//u32 TxLen;
	u8 *Payload = NULL;
	u32 PayloadLen;

	while (RxLen)
	{
		if (RxLen > sizeof(RDCtrl.RecBuf))
		{
			FinishLen = sizeof(RDCtrl.RecBuf);
		}
		else
		{
			FinishLen = RxLen;
		}

		RxLen -= OS_SocketReceive(RDCtrl.Net.SocketID, RDCtrl.RecBuf, FinishLen, NULL, NULL);
		RDCtrl.RecBuf[FinishLen] = 0;
		RDCtrl.Rxhead.Cmd = 0;
		Payload = MQTT_DecodeMsg(&RDCtrl.Rxhead, MQTT_TOPIC_LEN_MAX, &PayloadLen, RDCtrl.RecBuf, FinishLen);
		RDCtrl.PayloadBuf.Pos = 0;
		if (Payload != INVALID_HANDLE_VALUE)
		{
			RDCtrl.Rxhead.Data[RDCtrl.Rxhead.DataLen] = 0;
			RDCtrl.RxFlag = 1;
			if (Payload && PayloadLen)
			{
				memcpy(RDCtrl.PayloadBuf.Data, Payload, PayloadLen);
				RDCtrl.PayloadBuf.Pos = PayloadLen;
			}
		}
	}
	return 0;
}

u8 Remote_MQTTSend(u32 TxLen)
{
	if (!TxLen)
	{
		return 0;
	}
	RDCtrl.Net.To = MQTT_SEND_TO;
	Net_Send(&RDCtrl.Net, RDCtrl.SendBuf, TxLen);
	if (RDCtrl.Net.Result != NET_RES_SEND_OK)
	{
		MQTT("%d %d", TxLen, RDCtrl.Net.Result);
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
	TxLen = MQTT_ConnectMsg(&RDCtrl.TxBuf, &RDCtrl.PayloadBuf,
			MQTT_CONNECT_FLAG_CLEAN|MQTT_CONNECT_FLAG_WILL|MQTT_CONNECT_FLAG_WILLQOS1|MQTT_CONNECT_FLAG_USER|MQTT_CONNECT_FLAG_PASSWD,
			MQTT_SEND_TO * 2, NULL, RDCtrl.PubTopic, RDCtrl.User, RDCtrl.Password, RDCtrl.WillMsg,
			strlen(RDCtrl.WillMsg));
	if (!Remote_MQTTSend(TxLen))
	{
		MQTT("!");
		RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}

	RDCtrl.Net.To = MQTT_SEND_TO;
	Net_WaitReceive(&RDCtrl.Net);
	if (RDCtrl.Net.Result != NET_RES_UPLOAD)
	{
		MQTT("%02x", RDCtrl.Net.Result);
		RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}
	RDCtrl.RxFlag = 0;
	if (RDCtrl.Rxhead.Cmd != MQTT_CMD_CONNACK)
	{
		MQTT("%02x", RDCtrl.Rxhead.Cmd);
		RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}
	if (RDCtrl.Rxhead.Data[1])
	{
		MQTT("%02x %02x", RDCtrl.Rxhead.Data[0], RDCtrl.Rxhead.Data[1]);
		RDCtrl.State = REMOTE_STATE_DBG_MQTT_DISCONNECT;
		return -1;
	}
	RDCtrl.TxBuf.Pos = 0;
	RDCtrl.PayloadBuf.Pos = 0;
	RDCtrl.PackID++;
	Sub.Char = RDCtrl.SubTopic;
	Sub.Qos = MQTT_SUBSCRIBE_QOS2;
	TxLen = MQTT_SubscribeMsg(&RDCtrl.TxBuf, &RDCtrl.PayloadBuf, RDCtrl.PackID, &Sub, 1);

	if (!Remote_MQTTSend(TxLen))
	{
		MQTT("!");
		RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}

	RDCtrl.Net.To = MQTT_SEND_TO;
	Net_WaitReceive(&RDCtrl.Net);
	if (RDCtrl.Net.Result != NET_RES_UPLOAD)
	{
		MQTT("%02x", RDCtrl.Net.Result);
		RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}
	RDCtrl.RxFlag = 0;
	if (RDCtrl.Rxhead.Cmd != MQTT_CMD_SUBACK)
	{
		MQTT("%02x", RDCtrl.Rxhead.Cmd);
		RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}
	if (RDCtrl.Rxhead.PackID != RDCtrl.PackID)
	{
		MQTT("%d %d", (u32)RDCtrl.Rxhead.PackID, (u32)RDCtrl.PackID);
		RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}

	switch (RDCtrl.PayloadBuf.Data[0])
	{
	case 0:
	case 1:
	case 2:
		RDCtrl.State = REMOTE_STATE_DBG_MQTT_WAIT_START;
		return RDCtrl.State;
	default:
		MQTT("%02x", RDCtrl.PayloadBuf.Data[0]);
		RDCtrl.State = REMOTE_STATE_DBG_MQTT_DISCONNECT;
		return -1;
	}
}

s32 Remote_PayloadAnalyze(void)
{
	u32 OutLen;
	RDCtrl.OnlineKeepTime = gSys.Var[SYS_TIME] + MQTT_KEEP_TO;
	RDCtrl.PayloadBuf.Data[RDCtrl.PayloadBuf.Pos] = 0;
	//__Trace("%s", RDCtrl.PayloadBuf.Data);
	if (!memcmp(RDCtrl.PayloadBuf.Data, RDCtrl.IMEIStr, 16))
	{
		switch (RDCtrl.State)
		{
		case REMOTE_STATE_DBG_MQTT_WAIT_START:
			RDCtrl.State = REMOTE_STATE_DBG_MQTT_RUN;
			break;
		case REMOTE_STATE_DBG_MQTT_RUN:
			RDCtrl.State = REMOTE_STATE_DBG_MQTT_STOP;
			break;
		default:
			break;
		}
	}
	else
	{
		LV_SMSAnalyze(RDCtrl.PayloadBuf.Data, RDCtrl.PayloadBuf.Pos, RDCtrl.TempBuf, &OutLen);
		if (OutLen)
		{
			DBG("%s", RDCtrl.TempBuf);
		}
	}

	MQTT("%d %d %d", RDCtrl.State, RDCtrl.PubState, RDCtrl.SubState);
	return 1;

}

s32 Remote_MQTTRxAnalyze(void)
{
	s32 iRet = -1;
	u32 TxLen;
	RDCtrl.RxFlag = 0;
	switch (RDCtrl.Rxhead.Cmd)
	{
	case MQTT_CMD_PUBLISH:
		if (strcmp(RDCtrl.Rxhead.Data, RDCtrl.SubTopic))
		{
			MQTT("%s", RDCtrl.Rxhead.Data);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}

		switch (RDCtrl.Rxhead.Flag & MQTT_MSG_QOS_MASK)
		{
		case 0:
			Remote_PayloadAnalyze();
			iRet = 1;
			break;
		case MQTT_MSG_QOS1:
			Remote_PayloadAnalyze();
			TxLen = MQTT_PublishCtrlMsg(&RDCtrl.TxBuf, MQTT_CMD_PUBACK, RDCtrl.Rxhead.PackID);
			if (!Remote_MQTTSend(TxLen))
			{
				MQTT("!");
				RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
				break;
			}
			iRet = 1;
			break;
		case MQTT_MSG_QOS2:
			if (RDCtrl.SubState != MQTT_SUB_STATE_IDLE)
			{
				MQTT("%d", RDCtrl.SubState);
				RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
				break;
			}
			Remote_PayloadAnalyze();
			RDCtrl.SubPackID = RDCtrl.Rxhead.PackID;
			TxLen = MQTT_PublishCtrlMsg(&RDCtrl.TxBuf, MQTT_CMD_PUBREC, RDCtrl.SubPackID);
			if (!Remote_MQTTSend(TxLen))
			{
				MQTT("!");
				RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
				break;
			}
			RDCtrl.SubState = MQTT_SUB_STATE_REL;
			iRet = 1;
			break;
		default:
			MQTT("%d", RDCtrl.Rxhead.Flag);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		break;
	case MQTT_CMD_PUBACK:
		if (RDCtrl.PubState != MQTT_PUB_STATE_ACK)
		{
			MQTT("%d", RDCtrl.PubState);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		if (RDCtrl.Rxhead.PackID != RDCtrl.PubPackID)
		{
			MQTT("%d %d", RDCtrl.Rxhead.PackID, RDCtrl.PubPackID);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		RDCtrl.PubState = MQTT_PUB_STATE_IDLE;
		iRet = 1;
		break;

	case MQTT_CMD_PUBREC:
		if (RDCtrl.PubState != MQTT_PUB_STATE_REC)
		{
			MQTT("%d", RDCtrl.PubState);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		if (RDCtrl.Rxhead.PackID != RDCtrl.PubPackID)
		{
			MQTT("%d %d", RDCtrl.Rxhead.PackID, RDCtrl.PubPackID);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		TxLen = MQTT_PublishCtrlMsg(&RDCtrl.TxBuf, MQTT_CMD_PUBREL, RDCtrl.PubPackID);
		if (!Remote_MQTTSend(TxLen))
		{
			MQTT("!");
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		RDCtrl.PubState = MQTT_PUB_STATE_COMP;
		iRet = 1;
		break;
	case MQTT_CMD_PUBREL:
		if (RDCtrl.SubState != MQTT_SUB_STATE_REL)
		{
			MQTT("%d", RDCtrl.SubState);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		if (RDCtrl.Rxhead.PackID != RDCtrl.SubPackID)
		{
			MQTT("%d %d", RDCtrl.Rxhead.PackID, RDCtrl.SubPackID);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		TxLen = MQTT_PublishCtrlMsg(&RDCtrl.TxBuf, MQTT_CMD_PUBCOMP, RDCtrl.SubPackID);
		if (!Remote_MQTTSend(TxLen))
		{
			MQTT("!");
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		RDCtrl.SubState = MQTT_SUB_STATE_IDLE;
		iRet = 1;
		break;
	case MQTT_CMD_PUBCOMP:
		if (RDCtrl.PubState != MQTT_PUB_STATE_COMP)
		{
			MQTT("%d", RDCtrl.PubState);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		if (RDCtrl.Rxhead.PackID != RDCtrl.PubPackID)
		{
			MQTT("%d %d", RDCtrl.Rxhead.PackID, RDCtrl.PubPackID);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		RDCtrl.PubState = MQTT_PUB_STATE_IDLE;
		iRet = 1;
		break;
	case MQTT_CMD_PINGRESP:
		RDCtrl.IsHeart = 1;
		iRet = 1;
		break;
	default:
		MQTT("%d", RDCtrl.Rxhead.Cmd);
		iRet = 0;
		break;
	}
	MQTT("%d %d %d %d %d", RDCtrl.State, RDCtrl.PubState, RDCtrl.PubPackID, RDCtrl.SubState, RDCtrl.SubPackID);
	return iRet;
}


s32 Remote_MQTTPub(u8 * Topic, u8 *PubData, u32 PubLen, u8 Dup, u8 Qos, u8 Retain)
{
	u32 TxLen;
	switch (Qos)
	{
	case 0:
		break;
	case MQTT_MSG_QOS1:
		break;
	case MQTT_MSG_QOS2:
		break;
	default:
		return -1;
	}

	if (Qos)
	{
		RDCtrl.PackID++;
		TxLen = MQTT_PublishMsg(&RDCtrl.TxBuf, Dup|Qos|Retain, RDCtrl.PackID, Topic,
				PubData, PubLen);

	}
	else
	{
		TxLen = MQTT_PublishMsg(&RDCtrl.TxBuf, Retain, RDCtrl.PackID, Topic,
				PubData, PubLen);
	}
	if (!Remote_MQTTSend(TxLen))
	{
		MQTT("!");
		return -1;
	}

	switch (Qos)
	{
	case 0:
		break;
	case MQTT_MSG_QOS1:
		RDCtrl.PubState = MQTT_PUB_STATE_ACK;
		RDCtrl.PubPackID = RDCtrl.PackID;
		break;
	case MQTT_MSG_QOS2:
		RDCtrl.PubState = MQTT_PUB_STATE_REC;
		RDCtrl.PubPackID = RDCtrl.PackID;
		break;

	}
	MQTT("%d %d %d %d %d", RDCtrl.State, RDCtrl.PubState, RDCtrl.PubPackID, RDCtrl.SubState, RDCtrl.SubPackID);
	return 1;
}

s32 Remote_MQTTHeart(void)
{
	u32 TxLen;
	TxLen = MQTT_SingleMsg(&RDCtrl.TxBuf, MQTT_CMD_PINGREQ);
	if (!Remote_MQTTSend(TxLen))
	{
		MQTT("!");
		RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}
	return 0;
}

s32 Remote_MQTTWaitFinish(u32 To)
{
	u32 WaitTo;
	WaitTo = gSys.Var[SYS_TIME] + To;
	while ((RDCtrl.PubState != MQTT_PUB_STATE_IDLE) || (RDCtrl.SubState != MQTT_SUB_STATE_IDLE))
	{
		if (RDCtrl.RxFlag)
		{
			if (Remote_MQTTRxAnalyze() < 0)
			{
				MQTT("!");
				return -1;
			}
		}
		if ((RDCtrl.PubState == MQTT_PUB_STATE_IDLE) && (RDCtrl.SubState == MQTT_SUB_STATE_IDLE))
		{
			return 1;
		}
		if (WaitTo > gSys.Var[SYS_TIME])
		{
			RDCtrl.Net.To = WaitTo - gSys.Var[SYS_TIME] + 2;
			Net_WaitEvent(&RDCtrl.Net);
			if (RDCtrl.Net.Result != NET_RES_UPLOAD)
			{
				MQTT("%d", RDCtrl.Net.Result);
				return -1;
			}
		}
		else
		{
			MQTT("To!");
			return -1;
		}
	}
	return 1;
}

void Remote_Task(void *pData)
{

	IP_AddrUnion uIP;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	u32 TxLen;
	u32 WaitTo;
	u8 ErrorFlag;
	u8 HeatBeat = 0;
	u8 SendTrace;
	COS_EVENT Event = { 0 };
	RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
	RDCtrl.OnlineType = 0;
	RDCtrl.TxBuf.Data = RDCtrl.SendBuf;
	RDCtrl.TxBuf.MaxLen = sizeof(RDCtrl.SendBuf);
	RDCtrl.PayloadBuf.Data = RDCtrl.Payload;
	RDCtrl.PayloadBuf.MaxLen = sizeof(RDCtrl.Payload);
	strcpy(RDCtrl.User,  MQTT_USER);
	strcpy(RDCtrl.Password,  MQTT_PASSWORD);
	sprintf(RDCtrl.WillMsg, "%09d,%09d,%09d,%s error offline",
			(int)MainInfo->UID[2], (int)MainInfo->UID[1], (int)MainInfo->UID[0], RDCtrl.IMEIStr);
	sprintf(RDCtrl.PubTopic, "%s%s", MQTT_PUB_TOPIC, RDCtrl.IMEIStr);
	sprintf(RDCtrl.SubTopic, "%s%s", MQTT_SUB_TOPIC, RDCtrl.IMEIStr);
	SendTrace = 0;
	while(1)
	{
		switch (RDCtrl.State)
		{
		case REMOTE_STATE_DBG_CONNECT:
			RDCtrl.Net.To = 70;
			if (RDCtrl.Net.SocketID != INVALID_SOCKET)
			{
				Net_Disconnect(&RDCtrl.Net);
			}
			Net_Connect(&RDCtrl.Net, 0, REMOTE_URL);
			if (RDCtrl.Net.Result != NET_RES_CONNECT_OK)
			{
				if (RDCtrl.Net.SocketID != INVALID_SOCKET)
				{
					Net_Disconnect(&RDCtrl.Net);
				}
				RDCtrl.State = REMOTE_STATE_IDLE;
			}
			else
			{
				uIP.u32_addr = RDCtrl.Net.IPAddr.s_addr;
				MQTT("IP %d.%d.%d.%d OK", (u32)uIP.u8_addr[0], (u32)uIP.u8_addr[1],
						(u32)uIP.u8_addr[2], (u32)uIP.u8_addr[3]);
				RDCtrl.State = REMOTE_STATE_DBG_MQTT_CONNECT;
			}
			break;
		case REMOTE_STATE_DBG_MQTT_CONNECT:
			RDCtrl.PubState = MQTT_PUB_STATE_IDLE;
			RDCtrl.SubState = MQTT_SUB_STATE_IDLE;
			Remote_MQTTConnect();
			if (SendTrace)
			{
				RDCtrl.State = REMOTE_STATE_DBG_MQTT_RUN;
			}
			MQTT("%d %d %d", RDCtrl.State, RDCtrl.PubState, RDCtrl.SubState);

			break;
		case REMOTE_STATE_DBG_MQTT_WAIT_START:
			sprintf(RDCtrl.TempBuf, "%09d,%09d,%09d,%s online %d",
					(int)MainInfo->UID[2], (int)MainInfo->UID[1], (int)MainInfo->UID[0], RDCtrl.IMEIStr, RDCtrl.OnlineType);

			if (Remote_MQTTPub(RDCtrl.PubTopic, RDCtrl.TempBuf, strlen(RDCtrl.TempBuf),
					0, MQTT_MSG_QOS2, 0) < 0)
			{
				MQTT("!");
				RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
				break;
			}
			ErrorFlag = 0;
			WaitTo = gSys.Var[SYS_TIME] + MQTT_SEND_TO  + MQTT_SEND_TO / 2;
			while (gSys.Var[SYS_TIME] < WaitTo)
			{
				if (RDCtrl.RxFlag)
				{
					if (Remote_MQTTRxAnalyze() < 0)
					{
						ErrorFlag = 1;
						break;
					}
					if (RDCtrl.State == REMOTE_STATE_DBG_MQTT_RUN)
					{
						SendTrace = 1;
						break;
					}
				}
				RDCtrl.Net.To = WaitTo - gSys.Var[SYS_TIME] + 2;
				MQTT("%d %d %d", RDCtrl.Net.To, WaitTo, gSys.Var[SYS_TIME]);
				Net_WaitEvent(&RDCtrl.Net);
				if (RDCtrl.Net.Result == NET_RES_ERROR)
				{
					MQTT("!");
					RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
					ErrorFlag = 1;
					break;
				}
			}

			if (ErrorFlag)
			{
				break;
			}

			if (gSys.Var[SYS_TIME] >= WaitTo)
			{
				RDCtrl.State = REMOTE_STATE_DBG_MQTT_STOP;
			}
			break;

		case REMOTE_STATE_DBG_MQTT_RUN:
			if (RDCtrl.OnlineKeepTime < gSys.Var[SYS_TIME])
			{
				MQTT("%d %d", RDCtrl.OnlineKeepTime, gSys.Var[SYS_TIME]);
				RDCtrl.State = REMOTE_STATE_DBG_MQTT_STOP;
				break;
			}
			if (Remote_MQTTWaitFinish(MQTT_SEND_TO) < 0)
			{
				MQTT("!");
				RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
				break;
			}

			if (gSys.TraceBuf.Len)
			{
				HeatBeat = 0;
				ErrorFlag = 0;
				TxLen = QueryRBuffer(&gSys.TraceBuf, RDCtrl.TempBuf, 1340);
				if (Remote_MQTTPub(RDCtrl.PubTopic, RDCtrl.TempBuf, TxLen, 0, 0, 0) < 0)
				{
					MQTT("!");
					RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
					break;
				}

				if (RDCtrl.PubState != MQTT_PUB_STATE_IDLE)
				{
					if (Remote_MQTTWaitFinish(MQTT_SEND_TO) < 0)
					{
						MQTT("!");
						RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
						break;
					}
					else
					{
						DelRBuffer(&gSys.TraceBuf, TxLen);
					}
				}
				else
				{
					DelRBuffer(&gSys.TraceBuf, TxLen);
				}
			}
			else
			{
				RDCtrl.Net.To = 1;
				Net_WaitEvent(&RDCtrl.Net);
				HeatBeat++;
    			if (RDCtrl.Net.Result == NET_RES_ERROR)
    			{
    				MQTT("!");
    				RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
    			}
    			if (RDCtrl.RxFlag)
    			{
    				HeatBeat = 0;
					if (Remote_MQTTRxAnalyze() < 0)
					{
						ErrorFlag = 1;
						break;
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
			SendTrace = 0;
			Remote_MQTTWaitFinish(MQTT_SEND_TO);
			sprintf(RDCtrl.TempBuf, "%09d,%09d,%09d,%s offline",
					(int)MainInfo->UID[2], (int)MainInfo->UID[1], (int)MainInfo->UID[0], RDCtrl.IMEIStr);

			if (Remote_MQTTPub(RDCtrl.PubTopic, RDCtrl.TempBuf, strlen(RDCtrl.TempBuf),
					0, MQTT_MSG_QOS2, 0) < 0)
			{
				RDCtrl.State = REMOTE_STATE_DBG_DISCONNECT;
				break;
			}
			Remote_MQTTWaitFinish(MQTT_SEND_TO);
			RDCtrl.State = REMOTE_STATE_DBG_MQTT_DISCONNECT;
			break;
		case REMOTE_STATE_DBG_MQTT_DISCONNECT:
			TxLen = MQTT_SingleMsg(&RDCtrl.TxBuf, MQTT_CMD_DISCONNECT);
			RDCtrl.Net.To = MQTT_SEND_TO;
			Net_Send(&RDCtrl.Net, RDCtrl.SendBuf, TxLen);
			RDCtrl.State = REMOTE_STATE_DBG_DISCONNECT;
			break;

		case REMOTE_STATE_DBG_DISCONNECT:

			RDCtrl.Net.To = MQTT_SEND_TO;
			if (RDCtrl.Net.SocketID != INVALID_SOCKET)
			{
				Net_Disconnect(&RDCtrl.Net);
			}
			RDCtrl.State = REMOTE_STATE_IDLE;
			break;

		default:
			SendTrace = 0;
			COS_WaitEvent(gSys.TaskID[REMOTE_TASK_ID], &Event, COS_WAIT_FOREVER);
			if (Event.nEventId == EV_MMI_START_REMOTE)
			{
				MQTT("!");
				RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
				RDCtrl.OnlineType = Event.nParam1;
			}
			break;
		}

	}
}

void Remote_Config(void)
{
	gSys.TaskID[REMOTE_TASK_ID] = COS_CreateTask(Remote_Task, NULL,
					NULL, MMI_TASK_MAX_STACK_SIZE , MMI_TASK_PRIORITY + REMOTE_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI Remote Task");
	RDCtrl.Net.SocketID = INVALID_SOCKET;
	RDCtrl.Net.TaskID = gSys.TaskID[REMOTE_TASK_ID];
	RDCtrl.Net.Channel = GPRS_CH_REMOTE;
	RDCtrl.Net.TimerID = REMOTE_TIMER_ID;
	RDCtrl.Net.ReceiveFun = Remote_ReceiveAnalyze;
	RDCtrl.Net.TCPPort = REMOTE_PORT;
	sprintf(RDCtrl.IMEIStr, "%02x%02x%02x%02x%02x%02x%02x%02x", gSys.IMEI[0], gSys.IMEI[1], gSys.IMEI[2],
			gSys.IMEI[3], gSys.IMEI[4], gSys.IMEI[5], gSys.IMEI[6], gSys.IMEI[7]);
	RDCtrl.Rxhead.Data = COS_MALLOC(MQTT_TOPIC_LEN_MAX + 8);

}
