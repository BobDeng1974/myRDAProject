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
	uint16_t Len;
	uint8_t *Char;
}MQTT_UTF8Struct;

typedef struct
{
	uint8_t *Char;
	uint8_t Qos;
}MQTT_SubscribeStruct;

typedef struct
{
	uint8_t *Data;
	uint8_t *String;
	uint32_t DataLen;
	uint16_t PackID;
	uint8_t Cmd;
	uint8_t Flag;
}MQTT_HeadStruct;

uint32_t MQTT_EncodeMsg(MQTT_HeadStruct *Head, uint8_t *Payload, uint32_t PayloadLen, Buffer_Struct *Buf);
uint8_t* MQTT_DecodeMsg(MQTT_HeadStruct *Head, uint32_t HeadDataLenMax, uint32_t *PayloadLen, uint8_t *RxBuf, uint32_t RxLen);
uint32_t MQTT_ConnectMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, uint8_t Flag, uint16_t KeepTime,
		const int8_t *ClientID,
		const int8_t *WillTopic,
		const int8_t *User,
		const int8_t *Passwd,
		uint8_t *WillMsgData, uint16_t WillMsgLen);
uint32_t MQTT_PublishMsg(Buffer_Struct *TxBuf, uint8_t Flag, uint16_t PackID, const int8_t *Topic,
		uint8_t *Payload, uint32_t PayloadLen);
uint32_t MQTT_PublishCtrlMsg(Buffer_Struct *TxBuf, uint8_t Cmd, uint16_t PackID);
uint32_t MQTT_SubscribeMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, uint16_t PackID, MQTT_SubscribeStruct *Topic, uint32_t TopicNum);
uint32_t MQTT_UnSubscribeMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, uint16_t PackID, MQTT_SubscribeStruct *Topic, uint32_t TopicNum);
uint32_t MQTT_SingleMsg(Buffer_Struct *TxBuf, uint8_t Cmd);
#endif
