#include "user.h"
const StrFunStruct LVFun[];
extern Upgrade_FileStruct __attribute__((section (".file_ram"))) FileCache;
extern GPIO_ParamStruct __attribute__((section (".usr_ram"))) PinParam[PIN_MAX];
int32_t LV_SetPID(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	uint32_t PIDLen;
	uint32_t UID[2];
	int32_t Error = 0;
	uint8_t i;

	if (!LV->DataIn)
	{
		LV->Result = sprintf(LV->DataOut, "%s=%02u%09u", LVFun[LV->Sn].Cmd, (int)MainInfo->UID[1], (int)MainInfo->UID[0]);
		LV->Result = 0;
		return 0;
	}
	PIDLen = strlen(LV->DataIn);
	for (i = 0; i < PIDLen; i++)
	{
		if (!IsDigit(LV->DataIn[i]))
		{
			LV->Result = 0;
			return -1;
		}
	}
	if ( PIDLen == 11 )
	{
		UID[0] = strtol(LV->DataIn + 2, NULL, 10);
		UID[1] = AsciiToU32(LV->DataIn, 2);
	}
	else if ( PIDLen == 9 )
	{
		UID[0] = strtol(LV->DataIn, NULL, 10);
		UID[1] = 0;
	}
	else
	{
		LV->Result =0;
		return -1;
	}
	DBG("%u %u", UID[0], UID[1]);
	if ( (MainInfo->UID[0] != UID[0]) || (MainInfo->UID[1] != UID[1]) )
	{
		MainInfo->UID[0] = UID[0];
		MainInfo->UID[1] = UID[1];
		Error = Param_Save(PARAM_TYPE_MAIN);
	}
	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		LV->Result = sprintf(LV->DataOut, "%s=%02u%09u", LVFun[LV->Sn].Cmd, (int)MainInfo->UID[1], (int)MainInfo->UID[0]);
	}
	return Error;
}

int32_t LV_SetMainUrl(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	s8 Temp[2][64];
	CmdParam CP;
	Param_MainStruct MainInfo;
	int32_t Error = 0;
	memcpy(&MainInfo, &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo, sizeof(Param_MainStruct));
	if (!LV->DataIn)
	{

		LV->Result = sprintf(LV->DataOut, "%s=%s %u", LVFun[LV->Sn].Cmd,
				MainInfo.MainURL, MainInfo.TCPPort);
		LV->Result = 0;
		return 0;
	}
	CP.param_max_len = 64;
	CP.param_max_num = 2;
	CP.param_num = 0;
	CP.param_str = (s8 *)Temp;
	memset(Temp, 0, sizeof(Temp));
	CmdParseParam(LV->DataIn, &CP, ',');
	if (IsDigitStr(&Temp[1][0], strlen(&Temp[1][0])) && Temp[0][0])
	{
		strcpy(MainInfo.MainURL, &Temp[0][0]);
		MainInfo.TCPPort = strtol(&Temp[1][0], NULL, 10);
	}
	if (strcmp(MainInfo.MainURL, gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo.MainURL)
			|| (MainInfo.TCPPort != gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo.TCPPort))
	{
		strcpy(gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo.MainURL, MainInfo.MainURL);
		gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo.TCPPort = MainInfo.TCPPort;
		Error = Param_Save(PARAM_TYPE_MAIN);
	}
	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		LV->Result = sprintf(LV->DataOut, "%s=%s %u", LVFun[LV->Sn].Cmd,
				MainInfo.MainURL, MainInfo.TCPPort);
	}
	return 0;
}

int32_t LV_SetAPN(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	s8 Temp[3][20];
	CmdParam CP;
	Param_APNStruct APN;
	int32_t Error = 0;
	Param_APNStruct *pAPN = &gSys.nParam[PARAM_TYPE_APN].Data.APN;
	memset(&APN, 0, sizeof(Param_APNStruct));
	if (!LV->DataIn)
	{
		LV->Result = sprintf(LV->DataOut, "%s=%s,%s,%s", LVFun[LV->Sn].Cmd,
				pAPN->APNName, pAPN->APNUser, pAPN->APNPassword);
		LV->Result = 0;
		return 0;
	}
	CP.param_max_len = 20;
	CP.param_max_num = 3;
	CP.param_num = 0;
	CP.param_str = (s8 *)Temp;
	memset(Temp, 0, sizeof(Temp));
	CmdParseParam(LV->DataIn, &CP, ',');
	strcpy(APN.APNName, &Temp[0][0]);
	strcpy(APN.APNUser, &Temp[1][0]);
	strcpy(APN.APNPassword, &Temp[2][0]);

	if (memcmp(pAPN, &APN, sizeof(Param_APNStruct)))
	{
		DBG("New APN %s,%s,%s", APN.APNName, APN.APNUser, APN.APNPassword);
		memcpy(pAPN, &APN, sizeof(Param_APNStruct));
		Error = Param_Save(PARAM_TYPE_APN);
	}

	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		LV->Result = sprintf(LV->DataOut, "%s=%s,%s,%s", LVFun[LV->Sn].Cmd,
				pAPN->APNName, pAPN->APNUser, pAPN->APNPassword);
	}
	return 0;
}

