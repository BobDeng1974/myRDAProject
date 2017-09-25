#include "user.h"
#if (__CUST_CODE__ == __CUST_LB__ || __CUST_CODE__ == __CUST_LB_V2__)
//#define __LB_TEST__
//#define __LB_LBS__
//#define __LB_FLY_MODE_ENABLE__
#define LB_LOCK_CAR		"relay,1#"
#define LB_UNLOCK_CAR	"relay,0#"
Monitor_CtrlStruct __attribute__((section (".usr_ram"))) LBCtrl;
extern User_CtrlStruct __attribute__((section (".usr_ram"))) UserCtrl;

void LB_FlushDevInfo(void)
{
	uint8_t DevInfo = 0;
	IO_ValueUnion uIO;

	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	if (!gSys.nParam[PARAM_TYPE_ALARM2].Data.ParamDW.Param[PARAM_LOCK_CAR])
	{
		DevInfo = (1 << 7);
	}
	if (gSys.RMCInfo->LocatStatus)
	{
		DevInfo |= (1 << 6);
	}
	uIO.Val = gSys.Var[IO_VAL];
	if (uIO.IOVal.VCC)
	{
		DevInfo |= (1 << 2);
	}
	if (uIO.IOVal.ACC)
	{
		DevInfo |= (1 << 1);
	}
	if (gSys.State[ALARM_STATE])
	{
		DevInfo |= (1 << 0);
	}
	LB->DevInfo = DevInfo;

	if (gSys.Var[VBAT] <= 3400)
	{
		LB->Power = 0;
	}
	else if (gSys.Var[VBAT] <= 3700)
	{
		LB->Power = 2;
	}
	else if (gSys.Var[VBAT] <= 3900)
	{
		LB->Power = 4;
	}
	else
	{
		LB->Power = 5;
	}

	if (gSys.State[RSSI_STATE] >= 25)
	{
		LB->Signal = 4;
	}
	else if (gSys.State[RSSI_STATE] >= 15)
	{
		LB->Signal = 3;
	}
	else if (gSys.State[RSSI_STATE] >= 10)
	{
		LB->Signal = 2;
	}
	else if (gSys.State[RSSI_STATE])
	{
		LB->Signal = 1;
	}
	else
	{
		LB->Signal = 0;
	}

	LB->SatelliteNum = gSys.GSVInfoSave.Pos[0] + gSys.GSVInfoSave.Pos[1];
}

void LB_GetMCC(void)
{
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	uint32_t dwTemp;
	dwTemp = BCDToInt(gSys.IMSI, 2);
	LB->MCC = dwTemp;
	dwTemp = BCDToInt(gSys.IMSI + 2, 1);
	LB->MNC = dwTemp;
}

uint8_t LB_CheckUartHead(uint8_t Data)
{
	if (Data == LB_485_HEAD)
	{
		return 1;
	}
	return 0;
}

uint16_t LB_SendUartCmd(uint8_t Cmd, uint8_t *Data, uint8_t Len, uint8_t *Buf)
{
	uint16_t CRC16;
	if (Len > 200)
		return 0;
	Buf[LB_485_HEAD_POS] = LB_485_HEAD;
	Buf[LB_485_CMD_POS] = Cmd;
	Buf[LB_485_LEN_POS] = Len;
	memcpy(Buf + LB_485_DATA_POS, Data, Len);
	CRC16 = CRC16Cal(Buf, Len + LB_485_DATA_POS, CRC16_START, CRC16_MODBUS_GEN, 1);
	Buf[Len + LB_485_DATA_POS] = CRC16 & 0x00ff;
	Buf[Len + LB_485_DATA_POS + 1] = CRC16 >> 8;
	return Len + 5;
}

uint16_t LB_SendGPSInfo(uint8_t AlarmType, uint8_t *Buf)
{
	uint8_t GPSInfo[20];
	uint32_t dwTemp;
	uint8_t ucTemp;
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	Date_Union uDate;
	Time_Union uTime;
	memcpy(GPSInfo, gSys.IMEI, 8);
	uDate.dwDate = gSys.Var[UTC_DATE];
	uTime.dwTime = gSys.Var[UTC_TIME];
	GPSInfo[8] = uDate.Date.Year - 2000;
	GPSInfo[9] = uDate.Date.Mon;
	GPSInfo[10] = uDate.Date.Day;
	GPSInfo[11] = uTime.Time.Hour;
	GPSInfo[12] = uTime.Time.Min;
	GPSInfo[13] = uTime.Time.Sec;
	GPSInfo[14] = AlarmType;
	LB_FlushDevInfo();
	GPSInfo[15] = LB->DevInfo;
	GPSInfo[16] = LB->Power;
	GPSInfo[17] = LB->Signal;
	GPSInfo[18] = LB->SatelliteNum;
	dwTemp = gSys.RMCInfo->Speed * 1852 / 1000000;
	if (dwTemp > 255)
	{
		dwTemp = 255;
	}
	ucTemp = dwTemp;
	GPSInfo[19] = ucTemp;
	return LB_SendUartCmd(LB_485_DEV_INFO, GPSInfo, 20, Buf);
}

uint16_t LB_SendServerToECS(uint8_t *Buf)
{
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	return LB_SendUartCmd(LB_485_DIR_SEND, LB->ECSData + 1, LB->ECSDataLen - 1, Buf);
}

