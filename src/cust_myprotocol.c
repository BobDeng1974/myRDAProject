#if 0
#include "user.h"

int32_t My_NetDevPacket(uint8_t *DevID, uint8_t DevIDLen, uint8_t MsgID, My_MsgBodyStruct *MsgBody, uint8_t Qos, uint8_t *Out)
{
	//uint8_t Temp[1460];
	uint16_t Pos = 0;
	uint16_t CRC16;
	uint16_t MsgLen = DevIDLen + MsgBody->MsgLen + 6;
	uint32_t TxLen;
	uint8_t *Temp = COS_MALLOC(1460);
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
	return (int32_t)TxLen;
}

int32_t My_NetDevAuth(uint8_t *IMEI, uint8_t *ICCID, My_MsgBodyStruct *MsgBody)
{
	uint16_t Pos = 0;
	memcpy(MsgBody->MsgData + Pos, IMEI, 8);
	Pos += 8;
	memcpy(MsgBody->MsgData + Pos, ICCID, 10);
	Pos += 10;


}

int32_t My_NetDevUploadLocatInfo(Monitor_RecordStruct *Record, My_MsgBodyStruct *MsgBody)
{

}

int32_t My_NetDevResponse(uint8_t LastMsgID, uint16_t LastCRC16, uint8_t Result, My_MsgBodyStruct *MsgBody)
{
	MsgBody->MsgData[0] = LastMsgID;
	MsgBody->MsgData[1] = LastCRC16 >> 8;
	MsgBody->MsgData[2] = LastCRC16 & 0x00ff;
	MsgBody->MsgData[0] = Result;
	MsgBody->MsgLen = 4;
	return 0;
}

int32_t My_NetDevUploadParam(uint8_t ParamType, uint8_t ParamSn, My_MsgBodyStruct *MsgBody)
{

}

int32_t My_NetMonResponse(void *Param);
int32_t My_NetMonAuth(void *Param);
int32_t My_NetMonForceUpload(void *Param);
int32_t My_NetMonGetParam(void *Param);
int32_t My_NetMonSetParam(void *Param);
int32_t My_NetMonCtrlDev(void *Param);
int32_t My_NetMonDebug(void *Param);
#endif