int32_t LV_SetMileage(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	uint32_t Mileage, MileageKM, MileageM;
	Param_LocatStruct *LocatInfo = &gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo;
	int32_t Error = 0;

	if (!LV->DataIn)
	{
		LV->Result = sprintf(LV->DataOut, "%s=%u.%u", LVFun[LV->Sn].Cmd,
				(int)LocatInfo->MileageKM, (int)LocatInfo->MileageM / 100);
		LV->Result = 0;
		return 0;
	}
	Mileage = strtoul(LV->DataIn, 0, 10);

	MileageKM = Mileage / 10;
	MileageM = (Mileage % 10) * 100;
	if ( (LocatInfo->MileageKM != MileageKM) || (LocatInfo->MileageM != MileageM) )
	{
		DBG("New mileage %u %u", MileageKM, MileageM);
		LocatInfo->MileageKM = MileageKM;
		LocatInfo->MileageM = MileageM;
		Error = Param_Save(PARAM_TYPE_LOCAT);
	}

	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		LV->Result = sprintf(LV->DataOut, "%s=%u.%u", LVFun[LV->Sn].Cmd,
				(int)LocatInfo->MileageKM, (int)LocatInfo->MileageM / 100);
	}
	return 0;
}

int32_t LV_SetHeart(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	uint32_t dwTemp;
	uint32_t *Param = gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param;
	int32_t Error = 0;
	if (!LV->DataIn)
	{
		LV->Result = sprintf(LV->DataOut, "%s=%u", LVFun[LV->Sn].Cmd, (int)Param[PARAM_UPLOAD_HEART_PERIOD]);
		LV->Result = 0;
		return 0;
	}
	dwTemp = strtoul(LV->DataIn, 0, 10);
	if ( (dwTemp) && (dwTemp != Param[PARAM_UPLOAD_HEART_PERIOD]) )
	{
		DBG("New heart period %u", dwTemp);
		Param[PARAM_UPLOAD_HEART_PERIOD] = dwTemp;
		Error = Param_Save(PARAM_TYPE_MONITOR);
	}

	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		LV->Result = sprintf(LV->DataOut, "%s=%u", LVFun[LV->Sn].Cmd, (int)Param[PARAM_UPLOAD_HEART_PERIOD]);
	}
	return 0;
}

int32_t LV_SetNormal(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	uint32_t dwTemp;
	uint32_t *Param = gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param;
	int32_t Error = 0;
	if (!LV->DataIn)
	{
		LV->Result = sprintf(LV->DataOut, "%s=%u", LVFun[LV->Sn].Cmd, (int)Param[PARAM_UPLOAD_RUN_PERIOD]);
		LV->Result = 0;
		return 0;
	}
	dwTemp = strtoul(LV->DataIn, 0, 10);
	if ( (dwTemp) && (dwTemp != Param[PARAM_UPLOAD_RUN_PERIOD]) )
	{
		DBG("New run period %u", dwTemp);
		Param[PARAM_UPLOAD_RUN_PERIOD] = dwTemp;
		Error = Param_Save(PARAM_TYPE_MONITOR);
	}

	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		LV->Result = sprintf(LV->DataOut, "%s=%u", LVFun[LV->Sn].Cmd, (int)Param[PARAM_UPLOAD_RUN_PERIOD]);
	}
	return 0;
}

int32_t LV_SetStop(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	uint32_t dwTemp;
	uint32_t *Param = gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param;
	int32_t Error = 0;
	if (!LV->DataIn)
	{
		LV->Result = sprintf(LV->DataOut, "%s=%u", LVFun[LV->Sn].Cmd, (int)Param[PARAM_UPLOAD_STOP_PERIOD]);
		LV->Result = 0;
		return 0;
	}
	dwTemp = strtoul(LV->DataIn, 0, 10);
	if ( (dwTemp) && (dwTemp != Param[PARAM_UPLOAD_STOP_PERIOD]) )
	{
		DBG("New stop period %u", dwTemp);
		Param[PARAM_UPLOAD_STOP_PERIOD] = dwTemp;
		Error = Param_Save(PARAM_TYPE_MONITOR);
	}

	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		LV->Result = sprintf(LV->DataOut, "%s=%u", LVFun[LV->Sn].Cmd, (int)Param[PARAM_UPLOAD_STOP_PERIOD]);
	}
	return 0;
}

