#include "user.h"
#if (__CUST_CODE__ == __CUST_LY__ || __CUST_CODE__ == __CUST_LY_IOTDEV__)

Monitor_CtrlStruct __attribute__((section (".usr_ram"))) LYCtrl;
extern User_CtrlStruct __attribute__((section (".usr_ram"))) UserCtrl;

uint8_t LY_CheckUartHead(uint8_t Data)
{
	switch (Data)
	{
	case LY_UART_HEAD_4A:
	case LY_UART_HEAD_4B:
	case LY_UART_HEAD_5A:
		return 1;
	default:
		return 0;
	}
}

uint32_t LY_ComAnalyze(uint8_t *RxBuf, uint32_t RxLen, int32_t *Result)
{
	uint8_t Temp[8];
	uint8_t Check;
	uint32_t TxLen;
	LY_CustDataStruct *LY = (LY_CustDataStruct *)LYCtrl.CustData;
	if (RxLen < 4)
	{
		*Result = -2;
		return 0;
	}

	if (RxBuf[RxLen - 1] != LY_TAIL_FLAG)
	{
		*Result = -2;
		return 0;
	}
	switch (RxBuf[0])
	{
	case LY_UART_HEAD_4A:
		COM_TxReq(RxBuf, RxLen);
		memcpy(LY->ECUAck, LY->LastRx, 8);
		break;
	case LY_UART_HEAD_4B:
		TxLen = LY_ECUData(LYCtrl.TempBuf, RxBuf, RxLen, LY_RS232TC_VERSION);
		Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
		Check = XorCheck(RxBuf, RxLen - 2, 0);
		if (Check != RxBuf[RxLen - 2])
		{
			DBG("%02x %02x", Check, RxBuf[RxLen - 2]);
			*Result = -3;
			return 0;
		}
		else
		{
			Temp[0] = LY_UART_HEAD_5B;
			Temp[1] = RxBuf[1];
			Temp[2] = 0xff;
			Temp[3] = 2;
			Temp[4] = RxBuf[3];
			Temp[5] = RxBuf[RxLen - 2];
			Temp[6] = XorCheck(Temp, 6, 0);
			Temp[7] = LY_TAIL_FLAG;
			COM_TxReq(Temp, 8);
		}
		break;
	case LY_UART_HEAD_5A:
		COM_TxReq(RxBuf, RxLen);
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 0, 1, LY_RS232TC_VERSION, RxBuf, RxLen);
		Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
		break;
	}
	*Result = 0;
	return 0;
}

uint16_t LY_PackData(uint8_t *Dest, uint8_t *Src, uint16_t Len, uint8_t Version, uint8_t Cmd)
{
	uint32_t dwTemp;
	uint16_t wTemp;
	uint64_t Tamp;
	Date_Union uDate;
	Time_Union uTime;
	Dest[LY_PACK_HEAD] = LY_HEAD_FLAG;
	Dest[LY_PACK_HEAD + 1] = LY_HEAD_FLAG;
	Dest[LY_PACK_VER] = Version;
	Dest[LY_PACK_CMD] = Cmd;
	wTemp = htons(Len + 12);
	memcpy(Dest + LY_PACK_LEN, &wTemp, 2);
	dwTemp = htonl(LYCtrl.MonitorID.dwID);
	memcpy(Dest + LY_PACK_PID, &dwTemp, 4);
	uDate.dwDate = gSys.Var[UTC_DATE];
	uTime.dwTime = gSys.Var[UTC_TIME];
	Tamp = UTC2Tamp(&uDate.Date, &uTime.Time) - 28800;//绿源要求UTC-8小时
	LongToBCD(Tamp, Dest + LY_PACK_TAMP, 6);
	if (Len)
	{
		memcpy(Dest + LY_PACK_DATA, Src, Len);
	}
	Dest[LY_PACK_DATA + Len] = XorCheck(Dest, Len + LY_PACK_DATA, 0);
	Dest[LY_PACK_DATA + Len + 1] = LY_TAIL_FLAG;
	return LY_PACK_DATA + Len + 2;
}

uint16_t LY_AuthData(uint8_t *Dest)
{
	uint16_t Len = 0;
	uint32_t B[4], C[4], M[2];
	IP_AddrUnion uIP;
	memcpy(Dest + Len, gSys.IMEI, 8);
	Len += 8;
	memcpy(Dest + Len, gSys.IMSI, 8);
	Len += 8;
	memcpy(Dest + Len, gSys.ICCID, 10);
	Len += 10;
	uIP.u32_addr = htonl(LYCtrl.MonitorID.dwID);
	B[0] = uIP.u8_addr[0];
	B[1] = uIP.u8_addr[1];
	B[2] = uIP.u8_addr[2];
	B[3] = uIP.u8_addr[3];
	C[0] = (B[0] * 20) % 256;
	C[1] = (B[1] * 21) % 256;
	C[2] = (B[2] * 22) % 256;
	C[3] = (B[3] * 23) % 256;
	M[0] = (C[0] + C[2]) % 256;
	M[1] = (C[1] + C[3]) % 256;
	Dest[Len] = M[0];
	Dest[Len + 1] = M[1];
	Dest[Len + 2] = 0x21;
	Len += 3;
	return Len;
}

uint16_t LY_LogInData(uint8_t *Dest)
{
	uint16_t Len = 0;
	IP_AddrUnion uIP;
	Param_UserStruct *User = &gSys.nParam[PARAM_TYPE_USER].Data.UserInfo;
	uIP.u32_addr = User->LY.LYIP;
	memcpy(Dest + Len, uIP.u8_addr, 4);
	Len += 4;
	Dest[Len] = User->LY.LYTCPPort >> 8;
	Dest[Len + 1] = User->LY.LYTCPPort & 0x00ff;
	Len += 2;
	Dest[Len] = User->LY.LYUDPPort >> 8;
	Dest[Len + 1] = User->LY.LYUDPPort & 0x00ff;
	Len += 2;
	return Len;
}

