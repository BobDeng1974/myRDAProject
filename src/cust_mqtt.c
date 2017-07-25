#include "user.h"

u32 MQTT_AddUFT8String(Buffer_Struct *Buf, const s8 *String)
{
	u16 Strlen = strlen(String);
	u16 wTemp = htons(Strlen);
	if (Buf->MaxLen >= (Buf->Pos + Strlen + 2))
	{
		memcpy(Buf->Data + Buf->Pos, &wTemp, 2);
		memcpy(Buf->Data + Buf->Pos + 2, String, Strlen);
		Buf->Pos += Strlen + 2;
		return Buf->Pos;
	}
	else
	{
		return 0;
	}
}

u32 MQTT_EncodeMsg(MQTT_HeadStruct *Head, u8 *Payload, u32 PayloadLen, Buffer_Struct *Buf)
{
	u32 MsgLen = Head->DataLen + PayloadLen;
	u8 AddPackID = 0;
	if (Buf->MaxLen < (Head->DataLen + PayloadLen + 7))
	{
		DBG("buf len no enough");
		return 0;
	}

	if (MsgLen > 256 * 1024 * 1024)
	{
		DBG("%u", MsgLen);
		return 0;
	}
	switch (Head->Cmd)
	{
	case MQTT_CMD_PUBLISH:
		Buf->Data[0] = (Head->Cmd << 4) | Head->Flag;
		if (Buf->Data[0] & MQTT_MSG_QOS_MASK)
		{
			AddPackID = 1;
			MsgLen += 2;
		}
		break;
	case MQTT_CMD_PUBREL:
	case MQTT_CMD_SUBSCRIBE:
	case MQTT_CMD_UNSUBSCRIBE:
		Buf->Data[0] = (Head->Cmd << 4) | MQTT_MSG_QOS1;
		AddPackID = 1;
		MsgLen += 2;
		break;
	case MQTT_CMD_PUBACK:
	case MQTT_CMD_PUBREC:
	case MQTT_CMD_PUBCOMP:
		Buf->Data[0] = (Head->Cmd << 4) ;
		AddPackID = 1;
		MsgLen += 2;
		break;
	default:
		Buf->Data[0] = (Head->Cmd << 4);
		break;
	}

	Buf->Pos = 1;
	do
	{
		Buf->Data[Buf->Pos] = MsgLen % 128;
		MsgLen = MsgLen / 128;
		if (MsgLen > 0)
		{
			Buf->Data[Buf->Pos] |= 0x80;
		}
		Buf->Pos++;
	}while((MsgLen > 0) && (Buf->Pos <= 4));

	if (Head->DataLen)
	{
		if (Head->Data)
		{
			memcpy(Buf->Data + Buf->Pos, Head->Data, Head->DataLen);
			Buf->Pos += Head->DataLen;
		}
		else if (Head->String)
		{
			MQTT_AddUFT8String(Buf, Head->String);
		}
		else
		{
			DBG("!");
			return 0;
		}
	}

	if (AddPackID)
	{
		memcpy(Buf->Data + Buf->Pos, &Head->PackID, 2);
		Buf->Pos += 2;
	}


	if (Payload && PayloadLen)
	{
		memcpy(Buf->Data + Buf->Pos, Payload, PayloadLen);
		Buf->Pos += PayloadLen;
	}

	return Buf->Pos;
}