int32_t LV_SetCrash(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	uint32_t *Param = gSys.nParam[PARAM_TYPE_ALARM1].Data.ParamDW.Param;
	int32_t Error = 0;
	CmdParam CP;
	s8 Temp[2][20];
	uint32_t dwTemp[2];
	if (!LV->DataIn)
	{
		LV->Result = sprintf(LV->DataOut, "%s=%u,%u", LVFun[LV->Sn].Cmd, (int)Param[PARAM_CRASH_JUDGE_CNT],
				(int)Param[PARAM_CRASH_GS]);
		LV->Result = 0;
		return 0;
	}
	CP.param_max_len = 20;
	CP.param_max_num = 2;
	CP.param_num = 0;
	CP.param_str = (s8 *)Temp;
	memset(Temp, 0, sizeof(Temp));
	CmdParseParam(LV->DataIn, &CP, ',');

	dwTemp[0] = strtoul(&Temp[0][0], 0, 10);
	dwTemp[1] = strtoul(&Temp[1][0], 0, 10) * 5;

	if (!dwTemp[0])
	{
		dwTemp[0] = Param[PARAM_CRASH_JUDGE_CNT];
	}

	if (!dwTemp[1])
	{
		dwTemp[1] = Param[PARAM_CRASH_GS];
	}

	if ( (Param[PARAM_CRASH_JUDGE_CNT] != dwTemp[0]) || (Param[PARAM_CRASH_GS] != dwTemp[1]) )
	{
		DBG("New crash param %u %u", dwTemp[0], dwTemp[1]);
		Param[PARAM_CRASH_JUDGE_CNT] = dwTemp[0];
		Param[PARAM_CRASH_GS] = dwTemp[1];
		Error = Param_Save(PARAM_TYPE_ALARM1);
	}

	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		LV->Result = sprintf(LV->DataOut, "%s=%u,%u", LVFun[LV->Sn].Cmd, (int)Param[PARAM_CRASH_JUDGE_CNT],
				(int)Param[PARAM_CRASH_GS]);
	}
	return 0;
}

int32_t LV_SetPhone(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	uint8_t i;
	uint8_t PhoneDec[12];
	uint8_t PhoneBCD[6];
	Param_NumberStruct *Number = &gSys.nParam[PARAM_TYPE_NUMBER].Data.Number;
	int32_t Error = 0;
	if (!LV->DataIn)
	{
		memcpy(PhoneBCD, &Number->Phone[0].Num[1], 6);
		PhoneBCD[5] = PhoneBCD[5] >> 4;
		sprintf(PhoneDec, "%02u%02u%02u%02u%02u%01d", PhoneBCD[0], PhoneBCD[1], PhoneBCD[2], PhoneBCD[3], PhoneBCD[4],PhoneBCD[5]);
		LV->Result = sprintf(LV->DataOut, "%s=%s", LVFun[LV->Sn].Cmd, PhoneDec);
		LV->Result = 0;
		return 0;
	}
	memset(PhoneBCD, 0, 6);
	for (i = 0; i < 11; i++)
	{
		if (LV->DataIn[i])
		{
			PhoneDec[i] = LV->DataIn[i] - '0';
			if (PhoneDec[i] > 9)
			{
				Error = 1;
				goto LV_SET_PHONE_END;
			}
		}
		else
		{
			Error = 1;
			goto LV_SET_PHONE_END;
		}
	}
	PhoneBCD[0] = (PhoneDec[0] << 4) | (PhoneDec[1]);
	PhoneBCD[1] = (PhoneDec[2] << 4) | (PhoneDec[3]);
	PhoneBCD[2] = (PhoneDec[4] << 4) | (PhoneDec[5]);
	PhoneBCD[3] = (PhoneDec[6] << 4) | (PhoneDec[7]);
	PhoneBCD[4] = (PhoneDec[8] << 4) | (PhoneDec[9]);
	PhoneBCD[5] = (PhoneDec[10] << 4) | 0x0f;

	if (memcmp(&Number->Phone[0].Num[1], PhoneBCD, 6))
	{
		DBG("New alarm phone");
		memset(Number->Phone[0].Num, 0, 8);
		Number->Phone[0].Num[0] = 11;
		memcpy(&Number->Phone[0].Num[1], PhoneBCD, 6);
		HexTrace(Number->Phone[0].Num, 8);
		Error = Param_Save(PARAM_TYPE_NUMBER);
	}
LV_SET_PHONE_END:
	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		memcpy(PhoneBCD, &Number->Phone[0].Num[1], 6);
		PhoneBCD[5] = PhoneBCD[5] >> 4;
		sprintf(PhoneDec, "%02u%02u%02u%02u%02u%01d", PhoneBCD[0], PhoneBCD[1], PhoneBCD[2], PhoneBCD[3], PhoneBCD[4],PhoneBCD[5]);
		LV->Result = sprintf(LV->DataOut, "%s=%s", LVFun[LV->Sn].Cmd, PhoneDec);
	}
	return 0;
}

int32_t LV_Reboot(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	LV->DataOut[0] = 0;
	LV->Result = 0;
	SYS_Reset();
	return 0;
}

int32_t LV_FindPosition(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	Param_LocatStruct *LocatInfo = &gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo;
	sprintf(LV->DataOut, "%s=%c%u.%04d,%c%u.%04d", LVFun[LV->Sn].Cmd,
			LocatInfo->RMCSave.LatNS, (int)LocatInfo->RMCSave.LatDegree, (int)LocatInfo->RMCSave.LatMin/60,
			LocatInfo->RMCSave.LgtEW, (int)LocatInfo->RMCSave.LgtDegree, (int)LocatInfo->RMCSave.LgtMin/60);
	LV->Result = 0;
	return 0;
}