uint16_t LY_ResponseData(uint8_t *Dest, uint8_t Result, uint8_t IsECU, uint8_t Vesion, uint8_t *Data, uint16_t Len)
{
	uint8_t *TempBuf;
	uint16_t TxLen;
	LY_CustDataStruct *LY = (LY_CustDataStruct *)LYCtrl.CustData;
	TempBuf = COS_MALLOC(9 + Len);
	if (!TempBuf)
	{
		DBG("no mem!");
		return 0;
	}
	if (Len)
	{
		memcpy(TempBuf + 6, Data, Len);
	}
	if (IsECU)
	{
		memcpy(TempBuf, LY->ECUAck, 6);
		memcpy(TempBuf + 6 + Len, LY->ECUAck + 6, 2);
	}
	else
	{
		memcpy(TempBuf,  LY->LastRx, 6);
		memcpy(TempBuf + 6 + Len, LY->LastRx + 6, 2);
	}
	TempBuf[8 + Len] = Result;
	TxLen = LY_PackData(Dest, TempBuf, 9 + Len, Vesion, LY_TX_DEVICE_RES_CMD);
	COS_FREE(TempBuf);
	return TxLen;
}

uint16_t LY_LocatData(uint8_t *Dest, Monitor_RecordStruct *Record)
{
	uint32_t Pos = 0;
	uint32_t dwTemp;
	uint16_t wTemp;
	uint8_t ucTemp;
	uint8_t CN = Record->CN[0] + Record->CN[1] + Record->CN[2] + Record->CN[3];
	uint64_t Tamp;
	uint64_t GPSTamp;
	Date_Union uDate;
	Time_Union uTime;

	Dest[LY_PACK_HEAD] = LY_HEAD_FLAG;
	Dest[LY_PACK_HEAD + 1] = LY_HEAD_FLAG;
	Dest[LY_PACK_VER] = LY_MONITOR_VERSION;
	Dest[LY_PACK_CMD] = LY_TX_LOCAT_CMD;

	memcpy(Dest + LY_PACK_DATA + Pos, Record->CellInfoUnion.CellInfo.ID, 4);
	Pos += 4;
	IntToBCD(Record->DevStatus[MONITOR_STATUS_SIGNAL], &Dest[LY_PACK_DATA + Pos], 1);
	Pos++;
	IntToBCD(CN, &Dest[LY_PACK_DATA + Pos], 1);
	Pos++;

	if ('S' == Record->RMC.LatNS)
	{
		dwTemp = (Record->RMC.LatDegree + 90) * 1000000 + Record->RMC.LatMin * 100 / 60;
	}
	else
	{
		dwTemp = (Record->RMC.LatDegree + 0) * 1000000 + Record->RMC.LatMin * 100 / 60;
	}
	IntToBCD(dwTemp / 10, &Dest[LY_PACK_DATA + Pos], 4);
	Pos += 4;

	if ('W' == Record->RMC.LgtEW)
	{
		dwTemp = (Record->RMC.LgtDegree + 180) * 1000000 + Record->RMC.LgtMin * 100 / 60;
	}
	else
	{
		dwTemp = (Record->RMC.LgtDegree + 0) * 1000000 + Record->RMC.LgtMin * 100 / 60;
	}
	IntToBCD(dwTemp / 10, &Dest[LY_PACK_DATA + Pos], 4);
	Pos += 4;

	dwTemp = Record->RMC.Speed * 1852 / 1000000;
	IntToBCD(dwTemp, &Dest[LY_PACK_DATA + Pos], 2);
	Pos += 2;

	dwTemp = Record->RMC.Cog / 1000;
	IntToBCD(dwTemp, &Dest[LY_PACK_DATA + Pos], 2);
	Pos += 2;

	ucTemp = (1 << 3);
	if (Record->IOValUnion.IOVal.VCC)
	{
		ucTemp = (1 << 0);
	}

	if (CN > 3)
	{
		ucTemp |= (1 << 1);
	}

	if (Record->RMC.LocatStatus)
	{
		ucTemp |= (1 << 2);
	}
	Dest[LY_PACK_DATA + Pos] = ucTemp;
	Pos++;

	dwTemp = Record->MileageKM * 10 + Record->MileageM / 100;
	IntToBCD(dwTemp, &Dest[LY_PACK_DATA + Pos], 3);
	Pos += 3;

	memset(&Dest[LY_PACK_DATA + Pos], 0, 2);//外部电压，这里设置为0
	Pos += 2;

	IntToBCD(Record->Vbat, &Dest[LY_PACK_DATA + Pos], 2);
	Pos += 2;

	Dest[LY_PACK_DATA + Pos] = Record->DevStatus[MONITOR_STATUS_ALARM_ON];
	Pos++;

	Dest[LY_PACK_DATA + Pos] = 1; //MOS管
	Pos++;
	switch (Record->Alarm)
	{
	case 0:
		Dest[LY_PACK_DATA + Pos] = LY_NO_ALARM;
		break;
	case ALARM_TYPE_ACC_ON:
		Dest[LY_PACK_DATA + Pos] = LY_ACC_ON_ALARM;
		break;
	case ALARM_TYPE_ACC_OFF:
		Dest[LY_PACK_DATA + Pos] = LY_ACC_OFF_ALARM;;
		break;
	case ALARM_TYPE_CRASH:
		Dest[LY_PACK_DATA + Pos] = LY_CRASH_ALARM;
		break;
	case ALARM_TYPE_MOVE:
		Dest[LY_PACK_DATA + Pos] = LY_MOVE_ALARM;
		break;
	case ALARM_TYPE_CUTLINE:
		Dest[LY_PACK_DATA + Pos] = LY_CUT_ALARM;
		break;
	case ALARM_TYPE_LOWPOWER:
		Dest[LY_PACK_DATA + Pos] = LY_LOWPOWER_ALARM;
		break;
	default:
		Dest[LY_PACK_DATA + Pos] = LY_NO_ALARM;
		break;
	}
	if (Record->DevStatus[MONITOR_STATUS_SIM_ERROR])
	{
		Dest[LY_PACK_DATA + Pos] = LY_SIM_ALARM;
	}
	else if (Record->DevStatus[MONITOR_STATUS_GPS_ERROR])
	{
		Dest[LY_PACK_DATA + Pos] = LY_GNSS_ALARM;
	}
	else if (Record->DevStatus[MONITOR_STATUS_SENSOR_ERROR])
	{
		Dest[LY_PACK_DATA + Pos] = LY_SENSOR_ALARM;
	}
	else if (Record->DevStatus[MONITOR_STATUS_LOWPOWER])
	{
		Dest[LY_PACK_DATA + Pos] = LY_LOWPOWER_ALARM;
	}

	Pos++;

	IntToBCD(Record->CrashCNT, &Dest[LY_PACK_DATA + Pos], 2);
	Pos += 2;
	IntToBCD(Record->MoveCNT, &Dest[LY_PACK_DATA + Pos], 2);
	Pos += 2;

	uDate.dwDate = gSys.Var[UTC_DATE];
	uTime.dwTime = gSys.Var[UTC_TIME];
	Tamp = UTC2Tamp(&uDate.Date, &uTime.Time);
	uDate.dwDate = Record->uDate.dwDate;
	uTime.dwTime = Record->uTime.dwTime;
	GPSTamp = UTC2Tamp(&uDate.Date, &uTime.Time);
	//DBG("%x %x %u", uDate.dwDate, uTime.dwTime, (uint32_t)GPSTamp);
	if ( Tamp > (GPSTamp + 60) )
	{
		Dest[LY_PACK_DATA + Pos] = 0;
	}
	else
	{
		Dest[LY_PACK_DATA + Pos] = 1;
	}
	Pos++;
	//DBG("%u", Pos);
	wTemp = htons(Pos + 12);
	memcpy(Dest + LY_PACK_LEN, &wTemp, 2);
	dwTemp = htonl(LYCtrl.MonitorID.dwID);
	memcpy(Dest + LY_PACK_PID, &dwTemp, 4);
	LongToBCD(GPSTamp - 28800, Dest + LY_PACK_TAMP, 6);//绿源要求UTC-8小时
	Dest[LY_PACK_DATA + Pos] = XorCheck(Dest, Pos + LY_PACK_DATA, 0);
	Dest[LY_PACK_DATA + Pos + 1] = LY_TAIL_FLAG;
	//HexTrace(Dest, LY_PACK_DATA + Pos + 2);
	return LY_PACK_DATA + Pos + 2;
}

