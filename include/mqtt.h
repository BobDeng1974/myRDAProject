#ifndef __MQTT_H__
#define __MQTT_H__

#define MQTT_MSG_DUP 					(0x08)
#define MQTT_MSG_QOS_MASK				(0x06)
#define MQTT_MSG_QOS2 					(0x04)
#define MQTT_MSG_QOS1 					(0x02)
#define MQTT_MSG_RETAIN 				(0x01)
#define MQTT_CONNECT_FLAG_USER			(0x80)
#define MQTT_CONNECT_FLAG_PASSWD		(0x40)
#define MQTT_CONNECT_FLAG_WILLRETAIN	(0x20)
#define MQTT_CONNECT_FLAG_WILLQOS2		(0x10)
#define MQTT_CONNECT_FLAG_WILLQOS1		(0x08)
#define MQTT_CONNECT_FLAG_WILL			(0x04)
#define MQTT_CONNECT_FLAG_CLEAN			(0x02)
#define MQTT_SUBSCRIBE_QOS2				(0x02)
#define MQTT_SUBSCRIBE_QOS1				(0x01)
enum MQTTENUM
{
	MQTT_CMD_CONNECT = 1,
	MQTT_CMD_CONNACK,
	MQTT_CMD_PUBLISH,
	MQTT_CMD_PUBACK,
	MQTT_CMD_PUBREC,
	MQTT_CMD_PUBREL,
	MQTT_CMD_PUBCOMP,
	MQTT_CMD_SUBSCRIBE,
	MQTT_CMD_SUBACK,
	MQTT_CMD_UNSUBSCRIBE,
	MQTT_CMD_UNSUBACK,
	MQTT_CMD_PINGREQ,
	MQTT_CMD_PINGRESP,
	MQTT_CMD_DISCONNECT,
};

typedef struct
{
	u16 Len;
	u8 *Char;
}MQTT_UTF8Struct;

typedef struct
{
	u8 *Char;
	u8 Qos;
}MQTT_SubscribeStruct;

typedef struct
{
	u8 *Data;
	u8 *String;
	u32 DataLen;
	u16 PackID;
	u8 Cmd;
	u8 Flag;
}MQTT_HeadStruct;

u32 MQTT_EncodeMsg(MQTT_HeadStruct *Head, u8 *Payload, u32 PayloadLen, Buffer_Struct *Buf);
u8* MQTT_DecodeMsg(MQTT_HeadStruct *Head, u32 HeadDataLenMax, u32 *PayloadLen, u8 *RxBuf, u32 RxLen);
u32 MQTT_ConnectMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, u8 Flag, u16 KeepTime,
		const s8 *ClientID,
		const s8 *WillTopic,
		const s8 *User,
		const s8 *Passwd,
		u8 *WillMsgData, u16 WillMsgLen);
u32 MQTT_PublishMsg(Buffer_Struct *TxBuf, u8 Flag, u16 PackID, const s8 *Topic,
		u8 *Payload, u32 PayloadLen);
u32 MQTT_PublishCtrlMsg(Buffer_Struct *TxBuf, u8 Cmd, u16 PackID);
u32 MQTT_SubscribeMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, u16 PackID, MQTT_SubscribeStruct *Topic, u32 TopicNum);
u32 MQTT_SingleMsg(Buffer_Struct *TxBuf, u8 Cmd);
#endif