u8* MQTT_DecodeMsg(MQTT_HeadStruct *Head, u32 HeadDataLenMax, u32 *PayloadLen, u8 *RxBuf, u32 RxLen)
{
	u32 MsgLen = 0;
	u32 HeadDataLen = 0;
	u32 Pos;
	u8 *Payload = NULL;
	if (HeadDataLenMax < 2)
		return INVALID_HANDLE_VALUE;

	Head->Cmd = RxBuf[0] >> 4;
	Head->Flag = RxBuf[0] & 0x0f;
	if ( (Head->Flag & MQTT_MSG_QOS_MASK) == MQTT_MSG_QOS_MASK)
	{
		DBG("%02x", Head->Flag);
		return INVALID_HANDLE_VALUE;
	}
	Pos = 1;
	do
	{
		MsgLen += (RxBuf[Pos] & 0x7f) * pow(128, Pos - 1);
		if (RxBuf[Pos] & 0x80)
		{
			if ( (Pos >= RxLen) || (Pos >= 4) )
			{
				DBG("%u %u %02x", RxLen, Pos, RxBuf[Pos]);
				return INVALID_HANDLE_VALUE;
			}
			else
			{
				Pos++;
			}
		}
		else
		{
			Pos++;
			break;
		}

	}while ( (Pos < RxLen) && (Pos <= 4) );

	//__Trace("%u", MsgLen);

	if ( (MsgLen + Pos) != RxLen)
	{
		DBG("%u %u %u", MsgLen, Pos, RxLen);
		return INVALID_HANDLE_VALUE;
	}

	switch (Head->Cmd)
	{


	case MQTT_CMD_PUBLISH:
		//获取主题
		HeadDataLen = RxBuf[Pos];
		HeadDataLen = (HeadDataLen << 8) + RxBuf[Pos + 1];
		Pos += 2;
		if (HeadDataLen > HeadDataLenMax)
		{
			return INVALID_HANDLE_VALUE;
		}
		Head->DataLen = HeadDataLen;
		HeadDataLen = 2;

		memcpy(Head->Data, &RxBuf[Pos], Head->DataLen);
		Pos += Head->DataLen;
		HeadDataLen += Head->DataLen;

		if (Head->Flag & MQTT_MSG_QOS_MASK)
		{
			memcpy(&Head->PackID, RxBuf + Pos, 2);
			Head->PackID = htons(Head->PackID);
			Pos += 2;
			HeadDataLen += 2;
		}

		if (MsgLen > HeadDataLen)
		{
			Payload = &RxBuf[Pos];
			*PayloadLen = (MsgLen - HeadDataLen);
		}
		else
		{
			*PayloadLen = 0;
		}
		break;

	case MQTT_CMD_PINGRESP:
		if (MsgLen)
		{
			return INVALID_HANDLE_VALUE;
		}
		Head->DataLen = 0;
		*PayloadLen = 0;
		break;

	case MQTT_CMD_CONNACK:
		Head->DataLen = 2;
		memcpy(Head->Data, &RxBuf[Pos], Head->DataLen);
		*PayloadLen = 0;
		break;
	case MQTT_CMD_PUBACK:
	case MQTT_CMD_PUBREC:
	case MQTT_CMD_PUBREL:
	case MQTT_CMD_PUBCOMP:
		if ( (MsgLen != 2) || (Pos != 2))
		{
			DBG("%u %u", MsgLen, Pos);
		}

		memcpy(&Head->PackID, RxBuf + Pos, 2);
		Head->PackID = htons(Head->PackID);
		Head->DataLen = 0;
		*PayloadLen = 0;
		break;
	case MQTT_CMD_SUBACK:
		if ( (MsgLen != 3) || (Pos != 2) )
		{
			DBG("%u %u", MsgLen, Pos);
		}
		memcpy(&Head->PackID, RxBuf + Pos, 2);
		Head->PackID = htons(Head->PackID);
		Head->DataLen = 0;
		Pos += 2;
		Payload = &RxBuf[Pos];
		*PayloadLen = (MsgLen - 2);
		break;
	default:
		return INVALID_HANDLE_VALUE;
	}
	return Payload;
}