uint16_t LY_ECUData(uint8_t *Dest, uint8_t *Data, uint16_t Len, uint8_t Version)
{
	return LY_PackData(Dest, Data, Len, Version, LY_TX_ECU_CMD);
}

int32_t LY_AuthResponse(void *pData)
{
	LY_CustDataStruct *LY = (LY_CustDataStruct *)LYCtrl.CustData;
	uint32_t Pos = 0;
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	uint8_t *Data = Buffer->Data;
	IP_AddrUnion uIP;
	uint16_t TCPPort, UDPPort;
	Date_Union uDate;
	Time_Union uTime;
	Param_UserStruct *User = &gSys.nParam[PARAM_TYPE_USER].Data.UserInfo;
	memcpy(uIP.u8_addr, Data + Pos, 4);
	Pos += 4;
	memcpy(&TCPPort, Data + Pos, 2);
	Pos += 2;
	memcpy(&UDPPort, Data + Pos, 2);
	Pos += 2;
	TCPPort = htons(TCPPort);
	UDPPort = htons(UDPPort);
	uDate.Date.Year = Data[Pos++];
	uDate.Date.Year = uDate.Date.Year * 256 + Data[Pos++];
	uDate.Date.Mon = Data[Pos++];
	uDate.Date.Day = Data[Pos++];
	uTime.Time.Hour = Data[Pos++];
	uTime.Time.Min = Data[Pos++];
	uTime.Time.Sec = Data[Pos++];

	SYS_CheckTime(&uDate.Date, &uTime.Time);
	if ( (User->LY.LYIP != uIP.u32_addr) || (User->LY.LYTCPPort != TCPPort) || (User->LY.LYUDPPort != UDPPort) )
	{
		User->LY.LYIP = uIP.u32_addr;
		User->LY.LYTCPPort = TCPPort;
		User->LY.LYUDPPort = UDPPort;
		DBG("%u.%u.%u.%u, %u %u", (uint32_t)uIP.u8_addr[0], (uint32_t)uIP.u8_addr[1], (uint32_t)uIP.u8_addr[2],
					(uint32_t)uIP.u8_addr[3], (uint32_t)TCPPort, (uint32_t)UDPPort);

		Param_Save(PARAM_TYPE_USER);
	}
	LY->IsAuthOK = 1;
	return 0;
}

int32_t LY_MonitorResponse(void *pData)
{
	LY_CustDataStruct *LY = (LY_CustDataStruct *)LYCtrl.CustData;
	LY->NoAck = 0;
	return 0;
}

int32_t LY_SetPID(void *pData)
{
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	uint8_t *Data = Buffer->Data;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	uint32_t PID;
	uint32_t TxLen;
	int32_t Result;
	memcpy(&PID, Data, 4);
	PID = htonl(PID);
	Result = 0;
	if (MainInfo->UID[0] != PID)
	{
		DBG("New PID %u", PID);
		MainInfo->UID[0] = PID;
		Result = Param_Save(PARAM_TYPE_MAIN);
	}
	if (Result >= 0)
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	else
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 0, 0, LY_MONITOR_VERSION, NULL, 0);
	}

	Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
	return 0;
}

int32_t LY_SetAPN(void *pData)
{
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	uint8_t *Data = Buffer->Data;
	uint8_t Len;
	uint32_t Pos = 0;
	uint32_t TxLen;
	Param_APNStruct APN;
	int32_t Result = 0;
	Param_APNStruct *pAPN = &gSys.nParam[PARAM_TYPE_APN].Data.APN;
	memset(&APN, 0, sizeof(Param_APNStruct));

	Len = Data[Pos];
	Pos++;
	if (Len > sizeof(APN.APNName))
	{
		Result = -1;
		goto LY_SET_APN_END;
	}
	if (Len)
	{
		memcpy(APN.APNName, &Data[Pos], Len);
	}
	else
	{
		memcpy(APN.APNName, pAPN->APNName, sizeof(APN.APNName));
	}
	Pos += Len;

	Len = Data[Pos];
	Pos++;
	if (Len > sizeof(APN.APNUser))
	{
		Result = -1;
		goto LY_SET_APN_END;
	}
	if (Len)
	{
		memcpy(APN.APNUser, &Data[Pos], Len);
	}
	else
	{
		memcpy(APN.APNUser, pAPN->APNUser, sizeof(APN.APNUser));
	}
	Pos += Len;

	Len = Data[Pos];
	Pos++;
	if (Len > sizeof(APN.APNPassword))
	{
		Result = -1;
		goto LY_SET_APN_END;
	}
	if (Len)
	{
		memcpy(APN.APNPassword, &Data[Pos], Len);
	}
	else
	{
		memcpy(APN.APNPassword, pAPN->APNPassword, sizeof(APN.APNPassword));
	}
	Pos += Len;

	if (memcmp(pAPN, &APN, sizeof(Param_APNStruct)))
	{
		DBG("New APN %s,%s,%s", APN.APNName, APN.APNUser, APN.APNPassword);
		memcpy(pAPN, &APN, sizeof(Param_APNStruct));
		Result = Param_Save(PARAM_TYPE_APN);
	}
LY_SET_APN_END:
	if (Result >= 0)
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	else
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 0, 0, LY_MONITOR_VERSION, NULL, 0);
	}

	Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
	return 0;
}