void LB_ComAnalyze(uint8_t *Data, uint8_t Len, uint8_t TxCmd)
{
	uint16_t CRC16Org;
	uint16_t CRC16;
	uint8_t DataLen;
	uint8_t *DataStart;
	uint8_t Cmd;
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	CRC16Org = Data[Len - 1];
	CRC16Org = (CRC16Org << 8) + Data[Len - 2];
	CRC16 = CRC16Cal(Data, Len - 2, CRC16_START, CRC16_MODBUS_GEN, 1);
	if (CRC16 != CRC16Org)
	{
		DBG("crc16 %04x %04x", CRC16, CRC16Org);
		return;
	}
	Cmd = Data[1];
	DataLen = Data[2];
	if (DataLen != (Len - 5))
	{
		DBG("len %u %u", Len - 5, DataLen);
		return;
	}
	DataStart = &Data[2];
	if (Cmd == 0x80)
	{
		DBG("Tx Cmd Error %02x", Cmd);
		return ;
	}
	Cmd &= 0x7f;
	DBG("Cmd %02x", Cmd);

	if (DataLen) //必须有长度才能上传后台
	{
		switch (Cmd)
		{
		case LB_485_DIR_SEND:
			if (DataLen)
			{
				Data[2] = LB_LB_CTRL;
				DataLen++;
				switch (TxCmd)
				{
				case LB_485_DEV_INFO:
					LB_ECSToServerTx(DataStart, DataLen);
					break;
				case LB_485_DIR_SEND:
					if (LB->ECSNeedResponse)
					{
						LB->ECSNeedResponse = 0;
						LB_ServerToECSTx(DataStart, DataLen);
					}
					else
					{
						LB_ECSToServerTx(DataStart, DataLen);
					}
					break;
				default:
					DBG("%02x", TxCmd);
					break;
				}
			}
			break;
		}
	}

}

uint32_t LB_Pack(void *Src, uint16_t Len, uint8_t Cmd, uint8_t IsLong, uint8_t *Dst)
{
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	uint16_t MsgLen = Len + 5;
	uint16_t CRC16;
	uint32_t Pos = 0;
	uint8_t *Data = (uint8_t *)Src;
	if (IsLong)
	{
		Dst[Pos++] = LB_LONG_HEAD;
		Dst[Pos++] = LB_LONG_HEAD;
		Dst[Pos++] = MsgLen >> 8;
		Dst[Pos++] = MsgLen & 0x00ff;
	}
	else
	{
		Dst[Pos++] = LB_SHORT_HEAD;
		Dst[Pos++] = LB_SHORT_HEAD;
		Dst[Pos++] = MsgLen & 0x00ff;
	}
	Dst[Pos++] = Cmd;
	if (Len)
	{
		memcpy(Dst + Pos, Data, Len);
		Pos += Len;
	}
	Dst[Pos++] = LB->MsgSn >> 8;
	Dst[Pos++] = LB->MsgSn & 0x00ff;
	LB->MsgSn++;
	CRC16 = ~CRC16Cal(Dst + 2, Pos - 2, CRC16_START, CRC16_CCITT_GEN, 1);
	Dst[Pos++] = CRC16 >> 8;
	Dst[Pos++] = CRC16 & 0x00ff;

	Dst[Pos++] = LB_TAIL1;
	Dst[Pos++] = LB_TAIL2;
	return Pos;
}

uint32_t LB_LoginTx(void)
{
	LB_LoginBody MsgBody;
#ifdef __LB_TEST__
	MsgBody.DevID[0] = 0x07;
	MsgBody.DevID[1] = 0x52;
	MsgBody.DevID[2] = 0x53;
	MsgBody.DevID[3] = 0x36;
	MsgBody.DevID[4] = 0x78;
	MsgBody.DevID[5] = 0x90;
	MsgBody.DevID[6] = 0x02;
	MsgBody.DevID[7] = 0x42;
	//memset(MsgBody.DevID + 1, 0x99, 7);
#else
	memcpy(MsgBody.DevID, gSys.IMEI, 8);
#endif
	MsgBody.DevType[0] = LB_DEV_TYPE_H;
	MsgBody.DevType[1] = LB_DEV_TYPE_L;
	MsgBody.DevZone[0] = 0x32;
	MsgBody.DevZone[1] = 0x01;
	return LB_Pack(&MsgBody, sizeof(MsgBody), LB_LOGIN_TX, 0, LBCtrl.TxBuf);
}

uint32_t LB_HeartTx(void)
{
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	LB_HeartBody MsgBody;
	LB_FlushDevInfo();
	MsgBody.DevInfo[0] = LB->DevInfo;
	MsgBody.Power[0] = LB->Power;
	MsgBody.Signal[0] = LB->Signal;
	MsgBody.Language[0] = 0;
	MsgBody.Language[1] = LB_ENGLISH;
	return LB_Pack(&MsgBody, sizeof(MsgBody), LB_HEART_TX, 0, LBCtrl.TxBuf);
}

