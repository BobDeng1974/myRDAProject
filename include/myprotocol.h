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
	u16 MsgLen;
	u8 *MsgData;
}My_MsgBodyStruct;
s32 My_NetDevPacket(u8 *DevID, u8 DevIDLen, u8 MsgID, My_MsgBodyStruct *MsgBody, u8 Qos, u8 *Out);
s32 My_NetDevAuth(u8 *IMEI, u8 *ICCID, My_MsgBodyStruct *MsgBody);
s32 My_NetDevUploadLocatInfo(Monitor_RecordStruct *Record, My_MsgBodyStruct *MsgBody);
s32 My_NetDevResponse(u8 LastMsgID, u16 LastCRC16, u8 Result, My_MsgBodyStruct *MsgBody);
s32 My_NetDevUploadParam(u8 ParamType, u8 ParamSn, My_MsgBodyStruct *MsgBody);
s32 My_NetMonResponse(void *Param);
s32 My_NetMonAuth(void *Param);
s32 My_NetMonForceUpload(void *Param);
s32 My_NetMonGetParam(void *Param);
s32 My_NetMonSetParam(void *Param);
s32 My_NetMonCtrlDev(void *Param);
s32 My_NetMonDebug(void *Param);
#endif