int32_t LY_SetAuth(void *pData)
{
	LY_CustDataStruct *LY = (LY_CustDataStruct *)LYCtrl.CustData;
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	uint8_t *Data = Buffer->Data;
	uint8_t Len;
	uint16_t wTemp;
	uint32_t Pos = 0;
	uint32_t TxLen;
	Param_MainStruct Main;
	int32_t Result = 0;
	Param_MainStruct *pMain = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	memset(&Main, 0, sizeof(Param_MainStruct));
	Len = Data[Pos];
	Pos++;
	if (Len >= sizeof(Main.MainURL))
	{
		Result = -1;
		goto LY_SET_AUTH_END;
	}
	if (Len)
	{
		memcpy(Main.MainURL, &Data[Pos], Len);
	}
	else
	{
		memcpy(Main.MainURL, pMain->MainURL, sizeof(Main.MainURL));
	}
	Pos += Len;
	memcpy(&wTemp, &Data[Pos], 2);
	Main.TCPPort = htons(wTemp);

	if (memcmp(pMain->MainURL, Main.MainURL, sizeof(Main.MainURL)) || (Main.TCPPort != pMain->TCPPort))
	{
		DBG("New Auth %s %u", Main.MainURL, Main.TCPPort);
		memcpy(pMain->MainURL, Main.MainURL, sizeof(Main.MainURL));
		pMain->TCPPort = Main.TCPPort;
		Result = Param_Save(PARAM_TYPE_MAIN);
	}
LY_SET_AUTH_END:
	if (Result >= 0)
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	else
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 0, 0, LY_MONITOR_VERSION, NULL, 0);
		LY->NeedReAuth = 1;
	}
	Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
	return 0;

}

int32_t LY_SetHeartInterval(void *pData)
{
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	uint8_t *Data = Buffer->Data;
	uint16_t wTemp;
	int32_t Result = 0;
	uint32_t TxLen;
	memcpy(&wTemp, Data, 2);
	wTemp = htons(wTemp);
	if (wTemp != LYCtrl.Param[PARAM_UPLOAD_HEART_PERIOD])
	{
		DBG("New heart period %u", wTemp);
		LYCtrl.Param[PARAM_UPLOAD_HEART_PERIOD] = wTemp;
		Result = Param_Save(PARAM_TYPE_MONITOR);
	}

	if (Result >= 0)
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	else
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 0, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
	return 0;
}

int32_t LY_SetNormalInterval(void *pData)
{
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	uint8_t *Data = Buffer->Data;
	uint16_t wTemp;
	int32_t Result = 0;
	uint32_t TxLen;
	memcpy(&wTemp, Data, 2);
	wTemp = htons(wTemp);
	if (wTemp != LYCtrl.Param[PARAM_UPLOAD_RUN_PERIOD])
	{
		DBG("New run period %u", wTemp);
		LYCtrl.Param[PARAM_UPLOAD_RUN_PERIOD] = wTemp;
		Result = Param_Save(PARAM_TYPE_MONITOR);
	}

	if (Result >= 0)
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	else
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 0, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
	return 0;
}

int32_t LY_SetSleepInterval(void *pData)
{
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	uint8_t *Data = Buffer->Data;
	uint16_t wTemp;
	int32_t Result = 0;
	uint32_t TxLen;
	memcpy(&wTemp, Data, 2);
	wTemp = htons(wTemp);
	if (wTemp != LYCtrl.Param[PARAM_UPLOAD_STOP_PERIOD])
	{
		DBG("New stop period %u", wTemp);
		LYCtrl.Param[PARAM_UPLOAD_STOP_PERIOD] = wTemp;
		Result = Param_Save(PARAM_TYPE_MONITOR);
	}

	if (Result >= 0)
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	else
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 0, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
	return 0;
}

int32_t LY_SetCrashLevel(void *pData)
{
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	uint8_t *Data = Buffer->Data;
	uint32_t TxLen;
	int32_t Result = 0;
	uint32_t dwTemp[2];
	uint32_t *Param = gSys.nParam[PARAM_TYPE_ALARM1].Data.ParamDW.Param;
	if ( (Data[0] < 1) || (Data[0] > 10) )
	{
		Result = -1;
		goto LY_SET_CRASH_LEVEL_END;
	}

	if ( (Data[1] < 1) || (Data[1] > 16) )
	{
		Result = -1;
		goto LY_SET_CRASH_LEVEL_END;
	}
	dwTemp[0] = Data[0];
	dwTemp[1] = Data[1] * 5;
	if ( (Param[PARAM_CRASH_JUDGE_CNT] != dwTemp[0]) || (Param[PARAM_CRASH_GS] != dwTemp[1]) )
	{
		DBG("New crash param %u %u", dwTemp[0], dwTemp[1]);
		Param[PARAM_CRASH_JUDGE_CNT] = dwTemp[0];
		Param[PARAM_CRASH_GS] = dwTemp[1];
		Result = Param_Save(PARAM_TYPE_ALARM1);
	}

LY_SET_CRASH_LEVEL_END:
	if (Result >= 0)
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	else
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 0, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
	return 0;
}