int32_t LV_SetMoveRange(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	uint32_t dwTemp;
	uint32_t *Param = gSys.nParam[PARAM_TYPE_ALARM1].Data.ParamDW.Param;
	int32_t Error = 0;
	if (!LV->DataIn)
	{
		LV->Result = sprintf(LV->DataOut, "%s=%u", LVFun[LV->Sn].Cmd, (int)Param[PARAM_MOVE_RANGE]);
		return 0;
	}
	dwTemp = strtoul(LV->DataIn, 0, 10);
	if ( (dwTemp) && (dwTemp != Param[PARAM_MOVE_RANGE]) )
	{
		DBG("New move range %u", dwTemp);
		Param[PARAM_MOVE_RANGE] = dwTemp;
		Error = Param_Save(PARAM_TYPE_ALARM1);
	}

	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		LV->Result = sprintf(LV->DataOut, "%s=%u", LVFun[LV->Sn].Cmd, (int)Param[PARAM_MOVE_RANGE]);
	}
	return 0;
}

int32_t LV_Sat(void *Data)
{
	IP_AddrUnion uIP;
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	Param_APNStruct *pAPN = &gSys.nParam[PARAM_TYPE_APN].Data.APN;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	Param_UserStruct *User = &gSys.nParam[PARAM_TYPE_USER].Data.UserInfo;
	uint32_t *Param = gSys.nParam[PARAM_TYPE_ALARM1].Data.ParamDW.Param;
	uIP.u32_addr = User->LY.LYIP;
	sprintf(LV->DataOut, "%02u%09u/%u/%s,%s,%s/%s,%u/%02u.%02u.%02u.%02u,%u/%u,%u,%u/%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			(int)MainInfo->UID[1], (int)MainInfo->UID[0],
			(int)gSys.Var[SOFTWARE_VERSION], pAPN->APNName, pAPN->APNUser, pAPN->APNPassword,
			MainInfo->MainURL, MainInfo->TCPPort,
			uIP.u8_addr[0], uIP.u8_addr[1], uIP.u8_addr[2], uIP.u8_addr[3], User->LY.LYTCPPort,
			(int)Param[PARAM_CRASH_JUDGE_CNT], (int)Param[GSENSOR_ALARM_VAL], (int)Param[PARAM_MOVE_RANGE],
			gSys.ICCID[0], gSys.ICCID[1], gSys.ICCID[2], gSys.ICCID[3], gSys.ICCID[4],
			gSys.ICCID[5], gSys.ICCID[6], gSys.ICCID[7], gSys.ICCID[8], gSys.ICCID[9]);
	LV->Result = 0;
	return 0;
}

int32_t LV_State(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	Date_Union uDate;
	Time_Union uTime;
	IO_ValueUnion uIO;
	uint64_t Tamp;
	Param_LocatStruct *LocatInfo = &gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo;
	uDate.dwDate = gSys.Var[UTC_DATE];
	uTime.dwTime = gSys.Var[UTC_TIME];
	Tamp = UTC2Tamp(&uDate.Date, &uTime.Time);
	uIO.Val = gSys.Var[IO_VAL];
	sprintf(LV->DataOut, "%02u%09u/%u/%u/%u,%u,%u/%u,%u/%u,%u,%u,%u/%u,%u,%u,%u/%02x%02x%02x%02x,%u/%u.%u",
			MainInfo->UID[1], MainInfo->UID[0], (int)Tamp, gSys.Var[VBAT],
			gSys.Var[GSENSOR_KEEP_VAL], uIO.IOVal.VCC, uIO.IOVal.ACC,
			gSys.State[GPS_STATE], gSys.RMCInfo->LocatStatus,
			gSys.State[REG_STATE], gSys.State[GPRS_ATTACH_STATE], gSys.State[GPRS_ACT_STATE], gSys.State[GPRS_STATE],
			gSys.State[MONITOR_STATE], gSys.State[CRASH_STATE], gSys.State[MOVE_STATE], gSys.State[CUTLINE_STATE],
			gSys.CurrentCell.nTSM_LAI[3], gSys.CurrentCell.nTSM_LAI[4], gSys.CurrentCell.nTSM_CellID[0], gSys.CurrentCell.nTSM_CellID[1],
			gSys.CurrentCell.nTSM_AvRxLevel, (int)LocatInfo->MileageKM, (int)LocatInfo->MileageM/100);
	gSys.Var[GSENSOR_KEEP_VAL] = 0;
	LV->Result = 0;
	return 0;
}

int32_t LV_LocatInfo(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	LV->Result = 0;
	GPS_RemotePrint();
	if (PRINT_GPS != gSys.State[PRINT_STATE])
	{
		gSys.State[PRINT_STATE] = PRINT_GPS;
	}
	LV->Result = 1;
	return 0;
}

