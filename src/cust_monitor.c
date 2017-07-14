#include "user.h"
#define MONITOR_CACHE_DEBUG
Monitor_CacheStruct __attribute__((section (".cache_ram"))) Cache;
void Monitor_InitCache(void)
{
	u8 RecoveryFlag = 0;
	if ( (Cache.AlarmBuf.Data != (u8 *)&Cache.AlarmCache[0]) || (Cache.AlarmBuf.MaxLength != ALARM_CACHE_MAX) )
	{
		DBG("Alarm cache %08x %08x", Cache.AlarmBuf.Data, (u8 *)&Cache.AlarmCache[0]);
		RecoveryFlag = 1;
	}

	if ( (Cache.DataBuf.Data != (u8 *)&Cache.DataCache[0]) || (Cache.DataBuf.MaxLength != DATA_CACHE_MAX) )
	{
		DBG("Data cache %08x %08x", Cache.DataBuf.Data, (u8 *)&Cache.DataCache[0]);
		RecoveryFlag = 1;
	}

	if ( (Cache.ResBuf.Data != (u8 *)&Cache.ResCache[0]) || (Cache.ResBuf.MaxLength != RES_CACHE_MAX) )
	{
		DBG("Res cache %08x %08x", Cache.ResBuf.Data, (u8 *)&Cache.ResCache[0]);
		RecoveryFlag = 1;
	}

	if (RecoveryFlag)
	{
		InitRBuffer(&Cache.AlarmBuf, (u8 *)&Cache.AlarmCache[0], ALARM_CACHE_MAX, sizeof(Monitor_DataStruct));
		InitRBuffer(&Cache.DataBuf, (u8 *)&Cache.DataCache[0], DATA_CACHE_MAX, sizeof(Monitor_DataStruct));
		InitRBuffer(&Cache.ResBuf, (u8 *)&Cache.ResCache[0], RES_CACHE_MAX, sizeof(Monitor_ResponseStruct));
	}
	DBG("%d %d %d", Cache.ResBuf.Len, Cache.AlarmBuf.Len, Cache.DataBuf.Len);
}