int32_t LY_SetMileage(void *pData)
{
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	uint8_t *Data = Buffer->Data;
	uint32_t TxLen;
	int32_t Result = 0;
	uint32_t Mileage, MileageKM, MileageM;
	Param_LocatStruct *LocatInfo = &gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo;
	Mileage = Data[0];
	Mileage = Mileage * 256 + Data[1];
	Mileage = Mileage * 256 + Data[2];
	MileageKM = Mileage / 10;
	MileageM = (Mileage % 10) * 100;
	if ( (LocatInfo->MileageKM != MileageKM) || (LocatInfo->MileageM != MileageM) )
	{
		DBG("New mileage %u %u", MileageKM, MileageM);
		LocatInfo->MileageKM = MileageKM;
		LocatInfo->MileageM = MileageM;
		Result = Param_Save(PARAM_TYPE_LOCAT);
	}
	if (Result >= 0)
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	else
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 0, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
	return 0;
}

int32_t LY_SetOwner(void *pData)
{
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	uint8_t *Data = Buffer->Data;
	uint8_t i;
	uint8_t Num[6];
	uint32_t TxLen;
	int32_t Result = 0;
	Param_NumberStruct *Number = &gSys.nParam[PARAM_TYPE_NUMBER].Data.Number;
	for (i = 0; i < 6; i++)
	{
		IntToBCD(Data[i], Data + i, 1);
	}

	Num[0] = (Data[0] << 4) | (Data[1] >> 4);
	Num[1] = (Data[1] << 4) | (Data[2] >> 4);
	Num[2] = (Data[2] << 4) | (Data[3] >> 4);
	Num[3] = (Data[3] << 4) | (Data[4] >> 4);
	Num[4] = (Data[4] << 4) | (Data[5] >> 4);
	Num[5] = (Data[5] << 4) | 0x0f;

	if (memcmp(&Number->Phone[0].Num[1], Num, 6))
	{
		DBG("New alarm phone");
		memset(Number->Phone[0].Num, 0, 8);
		Number->Phone[0].Num[0] = 11;
		memcpy(&Number->Phone[0].Num[1], Num, 6);
		HexTrace(Number->Phone[0].Num, 8);
		Result = Param_Save(PARAM_TYPE_NUMBER);
	}

	if (Result >= 0)
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	else
	{
		TxLen = LY_ResponseData(LYCtrl.TempBuf, 0, 0, LY_MONITOR_VERSION, NULL, 0);
	}
	Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
	return 0;
}

int32_t LY_Restart(void *pData)
{
	uint32_t TxLen;
	LYCtrl.DevCtrlStatus = 1;
	TxLen = LY_ResponseData(LYCtrl.TempBuf, 0, 0, LY_MONITOR_VERSION, NULL, 0);
	Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
	return 0;
}

int32_t LY_ToECU(void *pData)
{
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	uint8_t *Data = Buffer->Data;
	LY_CustDataStruct *LY = (LY_CustDataStruct *)LYCtrl.CustData;
	if (LY->ToECUBuf.MaxLen < Buffer->Pos)
	{
		DBG("relength! %u %u", LY->ToECUBuf.MaxLen, Buffer->Pos);
		if (LY->ToECUBuf.Data)
		{
			COS_FREE(LY->ToECUBuf.Data);
		}
		LY->ToECUBuf.Data = COS_MALLOC(Buffer->Pos);
		if (!LY->ToECUBuf.Data)
		{
			DBG("no mem");
			return -1;
		}
		LY->ToECUBuf.MaxLen = Buffer->Pos;
	}
	memcpy(LY->ToECUBuf.Data, Data, Buffer->Pos);
	LY->ToECUBuf.Pos = Buffer->Pos;
	memcpy(LY->ECUAck, LY->LastRx, 8);
	User_Req(LY_USER_TO_ECU, 10, 0);//发往串口，串口应答超时时间10秒
	OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_USER_REQ, 0, 0, 0);
	return 0;
}

int32_t LY_UploadLocation(void *pData)
{
	Monitor_RecordStruct Data;
	Monitor_RecordStruct *Record = &Data;
	uint8_t Dest[128];
	uint32_t Pos = 0;
	uint32_t dwTemp, TxLen;
	uint8_t ucTemp;
	uint8_t CN;

	Monitor_Record(Record);
	CN = Record->CN[0] + Record->CN[1] + Record->CN[2] + Record->CN[3];

	memcpy(Dest + Pos, Record->CellInfoUnion.CellInfo.ID, 4);
	Pos += 4;
	IntToBCD(Record->DevStatus[MONITOR_STATUS_SIGNAL], &Dest[LY_PACK_DATA + Pos], 1);
	Pos++;
	IntToBCD(CN, &Dest[Pos], 1);
	Pos++;

	if ('S' == Record->RMC.LatNS)
	{
		dwTemp = (Record->RMC.LatDegree + 90) * 1000000 + Record->RMC.LatMin * 100 / 60;
	}
	else
	{
		dwTemp = (Record->RMC.LatDegree + 0) * 1000000 + Record->RMC.LatMin * 100 / 60;
	}
	IntToBCD(dwTemp / 10, &Dest[Pos], 4);
	Pos += 4;

	if ('W' == Record->RMC.LgtEW)
	{
		dwTemp = (Record->RMC.LgtDegree + 180) * 1000000 + Record->RMC.LgtMin * 100 / 60;
	}
	else
	{
		dwTemp = (Record->RMC.LgtDegree + 0) * 1000000 + Record->RMC.LgtMin * 100 / 60;
	}
	IntToBCD(dwTemp / 10, &Dest[Pos], 4);
	Pos += 4;

	dwTemp = Record->RMC.Speed * 1852 / 1000000;
	IntToBCD(dwTemp, &Dest[Pos], 2);
	Pos += 2;

	dwTemp = Record->RMC.Cog / 1000;
	IntToBCD(dwTemp, &Dest[Pos], 2);
	Pos += 2;

	ucTemp = (1 << 3);
	if (Record->IOValUnion.IOVal.VCC)
	{
		ucTemp = (1 << 0);
	}

	if (CN > 3)
	{
		ucTemp |= (1 << 1);
	}

	if (Record->RMC.LocatStatus)
	{
		ucTemp |= (1 << 2);
	}
	Dest[Pos] = ucTemp;
	Pos++;

	dwTemp = Record->MileageKM * 10 + Record->MileageM / 100;
	IntToBCD(dwTemp, &Dest[Pos], 3);
	Pos += 3;

	memset(&Dest[Pos], 0, 2);//外部电压，这里设置为0
	Pos += 2;

	IntToBCD(Record->Vbat, &Dest[Pos], 2);
	Pos += 2;

	Dest[Pos] = Record->DevStatus[MONITOR_STATUS_ALARM_ON];
	Pos++;

	Dest[Pos] = 1; //MOS管
	Pos++;
	switch (Record->Alarm)
	{
	case 0:
		Dest[Pos] = LY_NO_ALARM;
		break;
	case ALARM_TYPE_ACC_ON:
		Dest[Pos] = LY_ACC_ON_ALARM;
		break;
	case ALARM_TYPE_ACC_OFF:
		Dest[Pos] = LY_ACC_OFF_ALARM;;
		break;
	case ALARM_TYPE_CRASH:
		Dest[Pos] = LY_CRASH_ALARM;
		break;
	case ALARM_TYPE_MOVE:
		Dest[Pos] = LY_MOVE_ALARM;
		break;
	case ALARM_TYPE_CUTLINE:
		Dest[Pos] = LY_CUT_ALARM;
		break;
	case ALARM_TYPE_LOWPOWER:
		Dest[Pos] = LY_LOWPOWER_ALARM;
		break;
	default:
		Dest[Pos] = LY_NO_ALARM;
		break;
	}
	if (Record->DevStatus[MONITOR_STATUS_SIM_ERROR])
	{
		Dest[Pos] = LY_SIM_ALARM;
	}
	else if (Record->DevStatus[MONITOR_STATUS_GPS_ERROR])
	{
		Dest[Pos] = LY_GNSS_ALARM;
	}
	else if (Record->DevStatus[MONITOR_STATUS_SENSOR_ERROR])
	{
		Dest[Pos] = LY_SENSOR_ALARM;
	}
	else if (Record->DevStatus[MONITOR_STATUS_LOWPOWER])
	{
		Dest[Pos] = LY_LOWPOWER_ALARM;
	}

	Pos++;

	IntToBCD(Record->CrashCNT, &Dest[Pos], 2);
	Pos += 2;
	IntToBCD(Record->MoveCNT, &Dest[Pos], 2);
	Pos += 2;

	Dest[LY_PACK_DATA + Pos] = 1;
	Pos++;

	TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 0, LY_MONITOR_VERSION, Dest, Pos);
	Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
	return 0;
}