u32 MQTT_ConnectMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, u8 Flag, u16 KeepTime,
		const s8 *ClientID,
		const s8 *WillTopic,
		const s8 *User,
		const s8 *Passwd,
		u8 *WillMsgData, u16 WillMsgLen)
{
	u8 MsgHeadBuf[10] = {0, 4, 'M', 'Q', 'T', 'T', 4, 0, 0, 0};
	MQTT_HeadStruct Head;
	u16 wTemp;
	PayloadBuf->Pos = 0;
	if (ClientID)
	{
		if (!MQTT_AddUFT8String(PayloadBuf, ClientID))
		{
			DBG("!");
			return 0;
		}
	}
	else
	{
		memset(PayloadBuf->Data, 0, 2);
		PayloadBuf->Pos = 2;
	}

	memset(&Head, 0, sizeof(Head));
	Head.Cmd = MQTT_CMD_CONNECT;
	Head.Data = MsgHeadBuf;
	Head.DataLen = 10;
	MsgHeadBuf[7] = Flag;
	MsgHeadBuf[8] = (KeepTime >> 8);
	MsgHeadBuf[9] = KeepTime & 0x00ff;

	if (Flag & MQTT_CONNECT_FLAG_WILL)
	{
		if (!WillTopic || !WillMsgData || !WillMsgLen)
		{
			DBG("no will topic or msg!");
			return 0;
		}

		if (!MQTT_AddUFT8String(PayloadBuf, WillTopic))
		{
			DBG("!");
			return 0;
		}

		wTemp = htons(WillMsgLen);
		if (PayloadBuf->MaxLen >= (PayloadBuf->Pos + WillMsgLen + 2))
		{
			memcpy(PayloadBuf->Data + PayloadBuf->Pos, &wTemp, 2);
			memcpy(PayloadBuf->Data + PayloadBuf->Pos + 2, WillMsgData, WillMsgLen);
			PayloadBuf->Pos += WillMsgLen + 2;
		}
		else
		{
			return 0;
		}
	}

	if (Flag & MQTT_CONNECT_FLAG_USER)
	{
		if (!User)
		{
			DBG("no user string!");
			return 0;
		}

		if (!MQTT_AddUFT8String(PayloadBuf, User))
		{
			DBG("!");
			return 0;
		}
	}

	if (Flag & MQTT_CONNECT_FLAG_PASSWD)
	{
		if (!Passwd)
		{
			DBG("no password string!");
			return 0;
		}

		if (!MQTT_AddUFT8String(PayloadBuf, Passwd))
		{
			DBG("!");
			return 0;
		}
	}

	return MQTT_EncodeMsg(&Head, PayloadBuf->Data, PayloadBuf->Pos, TxBuf);
}

u32 MQTT_PublishMsg(Buffer_Struct *TxBuf, u8 Flag, u16 PackID, const s8 *Topic,
		u8 *Payload, u32 PayloadLen)
{
	MQTT_HeadStruct Head;
	memset(&Head, 0, sizeof(Head));
	Head.Cmd = MQTT_CMD_PUBLISH;
	Head.Flag = Flag;
	Head.DataLen = strlen(Topic) + 2;
	Head.String = (u8 *)Topic;
	Head.PackID = htons(PackID);
	return MQTT_EncodeMsg(&Head, Payload, PayloadLen, TxBuf);
}

u32 MQTT_PublishCtrlMsg(Buffer_Struct *TxBuf, u8 Cmd, u16 PackID)
{
	MQTT_HeadStruct Head;
	memset(&Head, 0, sizeof(Head));
	Head.Cmd = Cmd;
	Head.DataLen = 0;
	Head.Data = NULL;
	Head.PackID = htons(PackID);
	return MQTT_EncodeMsg(&Head, NULL, 0, TxBuf);
}

u32 MQTT_SubscribeMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, u16 PackID, MQTT_SubscribeStruct *Topic, u32 TopicNum)
{
	MQTT_HeadStruct Head;
	u32 i;
	memset(&Head, 0, sizeof(Head));
	Head.Cmd = MQTT_CMD_SUBSCRIBE;
	Head.DataLen = 0;
	Head.Data = NULL;
	Head.PackID = htons(PackID);
	PayloadBuf->Pos = 0;
	for(i = 0; i < TopicNum; i++)
	{
		if (!MQTT_AddUFT8String(PayloadBuf, Topic[i].Char))
		{
			DBG("!");
			return 0;
		}
		if (PayloadBuf->Pos >= PayloadBuf->MaxLen)
		{
			return 0;
		}
		PayloadBuf->Data[PayloadBuf->Pos++] = Topic[i].Qos;
	}
	return MQTT_EncodeMsg(&Head, PayloadBuf->Data, PayloadBuf->Pos, TxBuf);
}

u32 MQTT_SingleMsg(Buffer_Struct *TxBuf, u8 Cmd)
{
	MQTT_HeadStruct Head;
	memset(&Head, 0, sizeof(Head));
	Head.Cmd = Cmd;
	return MQTT_EncodeMsg(&Head, NULL, 0, TxBuf);
}