uint32_t LB_LocatTx(Monitor_RecordStruct *Record)
{
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	LB_LocatBody MsgBody;
	uint8_t ucTemp;
	uint16_t wTemp;
	uint32_t dwTemp;
	Date_Union uDate;
	Time_Union uTime;
	uint64_t Tamp;
	uint64_t GPSTamp;
	memset(&MsgBody, 0, sizeof(MsgBody));
	MsgBody.DateTime[0] = Record->uDate.Date.Year - 2000;
	MsgBody.DateTime[1] = Record->uDate.Date.Mon;
	MsgBody.DateTime[2] = Record->uDate.Date.Day;
	MsgBody.DateTime[3] = Record->uTime.Time.Hour;
	MsgBody.DateTime[4] = Record->uTime.Time.Min;
	MsgBody.DateTime[5] = Record->uTime.Time.Sec;
	ucTemp = Record->CN[0] + Record->CN[1] + Record->CN[2] + Record->CN[3];
	if (ucTemp > 15)
	{
		ucTemp = 15;
	}
	MsgBody.GPSInfo[0] = 0xC0 + ucTemp;

	dwTemp = Record->RMC.LgtDegree * 1800000 + Record->RMC.LgtMin * 3;
	dwTemp = htonl(dwTemp);
	memcpy(MsgBody.Lgt, &dwTemp, 4);

	dwTemp = Record->RMC.LatDegree * 1800000 + Record->RMC.LatMin * 3;
	dwTemp = htonl(dwTemp);
	memcpy(MsgBody.Lat, &dwTemp, 4);


	dwTemp = Record->RMC.Speed * 1852 / 1000000;
	if (dwTemp > 255)
	{
		dwTemp = 255;
	}
	ucTemp = dwTemp;
	MsgBody.Speed[0] = ucTemp;

	wTemp = Record->RMC.Cog / 1000;
	if (wTemp >= 360)
	{
		wTemp = 0;
	}

	if (Record->RMC.LocatStatus)
	{
		wTemp |= (1 << 12);
	}
	if (Record->RMC.LatNS != 'S')
	{
		wTemp |= (1 << 10);
	}
	if (Record->RMC.LgtEW == 'W')
	{
		wTemp |= (1 << 11);
	}
	MsgBody.State[0] = wTemp >> 8;
	MsgBody.State[1] = wTemp & 0x00ff;

	MsgBody.MCC[0] = LB->MCC >> 8;
	MsgBody.MCC[1] = LB->MCC & 0x00ff;
	MsgBody.MNC[0] = LB->MNC;
	MsgBody.LAI[0] = Record->CellInfoUnion.CellInfo.ID[2];
	MsgBody.LAI[1] = Record->CellInfoUnion.CellInfo.ID[3];
	MsgBody.CI[1] = Record->CellInfoUnion.CellInfo.ID[0];
	MsgBody.CI[2] = Record->CellInfoUnion.CellInfo.ID[1];
	MsgBody.ACC[0] = Record->IOValUnion.IOVal.ACC;

	uDate.dwDate = gSys.Var[UTC_DATE];
	uTime.dwTime = gSys.Var[UTC_TIME];
	Tamp = UTC2Tamp(&uDate.Date, &uTime.Time);
	uDate.dwDate = Record->uDate.dwDate;
	uTime.dwTime = Record->uTime.dwTime;
	GPSTamp = UTC2Tamp(&uDate.Date, &uTime.Time);

	if ( Tamp > (GPSTamp + 60) )
	{
		MsgBody.Type[0] = 1;
	}
	else
	{
		MsgBody.Type[0] = 0;
	}

	dwTemp = htonl(Record->MileageKM);
	memcpy(MsgBody.Mileage, &dwTemp, 4);
	return LB_Pack(&MsgBody, sizeof(MsgBody), LB_LOCAT_TX, 0, LBCtrl.TxBuf);
}

uint32_t LB_LBSTx(void)
{
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	LB_LBSBody MsgBody;
	uint8_t i;
	Date_Union uDate;
	Time_Union uTime;
	uDate.dwDate = gSys.Var[UTC_DATE];
	uTime.dwTime = gSys.Var[UTC_TIME];

	memset(&MsgBody, 0, sizeof(MsgBody));
	MsgBody.DateTime[0] = uDate.Date.Year - 2000;
	MsgBody.DateTime[1] = uDate.Date.Mon;
	MsgBody.DateTime[2] = uDate.Date.Day;
	MsgBody.DateTime[3] = uTime.Time.Hour;
	MsgBody.DateTime[4] = uTime.Time.Min;
	MsgBody.DateTime[5] = uTime.Time.Sec;

	MsgBody.MCC[0] = LB->MCC >> 8;
	MsgBody.MCC[1] = LB->MCC & 0x00ff;
	MsgBody.MNC[0] = LB->MNC;

	MsgBody.LBSList[0].LAI[0] = gSys.CurrentCell.nTSM_LAI[3];
	MsgBody.LBSList[0].LAI[1] = gSys.CurrentCell.nTSM_LAI[4];
	MsgBody.LBSList[0].CI[1] = gSys.CurrentCell.nTSM_CellID[0];
	MsgBody.LBSList[0].CI[2] = gSys.CurrentCell.nTSM_CellID[1];
	MsgBody.LBSList[0].RSSI[0] = 255 - gSys.CurrentCell.nTSM_AvRxLevel;

	for (i = 0;i < 6; i++)
	{
		if (gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[0] || gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[1])
		{
			MsgBody.LBSList[i + 1].LAI[0] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_LAI[3];
			MsgBody.LBSList[i + 1].LAI[1] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_LAI[4];
			MsgBody.LBSList[i + 1].CI[1] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[0];
			MsgBody.LBSList[i + 1].CI[2] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[1];
			MsgBody.LBSList[i + 1].RSSI[0] = 255 - gSys.NearbyCell.nTSM_NebCell[i].nTSM_AvRxLevel;
		}

	}
	MsgBody.Diff[0] = 0xff;
	MsgBody.Language[1] = 0;
	return LB_Pack(&MsgBody, sizeof(MsgBody), LB_LBS_TX, 0, LBCtrl.TempBuf);
}