const CmdFunStruct LYCmdFun[] =
{
		{
				LY_RX_AUTH_RES_CMD,
				LY_AuthResponse,
		},
		{
				LY_RX_START_RES_CMD,
				NULL,
		},
		{
				LY_RX_MONITOR_RES_CMD,
				LY_MonitorResponse,
		},
		{
				LY_RX_SET_PID_CMD,
				LY_SetPID,
		},
		{
				LY_RX_SET_APN_CMD,
				LY_SetAPN,
		},
		{
				LY_RX_SET_AUTH_CMD,
				LY_SetAuth,
		},
		{
				LY_RX_SET_HEART_INTERVAL_CMD,
				LY_SetHeartInterval,
		},
		{
				LY_RX_SET_NORMAL_INTERVAL_CMD,
				LY_SetNormalInterval,
		},
		{
				LY_RX_SET_SLEEP_INTERVAL_CMD,
				LY_SetSleepInterval,
		},
		{
				LY_RX_SET_CRASH_LEVEL_CMD,
				LY_SetCrashLevel,
		},
		{
				LY_RX_SET_MILEAGE_CMD,
				LY_SetMileage,
		},
		{
				LY_RX_SET_OWNER_CMD,
				LY_SetOwner,
		},
		{
				LY_RX_SET_RESTART_CMD,
				LY_Restart,
		},
		{
				LY_RX_TO_ECU_CMD,
				LY_ToECU,
		},
		{
				LY_RX_UPLOAD_LOCATION,
				LY_UploadLocation,
		}
};

