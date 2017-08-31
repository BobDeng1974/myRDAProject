#include "user.h"
//使用MQTT协议
#define REMOTE_URL			"www.bdclw.net"
#define REMOTE_PORT			(1883)
#define MQTT_USER			"rda8955"
#define MQTT_PASSWORD		"rda8955"
#define MQTT_PUB_TOPIC		"dbg/g/"
#define MQTT_SUB_TOPIC		"dbg/u/"
#define MQTT_SEND_TO		(60)
#define MQTT_KEEP_TO		(900)
#define MQTT_TOPIC_LEN_MAX	(128)
#define __MQTT_DEBUG__

#ifdef __MQTT_DEBUG__
#define MQTT(X, Y...)	__Trace("%s %u:"X, __FUNCTION__, __LINE__, ##Y)
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
	uint8_t RxBuf[MONITOR_TXBUF_LEN];
	uint8_t TxBuf[MONITOR_TXBUF_LEN];
	uint8_t TempBuf[MONITOR_TXBUF_LEN];
	uint8_t Payload[MONITOR_TXBUF_LEN];
	Buffer_Struct TxBuffer;
	Buffer_Struct PayloadBuffer;
	uint32_t OnlineKeepTime;
	uint16_t PackID;
	uint16_t SubPackID;
	uint16_t PubPackID;
	uint8_t IMEIStr[20];
	uint8_t ICCIDStr[24];
	uint8_t User[32];
	uint8_t Password[32];
	uint8_t WillMsg[MQTT_TOPIC_LEN_MAX];
	uint8_t PubTopic[MQTT_TOPIC_LEN_MAX];
	uint8_t SubTopic[MQTT_TOPIC_LEN_MAX];
	uint8_t OnlineType;
	uint8_t State;
	uint8_t SubState;
	uint8_t PubState;
	uint8_t RxFlag;
	uint8_t IsHeart;
}Remote_CtrlStruct;

Remote_CtrlStruct __attribute__((section (".usr_ram"))) RDCtrl;

int32_t Remote_ReceiveAnalyze(void *pData)
{
	uint32_t RxLen = (uint32_t)pData;
	uint32_t FinishLen = 0;
	int32_t Error;
	//uint32_t TxLen;
	uint8_t *Payload = NULL;
	uint32_t PayloadLen;

	while (RxLen)
	{
		if (RxLen > sizeof(RDCtrl.RxBuf))
		{
			FinishLen = sizeof(RDCtrl.RxBuf);
		}
		else
		{
			FinishLen = RxLen;
		}

		Error = OS_SocketReceive(RDCtrl.Net.SocketID, RDCtrl.RxBuf, FinishLen, NULL, NULL);
		if (Error <= 0)
		{
			DBG("%d", Error);
			return -1;
		}
		RxLen -= (uint32_t)Error;
		RDCtrl.RxBuf[FinishLen] = 0;
		RDCtrl.Rxhead.Cmd = 0;
		Payload = MQTT_DecodeMsg(&RDCtrl.Rxhead, MQTT_TOPIC_LEN_MAX, &PayloadLen, RDCtrl.RxBuf, FinishLen);
		RDCtrl.PayloadBuffer.Pos = 0;
		if (Payload != INVALID_HANDLE_VALUE)
		{
			RDCtrl.Rxhead.Data[RDCtrl.Rxhead.DataLen] = 0;
			RDCtrl.RxFlag = 1;
			if (Payload && PayloadLen)
			{
				memcpy(RDCtrl.PayloadBuffer.Data, Payload, PayloadLen);
				RDCtrl.PayloadBuffer.Pos = PayloadLen;
			}
		}
	}
	return 0;
}

uint8_t Remote_MQTTSend(uint32_t TxLen)
{
	if (!TxLen)
	{
		return 0;
	}
	RDCtrl.Net.To = MQTT_SEND_TO;
	Net_Send(&RDCtrl.Net, RDCtrl.TxBuf, TxLen);
	if (RDCtrl.Net.Result != NET_RES_SEND_OK)
	{
		MQTT("%u %u", TxLen, RDCtrl.Net.Result);
		return 0;
	}
	else
	{
		return 1;
	}
}