uint32_t LB_AlarmTx(Monitor_RecordStruct *Record)
{
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	LB_AlarmBody MsgBody;
	uint8_t ucTemp;
	uint16_t wTemp;
	uint32_t dwTemp;
//	Date_Union uDate;
//	Time_Union uTime;
//	uint64_t Tamp;
//	uint64_t GPSTamp;
	memset(&MsgBody, 0, sizeof(MsgBody));
	MsgBody.DateTime[0] = Record->uDate.Date.Year - 2000;
	MsgBody.DateTime[1] = Record->uDate.Date.Mon;
	MsgBody.DateTime[2] = Record->uDate.Date.Day;
	MsgBody.DateTime[3] = Record->uTime.Time.Hour;
	MsgBody.DateTime[4] = Record->uTime.Time.Min;
	MsgBody.DateTime[5] = Record->uTime.Time.Sec;
	ucTemp = Record->CN[0] + Record->CN[1] + Record->CN[2] + Record->CN[3];
	if (ucTemp > 15)
	{
		ucTemp = 15;
	}
	MsgBody.GPSInfo[0] = 0xC0 + ucTemp;

	dwTemp = Record->RMC.LatDegree * 1800000 + Record->RMC.LatMin * 3;
	dwTemp = htonl(dwTemp);
	memcpy(MsgBody.Lat, &dwTemp, 4);

	dwTemp = Record->RMC.LgtDegree * 1800000 + Record->RMC.LgtMin * 3;
	dwTemp = htonl(dwTemp);
	memcpy(MsgBody.Lgt, &dwTemp, 4);

	dwTemp = Record->RMC.Speed * 1852 / 1000000;
	if (dwTemp > 255)
	{
		dwTemp = 255;
	}
	ucTemp = dwTemp;
	MsgBody.Speed[0] = ucTemp;

	wTemp = Record->RMC.Cog / 1000;
	if (wTemp >= 360)
	{
		wTemp = 0;
	}

	if (Record->RMC.LocatStatus)
	{
		wTemp |= (1 << 12);
	}
	if (Record->RMC.LatNS != 'S')
	{
		wTemp |= (1 << 10);
	}
	if (Record->RMC.LgtEW == 'W')
	{
		wTemp |= (1 << 11);
	}
	MsgBody.State[0] = wTemp >> 8;
	MsgBody.State[1] = wTemp & 0x00ff;

	MsgBody.MCC[0] = LB->MCC >> 8;
	MsgBody.MCC[1] = LB->MCC & 0x00ff;
	MsgBody.MNC[0] = LB->MNC;
	MsgBody.LAI[0] = Record->CellInfoUnion.CellInfo.ID[2];
	MsgBody.LAI[1] = Record->CellInfoUnion.CellInfo.ID[3];
	MsgBody.CI[1] = Record->CellInfoUnion.CellInfo.ID[0];
	MsgBody.CI[2] = Record->CellInfoUnion.CellInfo.ID[1];
	MsgBody.LBSLen[0] = 9;
	LB_FlushDevInfo();
	MsgBody.DevInfo[0] = LB->DevInfo;
	MsgBody.Power[0] = LB->Power;
	MsgBody.Signal[0] = LB->Signal;
	DBG("%u %u %u", MsgBody.DevInfo[0], MsgBody.Power[0], MsgBody.Signal[0]);
	switch (Record->Alarm)
	{
	case 0:
		MsgBody.Language[0] = 0;
		break;
	case ALARM_TYPE_CRASH:
		MsgBody.Language[0] = LB_ALARM_CRASH;
		User_Req(LB_485_DEV_INFO, LB_ALARM_CRASH, 0);
		break;
	case ALARM_TYPE_MOVE:
		MsgBody.Language[0] = LB_ALARM_MOVE;
		User_Req(LB_485_DEV_INFO, LB_ALARM_MOVE, 0);
		break;
	case ALARM_TYPE_CUTLINE:
		MsgBody.Language[0] = LB_ALARM_CUTLINE;
		User_Req(LB_485_DEV_INFO, LB_ALARM_CUTLINE, 0);
		break;
	case ALARM_TYPE_OVERSPEED:
		MsgBody.Language[0] = LB_ALARM_OVERSPEED;
		User_Req(LB_485_DEV_INFO, LB_ALARM_OVERSPEED, 0);
		break;
	case ALARM_TYPE_LOWPOWER:
		MsgBody.Language[0] = LB_ALARM_LOWPOWER;
		User_Req(LB_485_DEV_INFO, LB_ALARM_LOWPOWER, 0);
		break;
	case ALARM_TYPE_NOPOWER:
		MsgBody.Language[0] = LB_ALARM_NOPOWER;
		User_Req(LB_485_DEV_INFO, LB_ALARM_NOPOWER, 0);
		break;
	case ALARM_TYPE_ACC_ON:
		MsgBody.Language[0] = LB_ALARM_ACC_ON;
		User_Req(LB_485_DEV_INFO, LB_ALARM_ACC_ON, 0);
		break;
	case ALARM_TYPE_ACC_OFF:
		MsgBody.Language[0] = LB_ALARM_ACC_OFF;
		User_Req(LB_485_DEV_INFO, LB_ALARM_ACC_OFF, 0);
		break;
	default:
		MsgBody.Language[0] = 0;
		break;
	}
	MsgBody.Language[1] = 0;
	return LB_Pack(&MsgBody, sizeof(MsgBody), LB_ALARM_TX, 0, LBCtrl.TxBuf);
}

uint32_t LB_ECSToServerTx(uint8_t *Src, uint16_t Len)
{
	uint32_t TxLen;
	TxLen = LB_Pack(Src, Len, LB_ECS_TO_SERV, 1, LBCtrl.TempBuf);
	Monitor_RecordResponse(LBCtrl.TempBuf, TxLen);
	return 0;
}

uint32_t LB_ServerToECSTx(uint8_t *Src, uint16_t Len)
{
	uint32_t TxLen;
	TxLen = LB_Pack(Src, Len, LB_SERV_TO_ECS, 1, LBCtrl.TempBuf);
	Monitor_RecordResponse(LBCtrl.TempBuf, TxLen);
	return 0;
}

int32_t LB_LoginRx(void *pData)
{
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	LB->IsAuthOK = 1;
	return 0;
}

int32_t LB_HeartRx(void *pData)
{
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	LB->NoAck = 0;
	return 0;
}