int32_t LV_GetParam(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	uint32_t SnLen = strlen(LV->DataIn);
	uint8_t Sn, i;

	if (!LV->DataIn || (SnLen != 2))
	{
		LV->Result = sprintf(LV->DataOut, "%s error1\r\n", LVFun[LV->Sn].Cmd);
		LV->Result = 0;
		return 0;
	}
	Sn = AsciiToU32(LV->DataIn, 2);
	LV->DataOut[0] = 0;
	switch (Sn)
	{
	case PARAM_TYPE_SYS:
		for (i = 0; i < PARAM_SYS_MAX; i++)
		{
			sprintf(LV->DataOut, "%s|%02u %u", LV->DataOut, i, (int)gSys.nParam[Sn].Data.ParamDW.Param[i]);
		}
		break;
	case PARAM_TYPE_GPS:
		for (i = 0; i < PARAM_GPS_MAX; i++)
		{
			sprintf(LV->DataOut, "%s|%02u %u", LV->DataOut, i, (int)gSys.nParam[Sn].Data.ParamDW.Param[i]);
		}
		break;
	case PARAM_TYPE_MONITOR:
		for (i = 0; i < PARAM_MONITOR_MAX; i++)
		{
			sprintf(LV->DataOut, "%s|%02u %u", LV->DataOut, i, (int)gSys.nParam[Sn].Data.ParamDW.Param[i]);
		}
		break;
	case PARAM_TYPE_ALARM1:
		for (i = 0; i < PARAM_ALARM1_MAX; i++)
		{
			sprintf(LV->DataOut, "%s|%02u %u", LV->DataOut, i, (int)gSys.nParam[Sn].Data.ParamDW.Param[i]);
		}
		break;
	case PARAM_TYPE_ALARM2:
		for (i = 0; i < PARAM_ALARM2_MAX; i++)
		{
			sprintf(LV->DataOut, "%s|%02u %u", LV->DataOut, i, (int)gSys.nParam[Sn].Data.ParamDW.Param[i]);
		}
		break;
	default:
		LV->Result = sprintf(LV->DataOut, "%s error2\r\n", LVFun[LV->Sn].Cmd);
		LV->Result = 0;
		return 0;
	}
	return 0;
}

int32_t LV_SetParam(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	//uint32_t SnLen = strlen(LV->DataIn);
	uint8_t Sn, Param;
	uint32_t Val;
	int32_t Error = 0;
	if (!LV->DataIn)
	{
		LV->Result = sprintf(LV->DataOut, "%s error1\r\n", LVFun[LV->Sn].Cmd);
		LV->Result = 0;
		return 0;
	}
	Sn = AsciiToU32(LV->DataIn, 2);
	Param = AsciiToU32(LV->DataIn + 2, 2);
	Val = AsciiToU32(LV->DataIn + 4, strlen(LV->DataIn + 4));

	switch (Sn)
	{

	case PARAM_TYPE_SYS:
	case PARAM_TYPE_GPS:
	case PARAM_TYPE_MONITOR:
	case PARAM_TYPE_ALARM1:
	case PARAM_TYPE_ALARM2:
		break;
	default:
		LV->Result = sprintf(LV->DataOut, "%s error2\r\n", LVFun[LV->Sn].Cmd);
		LV->Result = 0;
		return 0;
	}

	if (Param >= 15)
	{
		LV->Result = sprintf(LV->DataOut, "%s error3\r\n", LVFun[LV->Sn].Cmd);
		LV->Result = 0;
		return 0;
	}

	gSys.nParam[Sn].Data.ParamDW.Param[Param] = Val;
	Error = Param_Save(Sn);
	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		LV->Result = sprintf(LV->DataOut, "%u %u=%u", Sn, Param, (int)gSys.nParam[Sn].Data.ParamDW.Param[Param]);
	}
	return 0;
}

int32_t LV_Format(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	uint32_t SnLen = strlen(LV->DataIn);
	uint8_t Sn;
	int32_t Error = 0;
	if (!LV->DataIn || (SnLen != 2))
	{
		LV->Result = sprintf(LV->DataOut, "%s error1\r\n", LVFun[LV->Sn].Cmd);
		LV->Result = 0;
		return 0;
	}
	Sn = AsciiToU32(LV->DataIn, 2);
	if (Sn >= PARAM_TYPE_MAX)
	{
		LV->Result = sprintf(LV->DataOut, "%s error2\r\n", LVFun[LV->Sn].Cmd);
		LV->Result = 0;
		return 0;
	}
	Error = Param_Format(Sn);
	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		LV->Result = sprintf(LV->DataOut, "%s %u ok\r\n", LVFun[LV->Sn].Cmd, Sn);
	}
	return 0;
}

int32_t LV_TestOn(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	gSys.State[PRINT_STATE] = PRINT_TEST;
	LV->Result = 1;
	return 0;
}

int32_t LV_TestOff(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	gSys.State[PRINT_STATE] = PRINT_NORMAL;
	LV->Result = 1;
	return 0;
}

int32_t LV_Upgrade(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	LV->Result = 1;
	FTP_StartCmd(LV->DataIn, (uint8_t *)&FileCache);
	User_GPRSUpgradeStart();
	return 0;
}

int32_t LV_Ftp(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	LV->Result = 1;
	FTP_StartCmd(LV->DataIn, (uint8_t *)&FileCache);
	User_GPRSUpgradeStart();
	return 0;
}

int32_t LV_Ble(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	LV->Result = 1;
	FTP_StartCmd(LV->DataIn, (uint8_t *)&FileCache);
	User_DevUpgradeStart();
	return 0;
}