void Monitor_Record(Monitor_DataStruct *MonitorData)
{
	u8 i,j;
	MonitorData->uRecord.Data.uDate.dwDate = gSys.Var[UTC_DATE];
	MonitorData->uRecord.Data.uTime.dwTime = gSys.Var[UTC_TIME];
	MonitorData->uRecord.Data.CellInfoUnion.CellID = gSys.Var[CELL_ID];
	MonitorData->uRecord.Data.GsensorVal = gSys.Var[GSENSOR_MONITOR_VAL];
	gSys.Var[GSENSOR_MONITOR_VAL] = 0;
	MonitorData->uRecord.Data.Vbat = gSys.Var[VBAT];
	MonitorData->uRecord.Data.MileageKM = gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageKM;
	MonitorData->uRecord.Data.MileageM = gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageM;
	MonitorData->uRecord.Data.RMC = gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave;
	MonitorData->uRecord.Data.IOValUnion.Val = gSys.Var[IO_VAL];
	MonitorData->uRecord.Data.DevStatus[MONITOR_STATUS_ALARM_ON] = gSys.State[ALARM_STATE];
	MonitorData->uRecord.Data.DevStatus[MONITOR_STATUS_SIGNAL] = gSys.State[RSSI_STATE];
	MonitorData->uRecord.Data.DevStatus[MONITOR_STATUS_ANT_SHORT] = gSys.Error[ANT_SHORT_ERROR];
	MonitorData->uRecord.Data.DevStatus[MONITOR_STATUS_ANT_BREAK] = gSys.Error[ANT_BREAK_ERROR];
	MonitorData->uRecord.Data.DevStatus[MONITOR_STATUS_GPS_ERROR] = gSys.Error[GPS_ERROR];
	MonitorData->uRecord.Data.DevStatus[MONITOR_STATUS_LOWPOWER] = (gSys.State[SYSTEM_STATE] == SYSTEM_POWER_ON)?0:1;
	MonitorData->uRecord.Data.DevStatus[MONITOR_STATUS_SENSOR_ERROR] = gSys.Error[SENSOR_ERROR];
	MonitorData->uRecord.Data.DevStatus[MONITOR_STATUS_SIM_ERROR] = gSys.Error[SIM_ERROR];
	MonitorData->uRecord.Data.DevStatus[MONITOR_STATUS_GPRS_ERROR] = gSys.Error[GPRS_ERROR];
	MonitorData->uRecord.Data.DevStatus[MONITOR_STATUS_OVERSPEED] = gSys.State[OVERSPEED_STATE];
	for (i = 0; i < gSys.GSVInfoSave.Pos[0]; i++)
	{
		if (gSys.GSVInfoSave.CN[0][i] >= 44)
		{
			MonitorData->uRecord.Data.CN[0]++;
		}
		else if (gSys.GSVInfoSave.CN[0][i] >= 40)
		{
			MonitorData->uRecord.Data.CN[1]++;
		}
		else if (gSys.GSVInfoSave.CN[0][i] >= 35)
		{
			MonitorData->uRecord.Data.CN[2]++;
		}
		else if (gSys.GSVInfoSave.CN[0][i] >= 28)
		{
			MonitorData->uRecord.Data.CN[3]++;
		}
	}

	for (i = 0; i < gSys.GSVInfoSave.Pos[1]; i++)
	{
		if (gSys.GSVInfoSave.CN[1][i] >= 44)
		{
			MonitorData->uRecord.Data.CN[0]++;
		}
		else if (gSys.GSVInfoSave.CN[1][i] >= 40)
		{
			MonitorData->uRecord.Data.CN[1]++;
		}
		else if (gSys.GSVInfoSave.CN[1][i] >= 35)
		{
			MonitorData->uRecord.Data.CN[2]++;
		}
		else if (gSys.GSVInfoSave.CN[1][i] >= 28)
		{
			MonitorData->uRecord.Data.CN[3]++;
		}
	}
	j = 0;
	for (i = 0; i < gSys.NearbyCell.nTSM_NebCellNUM; i++)
	{
		if (gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[0])
		{
			MonitorData->uRecord.Data.NearCellInfoUnion[j].CellInfo.ID[0] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[0];
			MonitorData->uRecord.Data.NearCellInfoUnion[j].CellInfo.ID[1] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[1];
			MonitorData->uRecord.Data.NearCellInfoUnion[j].CellInfo.ID[2] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_LAI[3];
			MonitorData->uRecord.Data.NearCellInfoUnion[j].CellInfo.ID[3] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_LAI[4];
			MonitorData->uRecord.Data.NearSingal[j] = RssiToCSQ(gSys.NearbyCell.nTSM_NebCell[i].nTSM_AvRxLevel);
			j++;
			if (j >= 2)
				break;
		}
	}
}

void Monitor_RecordData(void)
{
	Monitor_DataStruct MonitorData;
	memset(&MonitorData, 0, sizeof(Monitor_DataStruct));
	if (gSys.RMCInfo->LocatStatus)
	{
		SYS_CheckTime(&gSys.RMCInfo->UTCDate, &gSys.RMCInfo->UTCTime);
	}
	Monitor_Record(&MonitorData);
	MonitorData.CRC32 = __CRC32((u8 *)&MonitorData.uRecord.Data, sizeof(Monitor_RecordStruct), CRC32_START);
	WriteRBufferForce(&Cache.DataBuf, (u8 *)&MonitorData, 1);
#ifdef MONITOR_CACHE_DEBUG
	if (Cache.DataBuf.Len > 1)
	{
		DBG("Data cache len %d", Cache.DataBuf.Len);
	}
#endif
}