int32_t LB_CmdRx(void *pData)
{
	uint8_t Temp[260];
	uint8_t InfoLen;
	uint16_t PackLen;
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	uint8_t *CmdStart;
	uint8_t CmdLen;
	if (Buffer->Pos < 8)
	{
		return 0;
	}
	memcpy(Temp, Buffer->Data + 1, 4);
	CmdStart = Buffer->Data + 5;
	CmdLen = Buffer->Pos - 7;
	Temp[4] = 0x01;
	memcpy(Temp + 5, CmdStart, CmdLen);
	InfoLen = 5 + CmdLen;
	if (CmdLen != strlen(LB_LOCK_CAR))
	{
		goto CMD_FINISH;
	}

	if (!memcmp(CmdStart, LB_LOCK_CAR, CmdLen))
	{
		DBG("lock car!");
		gSys.nParam[PARAM_TYPE_ALARM2].Data.ParamDW.Param[PARAM_LOCK_CAR] = 1;
		Param_Save(PARAM_TYPE_ALARM2);
		GPIO_Write(USER_IO_PIN, gSys.nParam[PARAM_TYPE_ALARM2].Data.ParamDW.Param[PARAM_LOCK_CAR]);
	}
	else if (!memcmp(CmdStart, LB_UNLOCK_CAR, CmdLen))
	{
		DBG("unlock car!");
		gSys.nParam[PARAM_TYPE_ALARM2].Data.ParamDW.Param[PARAM_LOCK_CAR] = 0;
		Param_Save(PARAM_TYPE_ALARM2);
		GPIO_Write(USER_IO_PIN, gSys.nParam[PARAM_TYPE_ALARM2].Data.ParamDW.Param[PARAM_LOCK_CAR]);
	}
CMD_FINISH:
	PackLen = LB_Pack(Temp, InfoLen, LB_CMD_TX, 0, LBCtrl.TempBuf);
	Monitor_RecordResponse(LBCtrl.TempBuf, PackLen);
	return 0;
}

int32_t LB_AlarmRx(void *pData)
{
	return 0;
}

int32_t LB_TimeRx(void *pData)
{
	return 0;
}

int32_t LB_DWChineseRx(void *pData)
{
	return 0;
}

int32_t LB_DWEnglishRx(void *pData)
{
	return 0;
}

int32_t LB_ECSToServerRx(void *pData)
{
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	if (Buffer->Pos > 1)
	{
		if (Buffer->Pos > 2)
		{
			Buffer->Pos -= 2;
		}
		memcpy(LB->ECSData, Buffer->Data, Buffer->Pos);
		LB->ECSDataLen = Buffer->Pos;
		HexTrace(LB->ECSData, LB->ECSDataLen);
		LB->ECSNeedResponse = 0;
		User_Req(LB_485_DIR_SEND, 0, 0);
		OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_USER_REQ, 0, 0, 0);
	}
	return 0;
}

int32_t LB_ServerToECSRx(void *pData)
{
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	if (Buffer->Pos > 1)
	{
		if (Buffer->Pos > 2)
		{
			Buffer->Pos -= 2;
		}
		memcpy(LB->ECSData, Buffer->Data, Buffer->Pos);
		LB->ECSDataLen = Buffer->Pos;
		HexTrace(LB->ECSData, LB->ECSDataLen);
		LB->ECSNeedResponse = 1;
		User_Req(LB_485_DIR_SEND, 0, 0);
		OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_USER_REQ, 0, 0, 0);
	}
	return 0;
}
const CmdFunStruct LBCmdFun[] =
{
		{
				LB_LOGIN_RX,
				LB_LoginRx,
		},
		{
				LB_HEART_RX,
				LB_HeartRx,
		},
		{
				LB_CMD_RX,
				LB_CmdRx,
		},
		{
				LB_ALARM_RX,
				LB_AlarmRx,
		},
		{
				LB_TIME_RX,
				LB_TimeRx,
		},
		{
				LB_DW_CH_RX,
				LB_DWChineseRx,
		},
		{
				LB_DW_EN_RX,
				LB_DWEnglishRx,
		},
		{
				LB_SERV_TO_ECS,
				LB_ServerToECSRx,
		},
		{
				LB_ECS_TO_SERV,
				LB_ECSToServerRx,
		},
};