int32_t LY_ReceiveAnalyze(void *pData)
{
	uint32_t RxLen = (uint32_t)pData;
	uint32_t FinishLen = 0,i,j;
	uint8_t Check,Cmd;
	Buffer_Struct Buffer;
	LY_CustDataStruct *LY = (LY_CustDataStruct *)LYCtrl.CustData;
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

		RxLen -= OS_SocketReceive(LYCtrl.Net.SocketID, LYCtrl.RxBuf, FinishLen, NULL, NULL);
		//加入协议分析
		HexTrace(LYCtrl.RxBuf, FinishLen);
		for (i = 0; i < FinishLen; i++)
		{
			switch (LYCtrl.RxState)
			{
			case LY_PRO_FIND_HEAD1:
				if (LY_HEAD_FLAG == LYCtrl.RxBuf[i])
				{
					LYCtrl.AnalyzeBuf[0] = LY_HEAD_FLAG;
					LYCtrl.RxState = LY_PRO_FIND_HEAD2;
				}
				break;
			case LY_PRO_FIND_HEAD2:
				if (LY_HEAD_FLAG == LYCtrl.RxBuf[i])
				{
					LYCtrl.AnalyzeBuf[1] = LY_HEAD_FLAG;
					LYCtrl.RxState = LY_PRO_FIND_LEN;
					LYCtrl.RxLen = 2;
				}
				else
				{
					LYCtrl.RxState = LY_PRO_FIND_HEAD1;
				}
				break;
			case LY_PRO_FIND_LEN:
				LYCtrl.AnalyzeBuf[LYCtrl.RxLen++] = LYCtrl.RxBuf[i];
				if (LYCtrl.RxLen >= 6)
				{
					LYCtrl.RxNeedLen = LYCtrl.AnalyzeBuf[LY_PACK_LEN];
					LYCtrl.RxNeedLen = LYCtrl.RxNeedLen * 256 + LYCtrl.AnalyzeBuf[LY_PACK_LEN + 1];
					LYCtrl.RxNeedLen += LY_PACK_PID;
					LYCtrl.RxState = LY_PRO_FIND_TAIL;
				}
				break;
			case LY_PRO_FIND_TAIL:
				LYCtrl.AnalyzeBuf[LYCtrl.RxLen++] = LYCtrl.RxBuf[i];
				if (LYCtrl.RxLen >= LYCtrl.RxNeedLen)
				{
					if (LY_TAIL_FLAG == LYCtrl.AnalyzeBuf[LYCtrl.RxNeedLen - 1])
					{
						Check = XorCheck(LYCtrl.AnalyzeBuf, LYCtrl.RxNeedLen - 2, 0);
						if (Check != LYCtrl.AnalyzeBuf[LYCtrl.RxNeedLen - 2])
						{
							DBG("%02x, %02x", Check, LYCtrl.AnalyzeBuf[LYCtrl.RxNeedLen - 2]);
						}
						else
						{
							memcpy(LY->LastRx, &LYCtrl.AnalyzeBuf[LY_PACK_TAMP], 6);
							LY->LastRx[6] = Check;
							LY->LastRx[7] = LYCtrl.AnalyzeBuf[LY_PACK_CMD];
							for (j = 0;j < sizeof(LYCmdFun)/sizeof(CmdFunStruct); j++)
							{
								Cmd = LYCmdFun[j].Cmd & 0x000000ff;
								if (Cmd == LYCtrl.AnalyzeBuf[LY_PACK_CMD])
								{
									DBG("Cmd %02x", Cmd);
									Buffer.Data = LYCtrl.AnalyzeBuf + LY_PACK_DATA;
									Buffer.Pos = LYCtrl.RxNeedLen - LY_PACK_DATA;
									LYCmdFun[j].Func((void *)&Buffer);
									break;
								}
							}

						}
						LYCtrl.RxState = LY_PRO_FIND_HEAD1;
					}
					else
					{
						DBG("Tail error %02x", LYCtrl.AnalyzeBuf[LYCtrl.RxNeedLen - 1]);
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

uint8_t LY_Connect(Monitor_CtrlStruct *Monitor, Net_CtrlStruct *Net, int8_t *Url)
{
	uint8_t ProcessFinish = 0;
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
		ProcessFinish = 1;
	}
	return ProcessFinish;
}

uint8_t LY_Send(Monitor_CtrlStruct *Monitor, Net_CtrlStruct *Net, uint32_t Len)
{
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
		Led_Flush(LED_TYPE_GSM, LED_ON);
		return 1;
	}
}

void LY_Task(void *pData)
{
	Monitor_CtrlStruct *Monitor = &LYCtrl;
	Net_CtrlStruct *Net = &LYCtrl.Net;
	LY_CustDataStruct *LY = (LY_CustDataStruct *)LYCtrl.CustData;
	Param_UserStruct *User = &gSys.nParam[PARAM_TYPE_USER].Data.UserInfo;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	//uint32_t SleepTime;
	uint32_t KeepTime;
	uint8_t ErrorOut = 0;

	COS_EVENT Event;
	uint8_t AuthCnt = 0;
	uint32_t TxLen = 0;
	uint8_t DataType = 0;
//下面变量为每个协议独有的
	DBG("Task start! %u %u %u %u %u %u %u %u %u" ,
			Monitor->Param[PARAM_GS_WAKEUP_MONITOR], Monitor->Param[PARAM_GS_JUDGE_RUN],
			Monitor->Param[PARAM_UPLOAD_RUN_PERIOD], Monitor->Param[PARAM_UPLOAD_STOP_PERIOD],
			Monitor->Param[PARAM_UPLOAD_HEART_PERIOD], Monitor->Param[PARAM_MONITOR_NET_TO],
			Monitor->Param[PARAM_MONITOR_KEEP_TO], Monitor->Param[PARAM_MONITOR_SLEEP_TO],
			Monitor->Param[PARAM_MONITOR_RECONNECT_MAX]);

	Monitor->MonitorID.dwID = MainInfo->UID[0];
    DBG("monitor id %u", Monitor->MonitorID.dwID);
    AuthCnt = 0;
    Monitor->IsWork = 1;
    KeepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_KEEP_TO];
    LY->ToECUBuf.Data = COS_MALLOC(32);
    LY->ToECUBuf.MaxLen = 32;
    gSys.State[MONITOR_STATE] = LY_STATE_AUTH;
    while (!ErrorOut)
    {
    	//绿源平台不允许休眠
#if 0
    	if (Monitor->IsWork && Monitor->Param[PARAM_MONITOR_KEEP_TO])
    	{
    		if (gSys.Var[SYS_TIME] > KeepTime)
    		{
    			DBG("sleep!");
    			gSys.Monitor->WakeupFlag = 0;
    			if (Net->SocketID != INVALID_SOCKET)
    			{
    				DBG("Need close socket before sleep!");
    				Net_Disconnect(Net);
    			}
    			State = LY_STATE_SLEEP;
    			Monitor->IsWork = 0;
    			SleepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_SLEEP_TO];
    		}
    	}
#endif
    	switch (gSys.State[MONITOR_STATE])
    	{

    	case LY_STATE_AUTH:
    		Monitor->IsWork = 1;
    		LY->IsAuthOK = 0;
    		Net->TCPPort = MainInfo->TCPPort;
    		Net->UDPPort = MainInfo->UDPPort;
    		Net->To = AuthCnt * 15;
    		Net_WaitTime(Net);
    		DBG("start auth!");
    		if (LY_Connect(Monitor, Net, MainInfo->MainURL))
    		{
    			Net->To = Monitor->Param[PARAM_MONITOR_NET_TO];
    			//发送认证
    			TxLen = LY_AuthData(Monitor->TempBuf);
    			TxLen = LY_PackData(Monitor->TxBuf, Monitor->TempBuf, TxLen, LY_MONITOR_VERSION, LY_TX_AUTH_CMD);
    			if (LY_Send(Monitor, Net, TxLen))
    			{
    				Net_WaitEvent(Net);
    				if (Net->Result == NET_RES_UPLOAD)
					{
						if (LY->IsAuthOK)
						{
							DBG("Auth success!");
							AuthCnt = 0;
							Monitor->ReConnCnt = 0;
							gSys.State[MONITOR_STATE] = LY_STATE_LOGIN;
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
				if ((User->LY.LYIP != 0) && (User->LY.LYTCPPort != 0))
				{
					DBG("use last data point!");
					AuthCnt = 0;
					Monitor->ReConnCnt = 0;
					gSys.State[MONITOR_STATE] = LY_STATE_LOGIN;

				}
				else
				{
					DBG("no last data point!");
					ErrorOut = 1;
				}

				break;
			}
    		break;
    	case LY_STATE_LOGIN:
    		LY->NoAck = 0;
    		Net->IPAddr.s_addr = User->LY.LYIP;
    		Net->TCPPort = User->LY.LYTCPPort;
    		Net->UDPPort = 0;
    		if (Monitor->ReConnCnt)
    		{
    			Net->To = 3 + Monitor->ReConnCnt * 30;
    			Net_WaitTime(Net);
    		}
    		if (LY_Connect(Monitor, Net, NULL))
    		{
        		TxLen = LY_LogInData(Monitor->TempBuf);
        		TxLen = LY_PackData(Monitor->TxBuf, Monitor->TempBuf, TxLen, LY_MONITOR_VERSION, LY_TX_START_CMD);
        		//开始登陆过程
        		if (LY_Send(Monitor, Net, TxLen))
        		{
        			gSys.State[MONITOR_STATE] = LY_STATE_DATA;
					break;
        		}
    		}
    		Monitor->ReConnCnt++;
			if (Monitor->ReConnCnt >= Monitor->Param[PARAM_MONITOR_RECONNECT_MAX])
			{
				DBG("Data IP connect fail!");
				ErrorOut = 1;
				break;
			}
			break;

    	case LY_STATE_DATA:
    		if (Net->Heart)
			{
				//合成心跳包
				Net->Heart = 0;
				LY->NoAck++;
				if (LY->NoAck >= 4)
				{
					DBG("NO ACK %u, ReConnect", LY->NoAck);
					gSys.State[MONITOR_STATE] = LY_STATE_LOGIN;
					continue;
				}
				TxLen = LY_PackData(Monitor->TxBuf, NULL, 0, LY_MONITOR_VERSION, LY_TX_HEART_CMD);
				LY_Send(Monitor, Net, TxLen);
				if (Net->Result != NET_RES_SEND_OK)
				{
					gSys.State[MONITOR_STATE] = LY_STATE_LOGIN;
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
    				TxLen = LY_LocatData(Monitor->TxBuf, &Monitor->Record);

    			}
    			else if (Monitor_GetCacheLen(CACHE_TYPE_DATA))
    			{
    				DataType = CACHE_TYPE_DATA;
    				Monitor_ExtractData(&Monitor->Record);
    				TxLen = LY_LocatData(Monitor->TxBuf, &Monitor->Record);
    			}

				LY_Send(Monitor, Net, TxLen);

				if (Net->Result != NET_RES_SEND_OK)
				{
					gSys.State[MONITOR_STATE] = LY_STATE_LOGIN;
					break;
				}
				else
				{
					Monitor_DelCache(DataType, 0);
				}
    		}
    		else
    		{

    			if (Monitor->Param[PARAM_MONITOR_KEEP_TO])
    			{
    				Net->To = Monitor->Param[PARAM_MONITOR_KEEP_TO];
    			}
    			else if (Monitor->Param[PARAM_UPLOAD_STOP_PERIOD] > Monitor->Param[PARAM_UPLOAD_RUN_PERIOD])
    			{
    				Net->To = Monitor->Param[PARAM_UPLOAD_STOP_PERIOD] * 2;
    			}
    			else
    			{
    				Net->To = Monitor->Param[PARAM_UPLOAD_RUN_PERIOD] * 2;
    			}
    			Net_WaitEvent(Net);
    			if (Net->Result != NET_RES_UPLOAD)
    			{
    				DBG("error!");
    				gSys.State[MONITOR_STATE] = LY_STATE_LOGIN;
    			}
    		}

    		if (gSys.Monitor->WakeupFlag)
    		{
    			KeepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_KEEP_TO];
    		}
    		gSys.Monitor->WakeupFlag = 0;

    		if (LY->NeedReAuth && !Monitor_GetCacheLen(CACHE_TYPE_ALL))
    		{
    			DBG("ReAuth!");
    			LY->NeedReAuth = 0;
    			gSys.State[MONITOR_STATE] = LY_STATE_AUTH;
    		}

			if (Monitor->DevCtrlStatus && !Monitor_GetCacheLen(CACHE_TYPE_ALL))
			{
				SYS_Reset();
			}
    		break;

//    	case LY_STATE_SLEEP:
//    		Net_WaitEvent(Net);
//    		if (Monitor->WakeupFlag)
//    		{
//    			DBG("alarm or vacc wakeup!");
//    			gSys.State[MONITOR_STATE] = LY_STATE_AUTH;
//    		}
//    		if (Monitor->Param[PARAM_MONITOR_SLEEP_TO])
//    		{
//        		if (gSys.Var[SYS_TIME] > SleepTime)
//        		{
//        			DBG("time wakeup!");
//        			gSys.State[MONITOR_STATE] = LY_STATE_AUTH;
//        		}
//    		}
//    		break;

    	default:
    		gSys.State[MONITOR_STATE] = LY_STATE_AUTH;
    		break;
    	}
    }

	SYS_Reset();
	while (1)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
	}
}

void LY_Config(void)
{
	gSys.TaskID[MONITOR_TASK_ID] = COS_CreateTask(LY_Task, NULL,
				NULL, MMI_TASK_MAX_STACK_SIZE , MMI_TASK_PRIORITY + MONITOR_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI LY Task");
	LYCtrl.Param = gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param;
	LYCtrl.Net.SocketID = INVALID_SOCKET;
	LYCtrl.Net.TaskID = gSys.TaskID[MONITOR_TASK_ID];
	LYCtrl.Net.Channel = GPRS_CH_MAIN_MONITOR;
	LYCtrl.Net.TimerID = MONITOR_TIMER_ID;
	LYCtrl.RxState = LY_PRO_FIND_HEAD1;
	LYCtrl.Net.ReceiveFun = LY_ReceiveAnalyze;
	LYCtrl.CustData = (LY_CustDataStruct *)COS_MALLOC(sizeof(LY_CustDataStruct));
	memset(LYCtrl.CustData, 0, sizeof(LY_CustDataStruct));
	gSys.Monitor = &LYCtrl;
	if (!LYCtrl.Param[PARAM_UPLOAD_RUN_PERIOD])
	{
		LYCtrl.Param[PARAM_UPLOAD_RUN_PERIOD] = 30;
	}
}
#endif