void Monitor_RecordAlarm(u8 Type, u16 CrashCNT, u16 MoveCNT)
{
	Monitor_DataStruct MonitorData;
	memset(&MonitorData, 0, sizeof(Monitor_DataStruct));
	Monitor_Record(&MonitorData);
	MonitorData.uRecord.Data.CrashCNT = CrashCNT;
	MonitorData.uRecord.Data.MoveCNT = MoveCNT;
	MonitorData.uRecord.Data.Alarm = Type;
	if ((Type == ALARM_TYPE_CRASH) || (Type == ALARM_TYPE_CUTLINE) || (Type == ALARM_TYPE_ACC_ON))
	{
		if (gSys.TaskID[REMOTE_TASK_ID])
		{
			OS_SendEvent(gSys.TaskID[REMOTE_TASK_ID], EV_MMI_START_REMOTE, Type, 0, 0);
		}
	}

	MonitorData.CRC32 = __CRC32((u8 *)&MonitorData.uRecord.Data, sizeof(Monitor_RecordStruct), CRC32_START);
	WriteRBufferForce(&Cache.AlarmBuf, (u8 *)&MonitorData, 1);
	Monitor_Wakeup();
#ifdef MONITOR_CACHE_DEBUG
	if (Cache.AlarmBuf.Len > 1)
	{
		DBG("Alarm cache len %d", Cache.AlarmBuf.Len);
	}
#endif
}

void Monitor_RecordResponse(u8 *Data, u32 Len)
{
	Monitor_ResponseStruct MonitorRes;
	memset(&MonitorRes, 0, sizeof(Monitor_ResponseStruct));
	if (Len >= 1024)
	{
		Len = 1024;
	}
	memcpy(MonitorRes.Data, Data, Len);
	MonitorRes.Len = Len;
	MonitorRes.CRC32 = __CRC32(MonitorRes.Data, Len, CRC32_START);
	WriteRBufferForce(&Cache.ResBuf, (u8 *)&MonitorRes, 1);
	if (!gSys.Monitor->IsWork)
	{
		Monitor_Wakeup();
	}
	else
	{
		OS_SendEvent(gSys.TaskID[MONITOR_TASK_ID], EV_MMI_MONITOR_WAKEUP, 0, 0, 0);
	}
#ifdef MONITOR_CACHE_DEBUG
	if (Cache.ResBuf.Len > 1)
	{
		DBG("Res cache len %d", Cache.ResBuf.Len);
	}
#endif
}

u8 Monitor_ExtractData(Monitor_RecordStruct *Data)
{
	u8 Len = 0;
	Monitor_DataStruct MonitorData;
	while (Cache.DataBuf.Len)
	{
		QueryRBuffer(&Cache.DataBuf, (u8 *)&MonitorData, 1);
		if (MonitorData.CRC32 != __CRC32((u8 *)&MonitorData.uRecord.Data, sizeof(Monitor_RecordStruct), CRC32_START))
		{
			DBG("%d data error!", Cache.DataBuf.Offset);
			DelRBuffer(&Cache.DataBuf, 1);
		}
		else
		{
			memcpy(Data, &MonitorData.uRecord.Data, sizeof(Monitor_RecordStruct));
			Len = 1;
			break;
		}
	}
#ifdef MONITOR_CACHE_DEBUG
	if (Cache.DataBuf.Len > 1)
	{
		DBG("Data cache len %d", Cache.DataBuf.Len);
	}
#endif
	return Len;
}

u8 Monitor_ExtractAlarm(Monitor_RecordStruct *Alarm)
{
	u8 Len = 0;
	Monitor_DataStruct MonitorData;
	while (Cache.AlarmBuf.Len)
	{
		QueryRBuffer(&Cache.AlarmBuf, (u8 *)&MonitorData, 1);
		if (MonitorData.CRC32 != __CRC32((u8 *)&MonitorData.uRecord.Data, sizeof(Monitor_RecordStruct), CRC32_START))
		{
			DBG("%d alarm error!", Cache.AlarmBuf.Offset);
			DelRBuffer(&Cache.AlarmBuf, 1);
		}
		else
		{
			memcpy(Alarm, &MonitorData.uRecord.Data, sizeof(Monitor_RecordStruct));
			Len = 1;
			break;
		}
	}
#ifdef MONITOR_CACHE_DEBUG
	if (Cache.AlarmBuf.Len > 1)
	{
		DBG("Alarm cache len %d", Cache.AlarmBuf.Len);
	}
#endif
	return Len;
}