int32_t LB_ReceiveAnalyze(void *pData)
{
	uint32_t RxLen = (uint32_t)pData;
	uint32_t FinishLen = 0,i,j;
	uint16_t CRC16, CRC16Org;
	uint32_t Cmd;
	Buffer_Struct Buffer;
	//LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	DBG("Receive %u", RxLen);

	while (RxLen)
	{
		if (RxLen > MONITOR_RXBUF_LEN)
		{
			FinishLen = MONITOR_RXBUF_LEN;
		}
		else
		{
			FinishLen = RxLen;
		}

		RxLen -= OS_SocketReceive(LBCtrl.Net.SocketID, LBCtrl.RxBuf, FinishLen, NULL, NULL);
		//加入协议分析
		HexTrace(LBCtrl.RxBuf, FinishLen);
		for (i = 0; i < FinishLen; i++)
		{
			switch (LBCtrl.RxState)
			{
			case LB_PRO_FIND_HEAD1:
				if (LB_SHORT_HEAD == LBCtrl.RxBuf[i])
				{
					LBCtrl.AnalyzeBuf[0] = LB_SHORT_HEAD;
					LBCtrl.RxState = LB_PRO_FIND_HEAD2;
				}

				if (LB_LONG_HEAD == LBCtrl.RxBuf[i])
				{
					LBCtrl.AnalyzeBuf[0] = LB_LONG_HEAD;
					LBCtrl.RxState = LB_PRO_FIND_HEAD3;
				}
				break;
			case LB_PRO_FIND_HEAD2:
				if (LB_SHORT_HEAD == LBCtrl.RxBuf[i])
				{
					LBCtrl.AnalyzeBuf[1] = LB_SHORT_HEAD;
					LBCtrl.RxState = LB_PRO_FIND_LEN;
					LBCtrl.RxLen = 2;
				}
				else
				{
					LBCtrl.RxState = LB_PRO_FIND_HEAD1;
				}
				break;
			case LB_PRO_FIND_HEAD3:
				if (LB_LONG_HEAD == LBCtrl.RxBuf[i])
				{
					LBCtrl.AnalyzeBuf[1] = LB_LONG_HEAD;
					LBCtrl.RxState = LB_PRO_FIND_LEN;
					LBCtrl.RxLen = 2;
				}
				else
				{
					LBCtrl.RxState = LB_PRO_FIND_HEAD1;
				}
				break;
			case LB_PRO_FIND_LEN:
				LBCtrl.AnalyzeBuf[LBCtrl.RxLen++] = LBCtrl.RxBuf[i];
				if (LBCtrl.RxLen == (LBCtrl.AnalyzeBuf[0] - LB_SHORT_HEAD + 3))
				{
					if (LB_SHORT_HEAD == LBCtrl.AnalyzeBuf[0])
					{
						LBCtrl.RxNeedLen = LBCtrl.AnalyzeBuf[LBCtrl.RxLen - 1] + LBCtrl.RxLen;
					}
					else
					{
						LBCtrl.RxNeedLen = LBCtrl.AnalyzeBuf[LBCtrl.RxLen - 2];
						LBCtrl.RxNeedLen = LBCtrl.RxNeedLen * 256 + LBCtrl.AnalyzeBuf[LBCtrl.RxLen - 1] + LBCtrl.RxLen;
					}

					LBCtrl.RxState = LB_PRO_FIND_TAIL1;
				}
				break;
			case LB_PRO_FIND_TAIL1:
				LBCtrl.AnalyzeBuf[LBCtrl.RxLen++] = LBCtrl.RxBuf[i];
				if (LBCtrl.RxLen >= (LBCtrl.RxNeedLen + 1) )
				{
					if (LB_TAIL1 == LBCtrl.RxBuf[i])
					{
						LBCtrl.RxState = LB_PRO_FIND_TAIL2;
					}
					else
					{
						DBG("Tail error %02x", LBCtrl.RxBuf[i]);
					}
				}
				break;
			case LB_PRO_FIND_TAIL2:
				LBCtrl.AnalyzeBuf[LBCtrl.RxLen++] = LBCtrl.RxBuf[i];
				if (LBCtrl.RxLen >= (LBCtrl.RxNeedLen + 2) )
				{
					if (LB_TAIL2 == LBCtrl.RxBuf[i])
					{
						CRC16 = ~CRC16Cal(LBCtrl.AnalyzeBuf + 2, LBCtrl.RxLen - 6, CRC16_START, CRC16_CCITT_GEN, 1);
						CRC16Org = LBCtrl.AnalyzeBuf[LBCtrl.RxLen - 4];
						CRC16Org = CRC16Org * 256 + LBCtrl.AnalyzeBuf[LBCtrl.RxLen - 3];
						if (CRC16 != CRC16Org)
						{
							DBG("%04x, %04x", CRC16, CRC16Org);
						}
						else
						{
							if (LB_SHORT_HEAD == LBCtrl.AnalyzeBuf[0])
							{
								Cmd = LBCtrl.AnalyzeBuf[3];
								Buffer.Data = LBCtrl.AnalyzeBuf + 4;//信息内容起始地址
								Buffer.Pos = LBCtrl.RxNeedLen - 5;
							}
							else
							{
								Cmd = LBCtrl.AnalyzeBuf[4];
								Buffer.Data = LBCtrl.AnalyzeBuf + 5;//信息内容起始地址
								Buffer.Pos = LBCtrl.RxNeedLen - 5;
							}

							for (j = 0;j < sizeof(LBCmdFun)/sizeof(CmdFunStruct); j++)
							{
								if (Cmd == (LBCmdFun[j].Cmd & 0x000000ff))
								{
									DBG("Cmd %08x", Cmd);
									LBCmdFun[j].Func((void *)&Buffer);
									break;
								}
							}

						}
						LBCtrl.RxState = LB_PRO_FIND_HEAD1;
					}
					else
					{
						DBG("Tail error %02x", LBCtrl.RxBuf[i]);
					}
				}
				break;
			}
		}
		if (RxLen)
			DBG("rest %u", RxLen);
	}
	return 0;
}

uint8_t LB_Connect(Monitor_CtrlStruct *Monitor, Net_CtrlStruct *Net, int8_t *Url)
{
	uint8_t ProcessFinish = 0;
	IP_AddrUnion uIP;
	Led_Flush(LED_TYPE_GSM, LED_FLUSH_SLOW);

	Net->To = Monitor->Param[PARAM_MONITOR_NET_TO];
	if (Net->SocketID != INVALID_SOCKET)
	{
		DBG("Need close socket before connect!");
		Net_Disconnect(Net);
	}

	if (Url)
	{
		Net_Connect(Net, 0, Url);
	}
	else
	{
		Net_Connect(Net, Net->IPAddr.s_addr, Url);
	}

	if (Net->Result != NET_RES_CONNECT_OK)
	{
		Led_Flush(LED_TYPE_GSM, LED_FLUSH_SLOW);
		if (Net->SocketID != INVALID_SOCKET)
		{
			Net_Disconnect(Net);
		}
		ProcessFinish = 0;
	}
	else
	{
		Led_Flush(LED_TYPE_GSM, LED_ON);
		uIP.u32_addr = Net->IPAddr.s_addr;
		DBG("IP %u.%u.%u.%u OK", (uint32_t)uIP.u8_addr[0], (uint32_t)uIP.u8_addr[1],
				(uint32_t)uIP.u8_addr[2], (uint32_t)uIP.u8_addr[3]);
		ProcessFinish = 1;
	}
	return ProcessFinish;
}

uint8_t LB_Send(Monitor_CtrlStruct *Monitor, Net_CtrlStruct *Net, uint32_t Len)
{
	//LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	Led_Flush(LED_TYPE_GSM, LED_FLUSH_FAST);
	Net->To = Monitor->Param[PARAM_MONITOR_NET_TO];
	DBG("%u", Len);
	HexTrace(Monitor->TxBuf, Len);
	Net_Send(Net, Monitor->TxBuf, Len);
	if (Net->Result != NET_RES_SEND_OK)
	{
		Led_Flush(LED_TYPE_GSM, LED_FLUSH_SLOW);
		return 0;
	}
	else
	{
		if (gSys.RecordCollect.IsRunMode)
		{
			Led_Flush(LED_TYPE_GSM, LED_ON);
		}
		else
		{
			Led_Flush(LED_TYPE_GSM, LED_OFF);
		}
		return 1;
	}
}

