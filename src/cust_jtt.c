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
u32 JTT_PacketHead(u16 MsgID, u16 MsgSn, u8 *SimID, u16 MsgLen, u16 MsgRSA, u8 *Buf)
{
	u32 Pos = 0;
	u16 Temp;
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
u32 JTT_MuiltPacketHead(u16 MsgID, u16 MsgSn, u8 *SimID, u16 MsgLen, u16 MsgRSA, u16 PacketNum, u16 PacketSn, u8 *Buf)
{
	u32 Pos = 0;
	u16 Temp;
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

s32 JTT_AnalyzeHead(u16 *MsgID, u16 *MsgSn, u8 *SimID, u8 *InBuf, u16 InLen, u32 *RxLen)
{
	u16 wTemp;
	u32 dwTemp;
	u16 Pos = 0;

	memcpy(&wTemp, InBuf + Pos, 2);
	Pos += 2;
	*MsgID = htons(wTemp);

	memcpy(&wTemp, InBuf + Pos, 2);
	Pos += 2;
	wTemp = htons(wTemp);
	if ((wTemp & 0x3ff) != InLen - JTT_PACK_HEAD_LEN)
	{
		DBG("%d %d", (u32)(wTemp & 0x3ff), (u32)InLen - JTT_PACK_HEAD_LEN);
		return -1;
	}
	*RxLen = (u32)(wTemp & 0x3ff);

	memcpy(SimID, InBuf + Pos, 6);
	Pos += 6;

	memcpy(&wTemp, InBuf + Pos, 2);
	*MsgSn = htons(wTemp);
	Pos += 2;
	return (s32)Pos;
}

u32 JTT_RegMsgBoby(u16 ProvinceID, u16 CityID, u8 *FactoryID, u8 *DeviceType, u8 *DeviceID, u8 Color, u8 *CarID, u16 CarIDLen, u8 *Buf)
{
	u32 Pos = 0;
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

u32 JTT_AuthMsgBoby(u8 *AuthCode, u32 AuthLen, u8 *Buf)
{
	memcpy(Buf, AuthCode, AuthLen);
	return AuthLen;
}

u32 JTT_UpgradeMsgBoby(u8 Type, u8 Result, u8 *Buf)
{
	Buf[0] = Type;
	Buf[1] = Result;
	return 2;
}

u32 JTT_LocatBaseInfoMsgBoby(Monitor_RecordStruct *Info, u8 *Buf)
{

	u32 Pos = 0;
	u8 StatusByte[32];
	u8 AlarmByte[32];
	u32 dwTemp, i;
	u16 wTemp;
	u8 ucTemp;
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
//	DBG("%d-%d-%d, %d:%d:%d",Info->uDate.Date.Year, Info->uDate.Date.Mon, Info->uDate.Date.Day,
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

u32 JTT_AddLocatMsgBoby(u8 AddID, u8 Len, u8 *pData, u8 *pBuf)
{
	u32 Pos;
	pBuf[0] = AddID;
	pBuf[1] = Len;
	memcpy(pBuf + 2, pData, Len);
	Pos = Len + 2;
	return Pos;
}

u32 JTT_DevResMsgBoby(u16 MsgID, u16 MsgSn, u8 Result, u8 *Buf)
{
	u16 wTemp;
	u32 Pos = 0;

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

u32 JTT_ParamMsgBoby(u16 MsgSn, u8 Num, u8 *Buf)
{
	u16 wTemp;
	wTemp = htons(MsgSn);
	memcpy(Buf, &wTemp, 2);
	Buf[2] = Num;
	return 3;
}

s32 JTT_AnalyzeMonitorRes(u16 *MsgID, u16 *MsgSn, u8 *Result, u8 *Buf)
{
	u16 wTemp;
	s32 Pos = 0;

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

s32 JTT_AnalyzeReg(u16 *MsgSn, u8 *Result, u8 *AuthCode, u32 *AuthLen, u8 *Buf, u32 RxLen)
{
	u16 wTemp;
	s32 Pos = 0;

	memcpy(&wTemp, Buf + Pos, 2);
	*MsgSn = htons(wTemp);
	Pos += 2;

	*Result = Buf[Pos];
	Pos++;

	if (*Result)
	{
		DBG("reg fail %d", (u32)(*Result));
		*AuthLen = 0;
		return -1;
	}

	memcpy(AuthCode, &Buf[Pos], RxLen - 3);
	*AuthLen = RxLen - 3;
	return Pos;
}

s32 JTT_AnalyzeDeviceCtrl(u8 *Buf, u8 *Result)
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
	u32 Temp1, Temp2;
	Temp1 = gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo.UID[0];
	Temp2 = gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo.UID[1];
	Temp2 = Temp2 * 10 + Temp1 / 100000000;
	Temp1 = Temp1 % 100000000;
	IntToBCD(Temp2, Monitor->MonitorID.ucID, 2);
	IntToBCD(Temp1, Monitor->MonitorID.ucID + 2, 4);
}