u32 Monitor_ExtractResponse(u8 *Response)
{
	u32 Len = 0;
	Monitor_ResponseStruct MonitorRes;
	while (Cache.ResBuf.Len)
	{
		QueryRBuffer(&Cache.ResBuf, (u8 *)&MonitorRes, 1);
		if (MonitorRes.Len > 1024)
		{
			DBG("%d response len error!", Cache.ResBuf.Offset);
			DelRBuffer(&Cache.ResBuf, 1);
			continue;
		}
		if (MonitorRes.CRC32 != __CRC32(&MonitorRes.Data[0], MonitorRes.Len, CRC32_START))
		{
			DBG("%d response error!", Cache.ResBuf.Offset);
			DelRBuffer(&Cache.ResBuf, 1);
		}
		else
		{
			memcpy(Response, MonitorRes.Data, MonitorRes.Len);
			Len = MonitorRes.Len;
			break;
		}
	}
#ifdef MONITOR_CACHE_DEBUG
	if (Cache.ResBuf.Len > 1)
	{
		DBG("Res cache len %d", Cache.ResBuf.Len);
	}
#endif
	return Len;
}

void Monitor_DelCache(u8 Type, u8 IsAll)
{
	switch (Type)
	{
	case CACHE_TYPE_RES:
		if (IsAll)
		{
			Cache.ResBuf.Len = 0;
			Cache.ResBuf.Offset = 0;
		}
		else
		{
			DelRBuffer(&Cache.ResBuf, 1);
		}
#ifdef MONITOR_CACHE_DEBUG
		if (Cache.ResBuf.Len)
		{
			DBG("Res cache len %d", Cache.ResBuf.Len);
		}
#endif
		break;
	case CACHE_TYPE_ALARM:
		if (IsAll)
		{
			Cache.AlarmBuf.Len = 0;
			Cache.AlarmBuf.Offset = 0;
		}
		else
		{
			DelRBuffer(&Cache.AlarmBuf, 1);
		}
#ifdef MONITOR_CACHE_DEBUG
		if (Cache.AlarmBuf.Len)
		{
			DBG("Alarm cache len %d", Cache.AlarmBuf.Len);
		}
#endif
		break;
	case CACHE_TYPE_DATA:
		if (IsAll)
		{
			Cache.DataBuf.Len = 0;
			Cache.DataBuf.Offset = 0;
		}
		else
		{
			DelRBuffer(&Cache.DataBuf, 1);
		}
#ifdef MONITOR_CACHE_DEBUG
		if (Cache.DataBuf.Len)
		{
			DBG("Data cache len %d", Cache.DataBuf.Len);
		}
#endif
		break;
	}
}

u32 Monitor_GetCacheLen(u8 Type)
{
	switch (Type)
	{
	case CACHE_TYPE_RES:
		return Cache.ResBuf.Len;

	case CACHE_TYPE_ALARM:
		return Cache.AlarmBuf.Len;

	case CACHE_TYPE_DATA:
		return Cache.DataBuf.Len;

	case CACHE_TYPE_ALL:
		return Cache.DataBuf.Len + Cache.AlarmBuf.Len + Cache.ResBuf.Len;
	}
	return 0;
}

void Monitor_Wakeup(void)
{
	gSys.Monitor->IsWork = 1;
	gSys.Monitor->RunStartTime = gSys.Var[SYS_TIME] + MONITOR_RUN_TIME;
	OS_SendEvent(gSys.TaskID[MONITOR_TASK_ID], EV_MMI_MONITOR_WAKEUP, 0, 0, 0);
}

void Monitor_Heart(void)
{
	gSys.Monitor->HeartStartTime = gSys.Var[SYS_TIME] + gSys.Monitor->Param[PARAM_UPLOAD_HEART_PERIOD];
	OS_SendEvent(gSys.TaskID[MONITOR_TASK_ID], EV_MMI_MONITOR_HEART, 0, 0, 0);
}