void LB_Task(void *pData)
{
	Monitor_CtrlStruct *Monitor = &LBCtrl;
	Net_CtrlStruct *Net = &LBCtrl.Net;
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	//Param_UserStruct *User = &gSys.nParam[PARAM_TYPE_USER].Data.UserInfo;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	uint32_t SleepTime = 0;
	uint32_t KeepTime;
	uint8_t ErrorOut = 0;
	uint8_t ucTemp;
	COS_EVENT Event;
	uint8_t AuthCnt = 0;
	uint32_t TxLen = 0;
	uint8_t DataType = 0;
	IO_ValueUnion uIO;
//下面变量为每个协议独有的
	DBG("Task start! %u %u %u %u %u %u %u %u %u" ,
			Monitor->Param[PARAM_GS_WAKEUP_MONITOR], Monitor->Param[PARAM_GS_JUDGE_RUN],
			Monitor->Param[PARAM_UPLOAD_RUN_PERIOD], Monitor->Param[PARAM_UPLOAD_STOP_PERIOD],
			Monitor->Param[PARAM_UPLOAD_HEART_PERIOD], Monitor->Param[PARAM_MONITOR_NET_TO],
			Monitor->Param[PARAM_MONITOR_KEEP_TO], Monitor->Param[PARAM_MONITOR_SLEEP_TO],
			Monitor->Param[PARAM_MONITOR_RECONNECT_MAX]);

    DBG("monitor id %u", Monitor->MonitorID.dwID);
    AuthCnt = 0;
    KeepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_KEEP_TO];
    gSys.State[MONITOR_STATE] = LB_STATE_AUTH;
    while (!ErrorOut)
    {
#ifdef __LB_FLY_MODE_ENABLE__
		if (gSys.RecordCollect.WakeupFlag || (gSys.State[CRASH_STATE] > ALARM_STATE_IDLE) || (gSys.State[MOVE_STATE] > ALARM_STATE_IDLE))
		{
			KeepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_KEEP_TO];
		}

    	if (gSys.RecordCollect.IsWork && Monitor->Param[PARAM_MONITOR_KEEP_TO])
    	{
    		uIO.Val = gSys.Var[IO_VAL];
    		if (!uIO.IOVal.VCC && (gSys.Var[SYS_TIME] > KeepTime))
    		{
    			DBG("sleep!");
    			gSys.RecordCollect.WakeupFlag = 0;

    			gSys.State[MONITOR_STATE] = LB_STATE_SLEEP;
    			gSys.RecordCollect.IsWork = 0;
    			SleepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_SLEEP_TO];

    		}
    	}
