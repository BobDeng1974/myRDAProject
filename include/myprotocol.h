#ifndef __MY_PROTOCOL_H__
#define __MY_PROTOCOL_H__

enum MY_NET_PROTOCOL_ENUM
{
	NP_DEV_RESPONSE = 0x01,
	NP_DEV_HEART,
	NP_DEV_AUTH,
	NP_DEV_UPLOAD_LOCAT,
	NP_DEV_UPLOAD_PARAM,
	NP_DEV_UPLOAD_INFO,
	NP_DEV_DEBUG,

	NP_MON_RESPONSE = 0x81,
	NP_MON_AUTH,
	NP_MON_FORCE_UPLOAD,
	NP_MON_GET_PARAM,
	NP_MON_SET_PARAM,
	NP_MON_CTRL_DEV,
	NP_MON_DEBUG,
};


typedef struct
{
	uint16_t MsgLen;
	uint8_t *MsgData;
}My_MsgBodyStruct;
int32_t My_NetDevPacket(uint8_t *DevID, uint8_t DevIDLen, uint8_t MsgID, My_MsgBodyStruct *MsgBody, uint8_t Qos, uint8_t *Out);
int32_t My_NetDevAuth(uint8_t *IMEI, uint8_t *ICCID, My_MsgBodyStruct *MsgBody);
int32_t My_NetDevUploadLocatInfo(Monitor_RecordStruct *Record, My_MsgBodyStruct *MsgBody);
int32_t My_NetDevResponse(uint8_t LastMsgID, uint16_t LastCRC16, uint8_t Result, My_MsgBodyStruct *MsgBody);
int32_t My_NetDevUploadParam(uint8_t ParamType, uint8_t ParamSn, My_MsgBodyStruct *MsgBody);
int32_t My_NetMonResponse(void *Param);
int32_t My_NetMonAuth(void *Param);
int32_t My_NetMonForceUpload(void *Param);
int32_t My_NetMonGetParam(void *Param);
int32_t My_NetMonSetParam(void *Param);
int32_t My_NetMonCtrlDev(void *Param);
int32_t My_NetMonDebug(void *Param);
#endif
