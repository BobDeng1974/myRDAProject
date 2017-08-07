#include "user.h"

/*
 * 808协议不分包添加包头
 * MsgID 	消息 ID
 * MsgSn 	消息流水号
 * SimID 	终端手机号
 * MsgLen	消息体长度
 * MsgRSA	消息体经过 RSA 算法加密
 * Buf		输出缓存
 */
uint32_t JTT_PacketHead(uint16_t MsgID, uint16_t MsgSn, uint8_t *SimID, uint16_t MsgLen, uint16_t MsgRSA, uint8_t *Buf)
{
	uint32_t Pos = 0;
	uint16_t Temp;
	MsgID = htons(MsgID);
	MsgSn = htons(MsgSn);
	MsgLen = (MsgLen & 0x03ff);
	Temp = MsgLen + ((MsgRSA)?JTT_MSG_RSA_FLAG:0);
	Temp = htons(Temp);
	memcpy(Buf + Pos, &MsgID, 2);
	Pos += 2;
	memcpy(Buf + Pos, &Temp, 2);
	Pos += 2;
	memcpy(Buf + Pos, SimID, 6);
	Pos += 6;
	memcpy(Buf + Pos, &MsgSn, 2);
	Pos += 2;
	return Pos;
}

/*
 * 808协议分包添加包头
 * MsgID 		消息 ID
 * MsgSn 		消息流水号
 * SimID 		终端手机号
 * MsgLen		消息体长度
 * MsgRSA		消息体经过 RSA 算法加密
 * PacketNum 	分包总数
 * PacketSn		第几个分包
 * Buf			输出缓存
 */
uint32_t JTT_MuiltPacketHead(uint16_t MsgID, uint16_t MsgSn, uint8_t *SimID, uint16_t MsgLen, uint16_t MsgRSA, uint16_t PacketNum, uint16_t PacketSn, uint8_t *Buf)
{
	uint32_t Pos = 0;
	uint16_t Temp;
	MsgID = htons(MsgID);
	MsgSn = htons(MsgSn);
	PacketNum = htons(PacketNum);
	PacketSn = htons(PacketSn);
	MsgLen = (MsgLen & 0x03ff);
	Temp = MsgLen + (MsgRSA)?JTT_MSG_RSA_FLAG:0;
	Temp = htons(Temp);
	memcpy(Buf + Pos, &MsgID, 2);
	Pos += 2;
	memcpy(Buf + Pos, &Temp, 2);
	Pos += 2;
	memcpy(Buf + Pos, SimID, 6);
	Pos += 6;
	memcpy(Buf + Pos, &MsgSn, 2);
	Pos += 2;
	memcpy(Buf + Pos, &PacketNum, 2);
	Pos += 2;
	memcpy(Buf + Pos, &PacketSn, 2);
	Pos += 2;
	return Pos;
}

int32_t JTT_AnalyzeHead(uint16_t *MsgID, uint16_t *MsgSn, uint8_t *SimID, uint8_t *InBuf, uint16_t InLen, uint32_t *RxLen)
{
	uint16_t wTemp;
	//uint32_t dwTemp;
	uint16_t Pos = 0;

	memcpy(&wTemp, InBuf + Pos, 2);
	Pos += 2;
	*MsgID = htons(wTemp);

	memcpy(&wTemp, InBuf + Pos, 2);
	Pos += 2;
	wTemp = htons(wTemp);
	if ((wTemp & 0x3ff) != InLen - JTT_PACK_HEAD_LEN)
	{
		DBG("%u %u", (uint32_t)(wTemp & 0x3ff), (uint32_t)InLen - JTT_PACK_HEAD_LEN);
		return -1;
	}
	*RxLen = (uint32_t)(wTemp & 0x3ff);

	memcpy(SimID, InBuf + Pos, 6);
	Pos += 6;

	memcpy(&wTemp, InBuf + Pos, 2);
	*MsgSn = htons(wTemp);
	Pos += 2;
	return (int32_t)Pos;
}