int32_t Remote_MQTTConnect(void)
{
	uint32_t TxLen;
	MQTT_SubscribeStruct Sub;
	TxLen = MQTT_ConnectMsg(&RDCtrl.TxBuffer, &RDCtrl.PayloadBuffer,
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
	RDCtrl.TxBuffer.Pos = 0;
	RDCtrl.PayloadBuffer.Pos = 0;
	RDCtrl.PackID++;
	Sub.Char = RDCtrl.SubTopic;
	Sub.Qos = MQTT_SUBSCRIBE_QOS2;
	TxLen = MQTT_SubscribeMsg(&RDCtrl.TxBuffer, &RDCtrl.PayloadBuffer, RDCtrl.PackID, &Sub, 1);

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
		MQTT("%u %u", (uint32_t)RDCtrl.Rxhead.PackID, (uint32_t)RDCtrl.PackID);
		RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}

	switch (RDCtrl.PayloadBuffer.Data[0])
	{
	case 0:
	case 1:
	case 2:
		RDCtrl.State = REMOTE_STATE_DBG_MQTT_WAIT_START;
		return RDCtrl.State;
	default:
		MQTT("%02x", RDCtrl.PayloadBuffer.Data[0]);
		RDCtrl.State = REMOTE_STATE_DBG_MQTT_DISCONNECT;
		return -1;
	}
}

int32_t Remote_PayloadAnalyze(void)
{
	uint32_t OutLen;
	RDCtrl.OnlineKeepTime = gSys.Var[SYS_TIME] + MQTT_KEEP_TO;
	RDCtrl.PayloadBuffer.Data[RDCtrl.PayloadBuffer.Pos] = 0;
	//__Trace("%s", RDCtrl.PayloadBuffer.Data);
	if (!memcmp(RDCtrl.PayloadBuffer.Data, RDCtrl.IMEIStr, 16))
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
		LV_SMSAnalyze(RDCtrl.PayloadBuffer.Data, RDCtrl.PayloadBuffer.Pos, RDCtrl.TempBuf, &OutLen);
		if (OutLen)
		{
			DBG("%s", RDCtrl.TempBuf);
		}
	}

	MQTT("%u %u %u", RDCtrl.State, RDCtrl.PubState, RDCtrl.SubState);
	return 1;

}