#endif
    	switch (gSys.State[MONITOR_STATE])
    	{

    	case LB_STATE_AUTH:
    		Led_Flush(LED_TYPE_GSM, LED_FLUSH_SLOW);
    		gSys.RecordCollect.IsWork = 1;
    		Net->TCPPort = MainInfo->TCPPort;
    		Net->UDPPort = MainInfo->UDPPort;
    		if (AuthCnt)
    		{
    			Net->To = AuthCnt * 15;
    			Net_WaitTime(Net);
    		}
    		DBG("start auth! %d", AuthCnt);
    		if (MainInfo->MainIP)
    		{
    			Net->IPAddr.s_addr = MainInfo->MainIP;
    			ucTemp = LB_Connect(Monitor, Net, NULL);
    		}
    		else
    		{
    			ucTemp = LB_Connect(Monitor, Net, MainInfo->MainURL);
    		}

    		if (ucTemp)
    		{
    			Net->To = Monitor->Param[PARAM_MONITOR_NET_TO];
    			//发送认证
    			TxLen = LB_LoginTx();
    			if (LB_Send(Monitor, Net, TxLen))
    			{
    				Net->To = 5;
    				Net_WaitEvent(Net);
    				if (Net->Result == NET_RES_UPLOAD)
					{
						if (LB->IsAuthOK)
						{
							DBG("Auth success!");
							AuthCnt = 0;
							Monitor->ReConnCnt = 0;
							gSys.State[MONITOR_STATE] = LB_STATE_DATA;
							TxLen = LB_Pack(NULL, 0, LB_TIME_TX, 0, LBCtrl.TempBuf);
							Monitor_RecordResponse(LBCtrl.TempBuf, TxLen);
							LB_GetMCC();
							break;
						}
					}

    			}
    			else
    			{
    				DBG("start send fail!");
    			}
    		}
			AuthCnt++;
			if (AuthCnt == 2)
			{
				GPRS_Restart();
			}

			if (AuthCnt >= Monitor->Param[PARAM_MONITOR_RECONNECT_MAX])
			{
				DBG("Auth too much!");
				ErrorOut = 1;
				break;
			}
    		break;

    	case LB_STATE_DATA:
  			if (Net->Heart)
			{
				//合成心跳包
				TxLen = LB_HeartTx();
				Net->Heart = 0;
				LB->NoAck++;
				if (LB->NoAck >= 4)
				{
					LB->NoAck = 0;
					DBG("NO ACK %u, ReConnect", LB->NoAck);
					gSys.State[MONITOR_STATE] = LB_STATE_AUTH;
					continue;
				}
				LB_Send(Monitor, Net, TxLen);
				if (Net->Result != NET_RES_SEND_OK)
				{
					gSys.State[MONITOR_STATE] = LB_STATE_AUTH;
					break;
				}
			}

    		if (Monitor_GetCacheLen(CACHE_TYPE_ALL))
    		{
    			if (Monitor_GetCacheLen(CACHE_TYPE_RES))
    			{
    				DataType = CACHE_TYPE_RES;
    				TxLen = Monitor_ExtractResponse(Monitor->TxBuf);
    			}
    			else if (Monitor_GetCacheLen(CACHE_TYPE_ALARM))
    			{
    				DataType = CACHE_TYPE_ALARM;
    				Monitor_ExtractAlarm(&Monitor->Record);
    				TxLen = LB_AlarmTx(&Monitor->Record);

    			}
    			else if (Monitor_GetCacheLen(CACHE_TYPE_DATA))
    			{
    				DataType = CACHE_TYPE_DATA;
    				Monitor_ExtractData(&Monitor->Record);
#ifdef __LB_LBS__
    				if (gSys.RecordCollect.IsRunMode && (1 == Monitor_GetCacheLen(CACHE_TYPE_DATA)))
    				{
    					if (!gSys.RMCInfo->LocatStatus)
    					{
    						DBG("lbs!");
    						TxLen = LB_LBSTx();
    						Monitor_RecordResponse(LBCtrl.TempBuf, TxLen);
    					}
    				}
#endif
    				TxLen = LB_LocatTx(&Monitor->Record);
    			}

				LB_Send(Monitor, Net, TxLen);

				if (Net->Result != NET_RES_SEND_OK)
				{
					gSys.State[MONITOR_STATE] = LB_STATE_AUTH;
					break;
				}
				else
				{
					Monitor_DelCache(DataType, 0);
				}
    		}
    		else
    		{

//    			if (Monitor->Param[PARAM_MONITOR_KEEP_TO])
//    			{
//    				Net->To = Monitor->Param[PARAM_MONITOR_KEEP_TO];
//    			}
//    			else if (Monitor->Param[PARAM_UPLOAD_STOP_PERIOD] > Monitor->Param[PARAM_UPLOAD_RUN_PERIOD])
//    			{
//    				Net->To = Monitor->Param[PARAM_UPLOAD_STOP_PERIOD] * 2;
//    			}
//    			else
//    			{
//    				Net->To = Monitor->Param[PARAM_UPLOAD_RUN_PERIOD] * 2;
//    			}

    			gSys.RecordCollect.WakeupFlag = 0;
    			if (KeepTime > gSys.Var[SYS_TIME])
    			{
    				Net->To = (KeepTime - gSys.Var[SYS_TIME]) + 2;
    			}
    			else
    			{
    				Net->To = 10;
    			}

    			Net_WaitEvent(Net);
//    			if (Net->Result != NET_RES_UPLOAD)
//    			{
//    				DBG("error!");
//    				gSys.State[MONITOR_STATE] = LB_STATE_AUTH;
//    			}

    			if (Net->Result == NET_RES_ERROR)
    			{
    				DBG("error!");
    				gSys.State[MONITOR_STATE] = LB_STATE_AUTH;
    			}

    		}

    		if (gSys.RecordCollect.WakeupFlag)
    		{
    			KeepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_KEEP_TO];
    		}
    		gSys.RecordCollect.WakeupFlag = 0;


			if (Monitor->DevCtrlStatus && !Monitor_GetCacheLen(CACHE_TYPE_ALL))
			{
				SYS_Reset();
			}
    		break;

    	case LB_STATE_SLEEP:
    		Led_Flush(LED_TYPE_GSM, LED_OFF);
    		Net->To = 15;
    		if (Net->SocketID != INVALID_SOCKET)
    		{
    			Net_Disconnect(Net);
    		}
    		if (!gSys.State[FLY_REQ_STATE])
    		{
				gSys.State[FLY_REQ_STATE] = 1;
				OS_FlyMode(1);
    		}
    		Net->To = 1;
    		Net_WaitEvent(Net);
    		if (gSys.RecordCollect.WakeupFlag)
    		{
    			DBG("alarm or vacc wakeup!");
    			gSys.State[MONITOR_STATE] = LB_STATE_AUTH;
    			if (gSys.State[FLY_REQ_STATE])
    			{
    				gSys.State[FLY_QUIT_REQ_STATE] = 1;
    			}
    		}
    		else
    		{
    			uIO.Val = gSys.Var[IO_VAL];
        		if (uIO.IOVal.VCC)
        		{
        			DBG("wakeup!");
        			gSys.State[MONITOR_STATE] = LB_STATE_AUTH;
        			if (gSys.State[FLY_REQ_STATE])
        			{
        				gSys.State[FLY_QUIT_REQ_STATE] = 1;
        			}
        		}
    		}

    		if (Monitor->Param[PARAM_MONITOR_SLEEP_TO])
    		{
        		if (gSys.Var[SYS_TIME] > SleepTime)
        		{
        			DBG("time wakeup!");
        			gSys.State[MONITOR_STATE] = LB_STATE_AUTH;
        		}
    		}
    		break;

    	default:
    		gSys.State[MONITOR_STATE] = LB_STATE_AUTH;
    		break;
    	}
    }

	SYS_Reset();
	while (1)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
	}
}

void LB_Config(void)
{
	gSys.TaskID[MONITOR_TASK_ID] = COS_CreateTask(LB_Task, NULL,
				NULL, MMI_TASK_MAX_STACK_SIZE , MMI_TASK_PRIORITY + MONITOR_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI LB Task");
	LBCtrl.Param = gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param;
	LBCtrl.Net.SocketID = INVALID_SOCKET;
	LBCtrl.Net.TaskID = gSys.TaskID[MONITOR_TASK_ID];
	LBCtrl.Net.Channel = GPRS_CH_MAIN_MONITOR;
	LBCtrl.Net.TimerID = MONITOR_TIMER_ID;
	LBCtrl.RxState = LB_PRO_FIND_HEAD1;
	LBCtrl.Net.ReceiveFun = LB_ReceiveAnalyze;
	LBCtrl.CustData = (LB_CustDataStruct *)COS_MALLOC(sizeof(LB_CustDataStruct));
	memset(LBCtrl.CustData, 0, sizeof(LB_CustDataStruct));

	if (!LBCtrl.Param[PARAM_UPLOAD_RUN_PERIOD])
	{
		LBCtrl.Param[PARAM_UPLOAD_RUN_PERIOD] = 30;
	}

}
#endif