uint32_t JTT_RegMsgBoby(uint16_t ProvinceID, uint16_t CityID, const s8 *FactoryID, const s8 *DeviceType, const s8 *DeviceID, uint8_t Color, const s8 *CarID, uint16_t CarIDLen, uint8_t *Buf)
{
	uint32_t Pos = 0;
	ProvinceID = htons(ProvinceID);
	CityID = htons(CityID);
	memcpy(Buf + Pos, &ProvinceID, 2);
	Pos += 2;
	memcpy(Buf + Pos, &CityID, 2);
	Pos += 2;
	memcpy(Buf + Pos, FactoryID, 5);
	Pos += 5;
	memcpy(Buf + Pos, DeviceType, 20);
	Pos += 20;
	memcpy(Buf + Pos, DeviceID, 7);
	Pos += 7;
	Buf[Pos] = Color;
	Pos += 1;
	memcpy(Buf + Pos, CarID, CarIDLen);
	Pos += CarIDLen;
	return Pos;
}

uint32_t JTT_AuthMsgBoby(uint8_t *AuthCode, uint32_t AuthLen, uint8_t *Buf)
{
	memcpy(Buf, AuthCode, AuthLen);
	return AuthLen;
}

uint32_t JTT_UpgradeMsgBoby(uint8_t Type, uint8_t Result, uint8_t *Buf)
{
	Buf[0] = Type;
	Buf[1] = Result;
	return 2;
}