void Monitor_Upload(void)
{
	OS_SendEvent(gSys.TaskID[MONITOR_TASK_ID], EV_MMI_MONITOR_UPLOAD, 0, 0, 0);
}

void Monitor_StateCheck(void)
{
	IO_ValueUnion IO;
	if (gSys.Monitor->Param[PARAM_GS_WAKEUP_MONITOR])
	{
		if (gSys.Var[GSENSOR_ALARM_VAL] >= G_POWER(gSys.Monitor->Param[PARAM_GS_WAKEUP_MONITOR]))
		{
			gSys.Monitor->WakeupFlag = 1;
		}
	}
	else
	{
		IO.Val = gSys.Var[IO_VAL];
		if (IO.IOVal.VACC)
		{
			gSys.Monitor->WakeupFlag = 1;
		}
	}



	if (gSys.Monitor->IsWork) //监控处于工作状态时
	{
		if (gSys.Monitor->Param[PARAM_GS_JUDGE_RUN])
		{
			if (gSys.Var[GSENSOR_ALARM_VAL] >= G_POWER(gSys.Monitor->Param[PARAM_GS_JUDGE_RUN]))
			{
				gSys.Monitor->RunStartTime = gSys.Var[SYS_TIME] + MONITOR_RUN_TIME;
			}
		}
		else
		{
			gSys.Monitor->RunStartTime = gSys.Var[SYS_TIME] + MONITOR_RUN_TIME;
		}

		if (gSys.Var[SYS_TIME] > gSys.Monitor->RunStartTime)
		{
			if (gSys.Monitor->IsRunMode)
			{
				gSys.Monitor->IsRunMode = 0;
				DBG("Entry car stop mode! %d %d", gSys.Var[SYS_TIME], gSys.Monitor->RunStartTime);
#ifdef __UART_AUTO_SLEEP_BY_RUN__
				COM_Sleep();
#endif
			}
		}
		else
		{
			if (!gSys.Monitor->IsRunMode)
			{
				gSys.Monitor->IsRunMode = 1;
				DBG("Entry car run mode!");
				if (gSys.Monitor->RecordStartTime >= gSys.Monitor->Param[PARAM_UPLOAD_STOP_PERIOD])
				{
					gSys.Monitor->RecordStartTime -= (gSys.Monitor->Param[PARAM_UPLOAD_STOP_PERIOD] - gSys.Monitor->Param[PARAM_UPLOAD_RUN_PERIOD]);
				}
				else
				{
					gSys.Monitor->RecordStartTime = gSys.Var[SYS_TIME] + gSys.Monitor->Param[PARAM_UPLOAD_RUN_PERIOD];
				}
#ifdef __UART_AUTO_SLEEP_BY_RUN__
				COM_Wakeup(gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_COM_BR]);
#endif
			}
		}

		if (gSys.Var[SYS_TIME] >= gSys.Monitor->RecordStartTime)
		{
			Monitor_RecordData();
			if (gSys.Monitor->IsRunMode)
			{
				gSys.Monitor->RecordStartTime = gSys.Var[SYS_TIME] + gSys.Monitor->Param[PARAM_UPLOAD_RUN_PERIOD];
			}
			else
			{
				gSys.Monitor->RecordStartTime = gSys.Var[SYS_TIME] + gSys.Monitor->Param[PARAM_UPLOAD_STOP_PERIOD];
			}
			Monitor_Upload();
		}

		if (gSys.Var[SYS_TIME] >= gSys.Monitor->HeartStartTime)
		{
			Monitor_Heart();

		}
	}
	else
	{
		gSys.Monitor->RunStartTime = gSys.Var[SYS_TIME] + MONITOR_RUN_TIME;
		if (gSys.Monitor->WakeupFlag)
		{
			//发出唤醒事件
			Monitor_Wakeup();
		}
	}
}