int32_t LV_RemoteStart(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	LV->Result = 1;
	if (gSys.TaskID[REMOTE_TASK_ID])
	{
		OS_SendEvent(gSys.TaskID[REMOTE_TASK_ID], EV_MMI_START_REMOTE, 0, 0, 0);
	}
	return 0;
}

int32_t LV_Call(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	uint8_t Num[21];
	uint8_t NumLen;
	NumLen = strlen(LV->DataIn);
	if (LV->DataIn[0] == '+')
	{
		if (NumLen <= 41)
		{
			NumLen = AsciiToGsmBcd(LV->DataIn + 1, NumLen, Num);
			if (NumLen)
			{
				HexTrace(Num, NumLen);
				NumLen = OS_Call(Num, NumLen, CFW_TELNUMBER_TYPE_INTERNATIONAL);
			}
		}
	}
	else
	{
		if (NumLen < 41)
		{
			NumLen = AsciiToGsmBcd(LV->DataIn, NumLen, Num);
			if (NumLen)
			{
				HexTrace(Num, NumLen);
				NumLen = OS_Call(Num, NumLen, CFW_TELNUMBER_TYPE_UNKNOWN);
			}
		}
	}
	LV->Result = 1;
	sprintf(LV->DataOut, "%s=%u", LVFun[LV->Sn].Cmd, NumLen);
	return 0;

}


int32_t LV_GetIMEI(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	LV->Result = sprintf(LV->DataOut, "%s=%01x%02x%02x%02x%02x%02x%02x%02x",
			LVFun[LV->Sn].Cmd, gSys.IMEI[0], gSys.IMEI[1], gSys.IMEI[2], gSys.IMEI[3], gSys.IMEI[4], gSys.IMEI[5], gSys.IMEI[6], gSys.IMEI[7]);
	LV->Result = 0;
	return 0;
}

int32_t LV_IOCtrl(void *Data)
{
	LV_AnalyzeStruct *LV = (LV_AnalyzeStruct *)Data;
	uint8_t Sn,Dir,Val;
	int32_t Error = 0;

	if (!LV->DataIn || (strlen(LV->DataIn) != 4) )
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
		return 0;
	}
	Sn = LV->DataIn[0] - '0';
	Sn = (Sn * 10) + (LV->DataIn[1] - '0');
	Dir = (LV->DataIn[2] - '0');
	Val = (LV->DataIn[3] - '0');
	if (Sn >= PIN_MAX)
	{
		Error = 1;
	}
	else
	{
		if (PinParam[Sn].APO.gpioId == HAL_GPIO_NONE)
		{
			Error = 1;
		}
		else
		{
			PinParam[Sn].IsWork = 1;
			PinParam[Sn].IsRevese = 0;
			PinParam[Sn].IsOut = (Dir)?1:0;
			PinParam[Sn].InitValue = (Val)?1:0;
			GPIO_Init(&PinParam[Sn]);
			if (!PinParam[Sn].IsOut)
			{
				PinParam[Sn].InitValue = GPIO_Read(Sn);
			}
			PinParam[Sn].IsWork = 0;
		}
	}

	if (Error)
	{
		LV->Result = 0;
		sprintf(LV->DataOut, "%s error", LVFun[LV->Sn].Cmd);
	}
	else
	{
		LV->Result = sprintf(LV->DataOut, "%s=%d,%d,%d", LVFun[LV->Sn].Cmd,
				Sn, PinParam[Sn].IsOut, PinParam[Sn].InitValue);
		LV->Result = 0;
	}
	return 0;
}


const StrFunStruct LVFun[] =
{
	{
		"devid",
		LV_SetPID,
	},
	{
		"setdns",
		LV_SetMainUrl,
	},
	{
		"setapn",
		LV_SetAPN,
	},
	{
		"setodo",
		LV_SetMileage,
	},
	{
		"sethearttime",
		LV_SetHeart,
	},
	{
		"setmovetime",
		LV_SetNormal,
	},
	{
		"setstoptime",
		LV_SetStop,
	},
	{
		"setcrashlevel",
		LV_SetCrash,
	},
	{
		"setalarmnumber",
		LV_SetPhone,
	},
	{
		"resetdev",
		LV_Reboot,
	},
	{
		"findpos",
		LV_FindPosition,
	},
	{
		"setstealradius",
		LV_SetMoveRange,
	},
	{
		"sat",
		LV_Sat,
	},
	{
		"state",
		LV_State,
	},
	{
		"getparam",
		LV_GetParam,
	},
	{
		"setparam",
		LV_SetParam,
	},
	{
		"format",
		LV_Format,
	},
	{
		"pctestend",
		LV_TestOff,
	},
	{
		"pcteststart",
		LV_TestOn,
	},
	{
		"upgrade",
		LV_Upgrade,
	},
	{
		"ftp",
		LV_Ftp,
	},
	{
		"ble",
		LV_Ble,
	},
	{
		"remote",
		LV_RemoteStart,
	},
	{
		"locat",
		LV_LocatInfo,
	},
	{
		"call",
		LV_Call,
	},
	{
		"getimei",
		LV_GetIMEI,
	},
	{
		"gpio",
		LV_IOCtrl,
	}
};