int32_t Remote_MQTTRxAnalyze(void)
{
	int32_t iRet = -1;
	uint32_t TxLen;
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
			TxLen = MQTT_PublishCtrlMsg(&RDCtrl.TxBuffer, MQTT_CMD_PUBACK, RDCtrl.Rxhead.PackID);
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
				MQTT("%u", RDCtrl.SubState);
				RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
				break;
			}
			Remote_PayloadAnalyze();
			RDCtrl.SubPackID = RDCtrl.Rxhead.PackID;
			TxLen = MQTT_PublishCtrlMsg(&RDCtrl.TxBuffer, MQTT_CMD_PUBREC, RDCtrl.SubPackID);
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
			MQTT("%u", RDCtrl.Rxhead.Flag);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		break;
	case MQTT_CMD_PUBACK:
		if (RDCtrl.PubState != MQTT_PUB_STATE_ACK)
		{
			MQTT("%u", RDCtrl.PubState);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		if (RDCtrl.Rxhead.PackID != RDCtrl.PubPackID)
		{
			MQTT("%u %u", RDCtrl.Rxhead.PackID, RDCtrl.PubPackID);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		RDCtrl.PubState = MQTT_PUB_STATE_IDLE;
		iRet = 1;
		break;

	case MQTT_CMD_PUBREC:
		if (RDCtrl.PubState != MQTT_PUB_STATE_REC)
		{
			MQTT("%u", RDCtrl.PubState);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		if (RDCtrl.Rxhead.PackID != RDCtrl.PubPackID)
		{
			MQTT("%u %u", RDCtrl.Rxhead.PackID, RDCtrl.PubPackID);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		TxLen = MQTT_PublishCtrlMsg(&RDCtrl.TxBuffer, MQTT_CMD_PUBREL, RDCtrl.PubPackID);
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
			MQTT("%u", RDCtrl.SubState);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		if (RDCtrl.Rxhead.PackID != RDCtrl.SubPackID)
		{
			MQTT("%u %u", RDCtrl.Rxhead.PackID, RDCtrl.SubPackID);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		TxLen = MQTT_PublishCtrlMsg(&RDCtrl.TxBuffer, MQTT_CMD_PUBCOMP, RDCtrl.SubPackID);
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
			MQTT("%u", RDCtrl.PubState);
			RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
			break;
		}
		if (RDCtrl.Rxhead.PackID != RDCtrl.PubPackID)
		{
			MQTT("%u %u", RDCtrl.Rxhead.PackID, RDCtrl.PubPackID);
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
		MQTT("%u", RDCtrl.Rxhead.Cmd);
		iRet = 0;
		break;
	}
	MQTT("%u %u %u %u %u", RDCtrl.State, RDCtrl.PubState, RDCtrl.PubPackID, RDCtrl.SubState, RDCtrl.SubPackID);
	return iRet;
}


int32_t Remote_MQTTPub(uint8_t * Topic, uint8_t *PubData, uint32_t PubLen, uint8_t Dup, uint8_t Qos, uint8_t Retain)
{
	uint32_t TxLen;
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
		TxLen = MQTT_PublishMsg(&RDCtrl.TxBuffer, Dup|Qos|Retain, RDCtrl.PackID, Topic,
				PubData, PubLen);

	}
	else
	{
		TxLen = MQTT_PublishMsg(&RDCtrl.TxBuffer, Retain, RDCtrl.PackID, Topic,
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
	MQTT("%u %u %u %u %u", RDCtrl.State, RDCtrl.PubState, RDCtrl.PubPackID, RDCtrl.SubState, RDCtrl.SubPackID);
	return 1;
}

int32_t Remote_MQTTHeart(void)
{
	uint32_t TxLen;
	TxLen = MQTT_SingleMsg(&RDCtrl.TxBuffer, MQTT_CMD_PINGREQ);
	if (!Remote_MQTTSend(TxLen))
	{
		MQTT("!");
		RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
		return -1;
	}
	return 0;
}

int32_t Remote_MQTTWaitFinish(uint32_t To)
{
	uint32_t WaitTo;
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
				MQTT("%u", RDCtrl.Net.Result);
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

void Remote_MQTTPre(void)
{
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	sprintf(RDCtrl.IMEIStr, "%02x%02x%02x%02x%02x%02x%02x%02x", gSys.IMEI[0], gSys.IMEI[1], gSys.IMEI[2],
			gSys.IMEI[3], gSys.IMEI[4], gSys.IMEI[5], gSys.IMEI[6], gSys.IMEI[7]);
	sprintf(RDCtrl.ICCIDStr, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", gSys.ICCID[0], gSys.ICCID[1],
			gSys.ICCID[2],gSys.ICCID[3], gSys.ICCID[4], gSys.ICCID[5], gSys.ICCID[6], gSys.ICCID[7],
			gSys.ICCID[8],gSys.ICCID[9]);
	sprintf(RDCtrl.WillMsg, "%09u%09u%09u,%s,%s error offline",
			(unsigned int)MainInfo->UID[2], (unsigned int)MainInfo->UID[1], (unsigned int)MainInfo->UID[0],
			RDCtrl.IMEIStr, RDCtrl.ICCIDStr);
	sprintf(RDCtrl.PubTopic, "%s%s", MQTT_PUB_TOPIC, RDCtrl.ICCIDStr);
	sprintf(RDCtrl.SubTopic, "%s%s", MQTT_SUB_TOPIC, RDCtrl.ICCIDStr);

}

void Remote_Task(void *pData)
{
	uint8_t FirstFlag = 1;
	IP_AddrUnion uIP;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	uint32_t TxLen;
	uint32_t WaitTo;
	uint8_t ErrorFlag;
	uint8_t HeatBeat = 0;
	uint8_t SendTrace;
	COS_EVENT Event = { 0 };
#ifndef __REMOTE_TRACE_ENABLE__
	while(1)
	{
		COS_WaitEvent(gSys.TaskID[REMOTE_TASK_ID], &Event, COS_WAIT_FOREVER);
	}
#endif
	RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
	RDCtrl.OnlineType = 0;
	RDCtrl.TxBuffer.Data = RDCtrl.TxBuf;
	RDCtrl.TxBuffer.MaxLen = sizeof(RDCtrl.TxBuf);
	RDCtrl.PayloadBuffer.Data = RDCtrl.Payload;
	RDCtrl.PayloadBuffer.MaxLen = sizeof(RDCtrl.Payload);
	strcpy(RDCtrl.User,  MQTT_USER);
	strcpy(RDCtrl.Password,  MQTT_PASSWORD);

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
				MQTT("IP %u.%u.%u.%u OK", (uint32_t)uIP.u8_addr[0], (uint32_t)uIP.u8_addr[1],
						(uint32_t)uIP.u8_addr[2], (uint32_t)uIP.u8_addr[3]);
				RDCtrl.State = REMOTE_STATE_DBG_MQTT_CONNECT;
				if (FirstFlag)
				{
					FirstFlag = 0;
					OS_Sleep(5 * SYS_TICK);
				}
				Remote_MQTTPre();
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
			MQTT("%u %u %u", RDCtrl.State, RDCtrl.PubState, RDCtrl.SubState);

			break;
		case REMOTE_STATE_DBG_MQTT_WAIT_START:
			sprintf(RDCtrl.TempBuf, "%09u%09u%09u,%s,%s,%08x,%u online %u",
					(unsigned int)MainInfo->UID[2], (unsigned int)MainInfo->UID[1], (unsigned int)MainInfo->UID[0],
					RDCtrl.IMEIStr, RDCtrl.ICCIDStr, __GetMainVersion(), gSys.Var[SOFTWARE_VERSION],
					RDCtrl.OnlineType);

			if (Remote_MQTTPub(RDCtrl.PubTopic, RDCtrl.TempBuf, strlen(RDCtrl.TempBuf),
					0, MQTT_MSG_QOS2, 0) < 0)
			{
				MQTT("!");
				RDCtrl.State = REMOTE_STATE_DBG_CONNECT;
				break;
			}
			ErrorFlag = 0;
			WaitTo = gSys.Var[SYS_TIME] + MQTT_SEND_TO  + 15;
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
				//MQTT("%u %u %u", RDCtrl.Net.To, WaitTo, gSys.Var[SYS_TIME]);
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
				MQTT("%u %u", RDCtrl.OnlineKeepTime, gSys.Var[SYS_TIME]);
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
			sprintf(RDCtrl.TempBuf, "%09u%09u%09u,%s,%s offline",
					(unsigned int)MainInfo->UID[2], (unsigned int)MainInfo->UID[1], (unsigned int)MainInfo->UID[0],
					RDCtrl.IMEIStr, RDCtrl.ICCIDStr);

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
			TxLen = MQTT_SingleMsg(&RDCtrl.TxBuffer, MQTT_CMD_DISCONNECT);
			RDCtrl.Net.To = MQTT_SEND_TO;
			Net_Send(&RDCtrl.Net, RDCtrl.TxBuf, TxLen);
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

	RDCtrl.Rxhead.Data = COS_MALLOC(MQTT_TOPIC_LEN_MAX + 8);

}