uint32_t JTT_LocatBaseInfoMsgBoby(Monitor_RecordStruct *Info, uint8_t *Buf)
{

	uint32_t Pos = 0;
	uint8_t StatusByte[32];
	uint8_t AlarmByte[32];
	uint32_t dwTemp, i;
	uint16_t wTemp;
	//uint8_t ucTemp;
	memset(StatusByte, 0, sizeof(StatusByte));
	memset(AlarmByte, 0, sizeof(AlarmByte));

	StatusByte[JTT_STATUS_ACC_ON] = Info->IOValUnion.IOVal.ACC;
	StatusByte[JTT_STATUS_LOCAT_STATUS] = Info->RMC.LocatStatus;
	StatusByte[JTT_STATUS_LOCAT_NS] = (Info->RMC.LatNS == 'S')?1:0;
	StatusByte[JTT_STATUS_LOCAT_EW] = (Info->RMC.LgtEW == 'W')?1:0;
	StatusByte[JTT_STATUS_ALARM_ON] = Info->DevStatus[MONITOR_STATUS_ALARM_ON];
	switch (Info->RMC.LocatMode)
	{
	case GN_GN_MODE:
		StatusByte[JTT_STATUS_GPS_MODE] = 1;
		StatusByte[JTT_STATUS_BD_MODE] = 1;
		break;
	case GN_GPS_MODE:
		StatusByte[JTT_STATUS_GPS_MODE] = 1;
		StatusByte[JTT_STATUS_BD_MODE] = 0;
		break;
	case GN_BD_MODE:
		StatusByte[JTT_STATUS_GPS_MODE] = 0;
		StatusByte[JTT_STATUS_BD_MODE] = 1;
		break;
	default:
		StatusByte[JTT_STATUS_GPS_MODE] = 0;
		StatusByte[JTT_STATUS_BD_MODE] = 0;
		break;
	}

	AlarmByte[JTT_ALARM_GNSS_ERROR] = Info->DevStatus[MONITOR_STATUS_GPS_ERROR];
	AlarmByte[JTT_ALARM_OVER_SPEED_ALARM] = Info->DevStatus[MONITOR_STATUS_OVERSPEED];
	AlarmByte[JTT_ALARM_ANT_BREAK] = Info->DevStatus[MONITOR_STATUS_ANT_BREAK];
	AlarmByte[JTT_ALARM_ANT_SHORT] = Info->DevStatus[MONITOR_STATUS_ANT_SHORT];
	AlarmByte[JTT_ALARM_LOW_POWER] = Info->DevStatus[MONITOR_STATUS_LOWPOWER];
	AlarmByte[JTT_ALARM_POWER_DOWN] = !Info->IOValUnion.IOVal.VCC;
	AlarmByte[JTT_ALARM_SENSER_ERROR] = Info->DevStatus[MONITOR_STATUS_SENSOR_ERROR];
	AlarmByte[JTT_ALARM_SIM_ERROR] = Info->DevStatus[MONITOR_STATUS_SIM_ERROR];
	AlarmByte[JTT_ALARM_GPRS_ERROR] = Info->DevStatus[MONITOR_STATUS_GPRS_ERROR];

	switch (Info->Alarm)
	{
	case ALARM_TYPE_MOVE:
		AlarmByte[JTT_ALARM_STEAL] = 1;
		break;
	case ALARM_TYPE_CRASH:
		AlarmByte[JTT_ALARM_CRASH] = 1;
		break;
	}

	dwTemp = 0;
	for (i = 0; i < 32; i++)
	{
		if (AlarmByte[i])
		{
			dwTemp |= (1 << i);
		}
	}
	dwTemp = htonl(dwTemp);
	memcpy(Buf + Pos, &dwTemp, 4);
	Pos += 4;

	dwTemp = 0;
	for (i = 0; i < 32; i++)
	{
		if (StatusByte[i])
		{
			dwTemp |= (1 << i);
		}
	}
	dwTemp = htonl(dwTemp);
	memcpy(Buf + Pos, &dwTemp, 4);
	Pos += 4;

	dwTemp = Info->RMC.LatDegree * 1000000 + Info->RMC.LatMin * 100 / 60;
	dwTemp = htonl(dwTemp);
	memcpy(Buf + Pos, &dwTemp, 4);
	Pos += 4;

	dwTemp = Info->RMC.LgtDegree * 1000000 + Info->RMC.LgtMin * 100 / 60;
	dwTemp = htonl(dwTemp);
	memcpy(Buf + Pos, &dwTemp, 4);
	Pos += 4;

	wTemp = Info->RMC.HighLevel;
//	wTemp = 0;
//	wTemp += ( (Info->CN[0] > 9)?(9):(Info->CN[0]) ) * 1000;
//	wTemp += ( (Info->CN[1] > 9)?(9):(Info->CN[1]) ) * 100;
//	wTemp += ( (Info->CN[2] > 9)?(9):(Info->CN[2]) ) * 10;
//	wTemp += ( (Info->CN[3] > 9)?(9):(Info->CN[3]) );
	wTemp = htons(wTemp);
	memcpy(Buf + Pos, &wTemp, 2);
	Pos += 2;

	dwTemp = Info->RMC.Speed * 1852 / 100000;
	wTemp = dwTemp;
	wTemp = htons(wTemp);
	memcpy(Buf + Pos, &wTemp, 2);
	Pos += 2;

	dwTemp = Info->RMC.Cog / 1000;
	wTemp = dwTemp;
	wTemp = htons(wTemp);
	memcpy(Buf + Pos, &wTemp, 2);
	Pos += 2;

	Tamp2UTC(UTC2Tamp(&Info->uDate.Date, &Info->uTime.Time) + 28800, &Info->uDate.Date, &Info->uTime.Time, 0);
//	DBG("%u-%u-%u, %u:%u:%u",Info->uDate.Date.Year, Info->uDate.Date.Mon, Info->uDate.Date.Day,
//			Info->uTime.Time.Hour, Info->uTime.Time.Min, Info->uTime.Time.Sec);

	IntToBCD(Info->uDate.Date.Year - 2000, Buf + Pos, 1);
	Pos++;
	IntToBCD(Info->uDate.Date.Mon, Buf + Pos, 1);
	Pos++;
	IntToBCD(Info->uDate.Date.Day, Buf + Pos, 1);
	Pos++;
	IntToBCD(Info->uTime.Time.Hour, Buf + Pos, 1);
	Pos++;
	IntToBCD(Info->uTime.Time.Min, Buf + Pos, 1);
	Pos++;
	IntToBCD(Info->uTime.Time.Sec, Buf + Pos, 1);
	Pos++;

//	if (gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param[PARAM_MONITOR_ADD_MILEAGE])
//	{
//		Buf[Pos] = 0x01;
//		Buf[Pos + 1] = 4;
//		Pos += 2;
//		dwTemp = Info->MileageKM * 10 + Info->MileageM / 100;
//		dwTemp = htonl(dwTemp);
//		memcpy(Buf + Pos, &dwTemp, 4);
//		Pos += 4;
//	}
//
	wTemp = 0;
	wTemp += ( (Info->CN[0] > 9)?(9):(Info->CN[0]) ) * 1000;
	wTemp += ( (Info->CN[1] > 9)?(9):(Info->CN[1]) ) * 100;
	wTemp += ( (Info->CN[2] > 9)?(9):(Info->CN[2]) ) * 10;
	wTemp += ( (Info->CN[3] > 9)?(9):(Info->CN[3]) );
	wTemp = htons(wTemp);
	Buf[Pos] = 0x02;
	Buf[Pos + 1] = 2;
	Pos += 2;
	memcpy(Buf + Pos, &wTemp, 2);
	Pos += 2;
	return Pos;
}

