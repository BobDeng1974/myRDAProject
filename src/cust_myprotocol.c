#if 0
#include "user.h"

s32 My_NetDevPacket(u8 *DevID, u8 DevIDLen, u8 MsgID, My_MsgBodyStruct *MsgBody, u8 Qos, u8 *Out)
{
	//u8 Temp[1460];
	u16 Pos = 0;
	u16 CRC16;
	u16 MsgLen = DevIDLen + MsgBody->MsgLen + 6;
	u32 TxLen;
	u8 *Temp = COS_MALLOC(1460);
	Temp[0] = MsgLen >> 8;
	Temp[1] = MsgLen & 0x00ff;
	Temp[2] = Qos;
	Temp[3] = MsgID;
	Pos = 4;
	memcpy(Temp + Pos, DevID, DevIDLen);
	Pos += DevIDLen;
	memcpy(Temp + Pos, MsgBody->MsgData, MsgBody->MsgLen);
	Pos += MsgBody->MsgLen;
	CRC16 = CRC16Cal(Temp, Pos, CRC16_START, CRC16_GEN);
	Temp[Pos++] = CRC16 >> 8;
	Temp[Pos++] = CRC16 & 0x00ff;
	TxLen = TransferPack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, Temp, MsgLen, Out);
	COS_FREE(Temp);
	return (s32)TxLen;
}

s32 My_NetDevAuth(u8 *IMEI, u8 *ICCID, My_MsgBodyStruct *MsgBody)
{
	u16 Pos = 0;
	memcpy(MsgBody->MsgData + Pos, IMEI, 8);
	Pos += 8;
	memcpy(MsgBody->MsgData + Pos, ICCID, 10);
	Pos += 10;


}

s32 My_NetDevUploadLocatInfo(Monitor_RecordStruct *Record, My_MsgBodyStruct *MsgBody)
{

}

s32 My_NetDevResponse(u8 LastMsgID, u16 LastCRC16, u8 Result, My_MsgBodyStruct *MsgBody)
{
	MsgBody->MsgData[0] = LastMsgID;
	MsgBody->MsgData[1] = LastCRC16 >> 8;
	MsgBody->MsgData[2] = LastCRC16 & 0x00ff;
	MsgBody->MsgData[0] = Result;
	MsgBody->MsgLen = 4;
	return 0;
}

s32 My_NetDevUploadParam(u8 ParamType, u8 ParamSn, My_MsgBodyStruct *MsgBody)
{

}

s32 My_NetMonResponse(void *Param);
s32 My_NetMonAuth(void *Param);
s32 My_NetMonForceUpload(void *Param);
s32 My_NetMonGetParam(void *Param);
s32 My_NetMonSetParam(void *Param);
s32 My_NetMonCtrlDev(void *Param);
s32 My_NetMonDebug(void *Param);
#endif