void LV_Print(uint8_t *Buf)
{
	uint8_t GNSSCN[52];
	uint8_t GNSSMAX[4];
	IO_ValueUnion uIO;
	IP_AddrUnion uIP;
	uint32_t TxLen;
	uint32_t i,j;
	Param_UserStruct *User = &gSys.nParam[PARAM_TYPE_USER].Data.UserInfo;
	Param_DWStruct *Monitor = &gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW;
	Param_APNStruct *APN = &gSys.nParam[PARAM_TYPE_APN].Data.APN;
	Param_DWStruct *Alarm1 = &gSys.nParam[PARAM_TYPE_ALARM1].Data.ParamDW;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	uint8_t PhoneDec[12];
	Param_NumberStruct *Number = &gSys.nParam[PARAM_TYPE_NUMBER].Data.Number;
	Param_LocatStruct *LocatInfo = &gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo;

	memset(GNSSCN, 0, sizeof(GNSSCN));
	for (i = 0; i < gSys.GSVInfoSave.Pos[0]; i++)
	{
		if (gSys.GSVInfoSave.CN[0][i] >= 51)
		{
			GNSSCN[51]++;
		}
		else
		{
			GNSSCN[gSys.GSVInfoSave.CN[0][i]]++;
		}
	}

	for (i = 0; i < gSys.GSVInfoSave.Pos[1]; i++)
	{
		if (gSys.GSVInfoSave.CN[1][i] >= 51)
		{
			GNSSCN[51]++;
		}
		else
		{
			GNSSCN[gSys.GSVInfoSave.CN[1][i]]++;
		}
	}

	j = 0;
	memset(GNSSMAX, 0, 4);
	for (i = 52; i > 0; i--)
	{
		if (GNSSCN[i - 1])
		{
			GNSSMAX[j] = i - 1;
			j++;
			if (j >= 4)
			{
				break;
			}
			GNSSCN[i - 1]--;
		}
	}

	uIO.Val = gSys.Var[IO_VAL];

	TxLen = sprintf(Buf, "GDTMINFO:%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\r\n",
			(int)MainInfo->UID[0], (int)gSys.State[GPS_STATE], (int)gSys.RMCInfo->LocatStatus, (int)gSys.GSVInfoSave.Pos[0] + gSys.GSVInfoSave.Pos[1],
			(int)gSys.GSVInfoSave.Pos[0] + gSys.GSVInfoSave.Pos[1], (int)GNSSMAX[0], (int)GNSSMAX[1], (int)GNSSMAX[2], (int)GNSSMAX[3],
			(int)gSys.State[SIM_STATE], (int)gSys.State[RSSI_STATE], (int)gSys.CurrentCell.nTSM_BER, (int)gSys.State[GPRS_STATE],
			(int)uIO.IOVal.VCC, (int)uIO.IOVal.ACC, (int)gSys.Var[GSENSOR_KEEP_VAL]);
	gSys.Var[GSENSOR_KEEP_VAL] = 0;
	COM_Tx(Buf, TxLen);

	TxLen = sprintf(Buf, "GDTMVER:%u.0.%u\r\n", gSys.Var[SOFTWARE_VERSION]/1000000, gSys.Var[SOFTWARE_VERSION]%1000000);
	COM_Tx(Buf, TxLen);

	TxLen = sprintf(Buf, "GDTMDNS:%s,%u,%u\r\n", MainInfo->MainURL, MainInfo->TCPPort, (int)gSys.State[MONITOR_STATE]);
	COM_Tx(Buf, TxLen);

	TxLen = sprintf(Buf, "GDTMICCID:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",
			gSys.ICCID[0], gSys.ICCID[1], gSys.ICCID[2], gSys.ICCID[3], gSys.ICCID[4],
			gSys.ICCID[5], gSys.ICCID[6], gSys.ICCID[7], gSys.ICCID[8], gSys.ICCID[9]);
	COM_Tx(Buf, TxLen);
	sprintf(PhoneDec, "%02u%02u%02u%02u%02u%01d", Number->Phone[0].Num[1], Number->Phone[0].Num[2],
			Number->Phone[0].Num[3], Number->Phone[0].Num[4], Number->Phone[0].Num[5], Number->Phone[0].Num[6] >> 4);
	uIP.u32_addr = User->LY.LYIP;
	TxLen = sprintf(Buf, "GDTMMULTI:%u.%u.%u.%u,%u,%s,%s,%s,%u,%s,%u,%u,%u,%u,%u,%u,%u\r\n",
			uIP.u8_addr[0], uIP.u8_addr[1], uIP.u8_addr[2], uIP.u8_addr[3], User->LY.LYTCPPort,
			APN->APNName, APN->APNUser, APN->APNPassword, gSys.Var[VBAT], PhoneDec,
			Monitor->Param[PARAM_UPLOAD_HEART_PERIOD], Monitor->Param[PARAM_UPLOAD_RUN_PERIOD],
			Monitor->Param[PARAM_UPLOAD_STOP_PERIOD], Alarm1->Param[PARAM_CRASH_JUDGE_CNT],
			Alarm1->Param[PARAM_CRASH_GS], (LocatInfo->MileageKM * 1000 + LocatInfo->MileageM) / 100, Alarm1->Param[PARAM_MOVE_RANGE]);
	COM_Tx(Buf, TxLen);
#ifdef __AD_ENABLE__
	TxLen = sprintf(Buf, "GDTMADC0:%u\r\n", gSys.Var[ADC0_VAL]);
	COM_Tx(Buf, TxLen);
#endif
}