uint32_t JTT_AddLocatMsgBoby(uint8_t AddID, uint8_t Len, uint8_t *pData, uint8_t *pBuf)
{
	uint32_t Pos;
	pBuf[0] = AddID;
	pBuf[1] = Len;
	memcpy(pBuf + 2, pData, Len);
	Pos = Len + 2;
	return Pos;
}

uint32_t JTT_DevResMsgBoby(uint16_t MsgID, uint16_t MsgSn, uint8_t Result, uint8_t *Buf)
{
	uint16_t wTemp;
	uint32_t Pos = 0;

	wTemp = htons(MsgSn);
	memcpy(Buf + Pos, &wTemp, 2);
	Pos += 2;

	wTemp = htons(MsgID);
	memcpy(Buf + Pos, &wTemp, 2);
	Pos += 2;

	Buf[Pos] = Result;
	Pos++;

	return Pos;
}

uint32_t JTT_ParamMsgBoby(uint16_t MsgSn, uint8_t Num, uint8_t *Buf)
{
	uint16_t wTemp;
	wTemp = htons(MsgSn);
	memcpy(Buf, &wTemp, 2);
	Buf[2] = Num;
	return 3;
}

int32_t JTT_AnalyzeMonitorRes(uint16_t *MsgID, uint16_t *MsgSn, uint8_t *Result, uint8_t *Buf)
{
	uint16_t wTemp;
	int32_t Pos = 0;

	memcpy(&wTemp, Buf + Pos, 2);
	*MsgSn = htons(wTemp);
	Pos += 2;

	memcpy(&wTemp, Buf + Pos, 2);
	*MsgID = htons(wTemp);
	Pos += 2;

	*Result = Buf[Pos];
	Pos++;
	return Pos;
}

int32_t JTT_AnalyzeReg(uint16_t *MsgSn, uint8_t *Result, uint8_t *AuthCode, uint32_t *AuthLen, uint8_t *Buf, uint32_t RxLen)
{
	uint16_t wTemp;
	int32_t Pos = 0;

	memcpy(&wTemp, Buf + Pos, 2);
	*MsgSn = htons(wTemp);
	Pos += 2;

	*Result = Buf[Pos];
	Pos++;

	if (*Result)
	{
		DBG("reg fail %u", (uint32_t)(*Result));
		*AuthLen = 0;
		return -1;
	}

	memcpy(AuthCode, &Buf[Pos], RxLen - 3);
	*AuthLen = RxLen - 3;
	return Pos;
}

int32_t JTT_AnalyzeDeviceCtrl(uint8_t *Buf, uint8_t *Result)
{
	switch(Buf[0])
	{
	case JTT_DEV_CTRL_SHUTDOWN:
		*Result = 0;
		break;
	case JTT_DEV_CTRL_RESET:
		*Result = 0;
		break;
	default:
		*Result = 1;
		break;
	}
	return 0;
}

void JTT_MakeMonitorID(Monitor_CtrlStruct *Monitor)
{
	uint32_t Temp1, Temp2;
	Temp1 = gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo.UID[0];
	Temp2 = gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo.UID[1];
	Temp2 = Temp2 * 10 + Temp1 / 100000000;
	Temp1 = Temp1 % 100000000;
	IntToBCD(Temp2, Monitor->MonitorID.ucID, 2);
	IntToBCD(Temp1, Monitor->MonitorID.ucID + 2, 4);
}