uint8_t LV_CheckHead(uint8_t Data)
{
	if ( (Data == 'A') || (Data == 'a') || (Data == '*'))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t LV_Receive(COM_CtrlStruct *COM, uint8_t Data)
{
	if ( (Data == '\r') || (Data == '\n') || (Data == '#'))
	{
		//DBG("%u", Data);
		if (Data == '#')
		{
			COM->RxBuf[COM->RxPos++] = Data;
		}
		COM->AnalyzeLen = COM->RxPos % COM_BUF_LEN;
		memcpy(COM->AnalyzeBuf, COM->RxBuf, COM->AnalyzeLen);
		COM->AnalyzeBuf[COM->AnalyzeLen] = 0;
		return 1;
	}
	else
	{
		COM->RxBuf[COM->RxPos++] = Data;
		return 0;
	}
}

void LV_Analyze(LV_AnalyzeStruct *LV)
{
	uint8_t i;
	LV->Result = -1;
	for (i = 0; i < sizeof(LVFun)/sizeof(StrFunStruct); i++)
	{
		if (!memcmp(LV->Cmd, LVFun[i].Cmd, LV->CmdLen))
		{
			DBG("find %s", LV->Cmd);
			LV->Sn = i;
			LVFun[i].Func(LV);
			break;
		}
	}
}

void LV_SMSAnalyze(uint8_t *InBuf, uint32_t InLen, uint8_t *OutBuf, uint32_t *OutLen)
{
	uint32_t i, CutPos;
	CutPos = 0;
	LV_AnalyzeStruct LV;

	for (i = 0;i < InLen; i++)
	{
		if ( (InBuf[i] >= 'A') && (InBuf[i] <= 'Z') )
		{
			InBuf[i] = InBuf[i] - 'A' + 'a';
		}
		if (InBuf[i] == ':')
		{
			if (!CutPos)
			{
				CutPos = i;
			}
		}
	}

	LV.Cmd = InBuf;
	LV.DataOut = OutBuf;
	LV.Result = 0;
	if (CutPos)
	{
		LV.DataIn = &InBuf[CutPos + 1];
		LV.CmdLen = CutPos;
	}
	else
	{
		LV.DataIn = NULL;
		LV.CmdLen = InLen;
	}
	LV_Analyze(&LV);
	*OutLen = strlen(LV.DataOut);
}

int32_t LV_ComAnalyze(COM_CtrlStruct *COM)
{
	uint32_t i, CutPos;
	CutPos = 0;
	LV_AnalyzeStruct LV;
#if (__CUST_CODE__ == __CUST_KQ__)
	if (!memcmp(COM->AnalyzeBuf, "*FCREPORT#", 10))
	{
		CutPos = KQ_BLEReport(COM->TempBuf, sizeof(COM->TempBuf));
		COM_Tx(COM->TempBuf, CutPos);
		return 0;
	}
#endif
	for (i = 0;i < COM->AnalyzeLen; i++)
	{
		if ( (COM->AnalyzeBuf[i] >= 'A') && (COM->AnalyzeBuf[i] <= 'Z') )
		{
			COM->AnalyzeBuf[i] = COM->AnalyzeBuf[i] - 'A' + 'a';
		}
		if (COM->AnalyzeBuf[i] == ':')
		{
			if (!CutPos)
			{
				CutPos = i;
			}
		}
		if ( (COM->AnalyzeBuf[i] == '\r') || (COM->AnalyzeBuf[i] == '\n') )
		{
			COM->AnalyzeBuf[i] = 0;
		}
	}

	if (memcmp(COM->AnalyzeBuf, "at+", 3))
	{
		DBG("head error %c%c%c", COM->AnalyzeBuf[0], COM->AnalyzeBuf[1], COM->AnalyzeBuf[2]);
		return -1;
	}
	LV.Cmd = COM->AnalyzeBuf + 3;
	LV.DataOut = COS_MALLOC(1024);
	if (!LV.DataOut)
	{
		return 0;
	}
	memset(LV.DataOut, 0, 1024);
	if (CutPos)
	{
		LV.DataIn = &COM->AnalyzeBuf[CutPos + 1];
		LV.CmdLen = CutPos - 3;
	}
	else
	{
		LV.DataIn = NULL;
		LV.CmdLen = COM->AnalyzeLen - 3;
	}
	LV_Analyze(&LV);
	if (LV.Result > 0)
	{
		COM_Tx("GDTMCMDOK\r\n", strlen("GDTMCMDOK\r\n"));
	}
	else
	{
		COM_Tx(LV.DataOut, strlen(LV.DataOut));
		COM_Tx("\r\n", 2);
	}
	COS_FREE(LV.DataOut);
	return 0;
}
