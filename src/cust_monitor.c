#include "user.h"
#define MONITOR_CACHE_DEBUG
Monitor_CacheStruct __attribute__((section (".cache_ram"))) Cache;
void Monitor_InitCache(void)
{
	uint8_t RecoveryFlag = 0;
	if ( (Cache.AlarmBuf.Data != (uint8_t *)&Cache.AlarmCache[0]) || (Cache.AlarmBuf.MaxLength != ALARM_CACHE_MAX) )
	{
		DBG("Alarm cache %08x %08x", Cache.AlarmBuf.Data, (uint8_t *)&Cache.AlarmCache[0]);
		RecoveryFlag = 1;
	}

	if ( (Cache.DataBuf.Data != (uint8_t *)&Cache.DataCache[0]) || (Cache.DataBuf.MaxLength != DATA_CACHE_MAX) )
	{
		DBG("Data cache %08x %08x", Cache.DataBuf.Data, (uint8_t *)&Cache.DataCache[0]);
		RecoveryFlag = 1;
	}

	if ( (Cache.ResBuf.Data != (uint8_t *)&Cache.ResCache[0]) || (Cache.ResBuf.MaxLength != RES_CACHE_MAX) )
	{
		DBG("Res cache %08x %08x", Cache.ResBuf.Data, (uint8_t *)&Cache.ResCache[0]);
		RecoveryFlag = 1;
	}

	if (RecoveryFlag)
	{
		InitRBuffer(&Cache.AlarmBuf, (uint8_t *)&Cache.AlarmCache[0], ALARM_CACHE_MAX, sizeof(Monitor_DataStruct));
		InitRBuffer(&Cache.DataBuf, (uint8_t *)&Cache.DataCache[0], DATA_CACHE_MAX, sizeof(Monitor_DataStruct));
		InitRBuffer(&Cache.ResBuf, (uint8_t *)&Cache.ResCache[0], RES_CACHE_MAX, sizeof(Monitor_ResponseStruct));
	}
	DBG("%u %u %u %u", sizeof(Monitor_RecordStruct), Cache.ResBuf.Len, Cache.AlarmBuf.Len, Cache.DataBuf.Len);
	memset(&gSys.RecordCollect, 0, sizeof(gSys.RecordCollect));
	gSys.RecordCollect.Param = gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param;
	gSys.RecordCollect.RunStartTime = gSys.Var[SYS_TIME] + MONITOR_RUN_TIME;
	gSys.RecordCollect.IsRunMode = 1;
}

void Monitor_Record(Monitor_RecordStruct *Record)
{
	uint8_t i,j;
	Record->uDate.dwDate = gSys.Var[UTC_DATE];
	Record->uTime.dwTime = gSys.Var[UTC_TIME];
	Record->CellInfoUnion.CellID = gSys.Var[CELL_ID];
	Record->GsensorVal = gSys.Var[GSENSOR_MONITOR_VAL];
	gSys.Var[GSENSOR_MONITOR_VAL] = 0;
	Record->Vbat = gSys.Var[VBAT];
	Record->MileageKM = gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageKM;
	Record->MileageM = gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.MileageM;
	Record->RMC = gSys.nParam[PARAM_TYPE_LOCAT].Data.LocatInfo.RMCSave;
	Record->IOValUnion.Val = gSys.Var[IO_VAL];
	Record->DevStatus[MONITOR_STATUS_ALARM_ON] = gSys.State[ALARM_STATE];
	Record->DevStatus[MONITOR_STATUS_SIGNAL] = gSys.State[RSSI_STATE];
	Record->DevStatus[MONITOR_STATUS_ANT_SHORT] = gSys.Error[ANT_SHORT_ERROR];
	Record->DevStatus[MONITOR_STATUS_ANT_BREAK] = gSys.Error[ANT_BREAK_ERROR];
	Record->DevStatus[MONITOR_STATUS_GPS_ERROR] = gSys.Error[GPS_ERROR];
	Record->DevStatus[MONITOR_STATUS_LOWPOWER] = (gSys.State[SYSTEM_STATE] == SYSTEM_POWER_ON)?0:1;
	Record->DevStatus[MONITOR_STATUS_SENSOR_ERROR] = gSys.Error[SENSOR_ERROR];
	Record->DevStatus[MONITOR_STATUS_SIM_ERROR] = gSys.Error[SIM_ERROR];
	Record->DevStatus[MONITOR_STATUS_GPRS_ERROR] = gSys.Error[GPRS_ERROR];
	Record->DevStatus[MONITOR_STATUS_OVERSPEED] = gSys.State[OVERSPEED_STATE];
	for (i = 0; i < gSys.GSVInfoSave.Pos[0]; i++)
	{
		if (gSys.GSVInfoSave.CN[0][i] >= 44)
		{
			Record->CN[0]++;
		}
		else if (gSys.GSVInfoSave.CN[0][i] >= 40)
		{
			Record->CN[1]++;
		}
		else if (gSys.GSVInfoSave.CN[0][i] >= 35)
		{
			Record->CN[2]++;
		}
		else if (gSys.GSVInfoSave.CN[0][i] >= 28)
		{
			Record->CN[3]++;
		}
	}

	for (i = 0; i < gSys.GSVInfoSave.Pos[1]; i++)
	{
		if (gSys.GSVInfoSave.CN[1][i] >= 44)
		{
			Record->CN[0]++;
		}
		else if (gSys.GSVInfoSave.CN[1][i] >= 40)
		{
			Record->CN[1]++;
		}
		else if (gSys.GSVInfoSave.CN[1][i] >= 35)
		{
			Record->CN[2]++;
		}
		else if (gSys.GSVInfoSave.CN[1][i] >= 28)
		{
			Record->CN[3]++;
		}
	}
	j = 0;
	for (i = 0; i < gSys.NearbyCell.nTSM_NebCellNUM; i++)
	{
		if (gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[0])
		{
			Record->NearCellInfoUnion[j].CellInfo.ID[0] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[0];
			Record->NearCellInfoUnion[j].CellInfo.ID[1] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[1];
			Record->NearCellInfoUnion[j].CellInfo.ID[2] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_LAI[3];
			Record->NearCellInfoUnion[j].CellInfo.ID[3] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_LAI[4];
			Record->NearSingal[j] = RssiToCSQ(gSys.NearbyCell.nTSM_NebCell[i].nTSM_AvRxLevel);
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
	Monitor_Record(&MonitorData.uRecord.Data);
	MonitorData.CRC32 = __CRC32((uint8_t *)&MonitorData.uRecord.Data, sizeof(Monitor_RecordStruct), CRC32_START);
	WriteRBufferForce(&Cache.DataBuf, (uint8_t *)&MonitorData, 1);
#ifdef MONITOR_CACHE_DEBUG
	if (Cache.DataBuf.Len > 1)
	{
		DBG("Data cache len %u", Cache.DataBuf.Len);
	}
#endif
}

void Monitor_RecordAlarm(uint8_t Type, uint16_t CrashCNT, uint16_t MoveCNT)
{
	Monitor_DataStruct MonitorData;
	memset(&MonitorData, 0, sizeof(Monitor_DataStruct));
	Monitor_Record(&MonitorData.uRecord.Data);
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

	MonitorData.CRC32 = __CRC32((uint8_t *)&MonitorData.uRecord.Data, sizeof(Monitor_RecordStruct), CRC32_START);
	WriteRBufferForce(&Cache.AlarmBuf, (uint8_t *)&MonitorData, 1);
	Monitor_Wakeup();
#ifdef MONITOR_CACHE_DEBUG
	if (Cache.AlarmBuf.Len > 1)
	{
		DBG("Alarm cache len %u", Cache.AlarmBuf.Len);
	}
#endif
}

void Monitor_RecordResponse(uint8_t *Data, uint32_t Len)
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
	WriteRBufferForce(&Cache.ResBuf, (uint8_t *)&MonitorRes, 1);
	Monitor_Wakeup();
#ifdef MONITOR_CACHE_DEBUG
	if (Cache.ResBuf.Len > 1)
	{
		DBG("Res cache len %u", Cache.ResBuf.Len);
	}
#endif
}

uint8_t Monitor_ExtractData(Monitor_RecordStruct *Data)
{
	uint8_t Len = 0;
	Monitor_DataStruct MonitorData;
	while (Cache.DataBuf.Len)
	{
		QueryRBuffer(&Cache.DataBuf, (uint8_t *)&MonitorData, 1);
		if (MonitorData.CRC32 != __CRC32((uint8_t *)&MonitorData.uRecord.Data, sizeof(Monitor_RecordStruct), CRC32_START))
		{
			DBG("%u data error!", Cache.DataBuf.Offset);
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
		DBG("Data cache len %u", Cache.DataBuf.Len);
	}
#endif
	return Len;
}

uint8_t Monitor_ExtractAlarm(Monitor_RecordStruct *Alarm)
{
	uint8_t Len = 0;
	Monitor_DataStruct MonitorData;
	while (Cache.AlarmBuf.Len)
	{
		QueryRBuffer(&Cache.AlarmBuf, (uint8_t *)&MonitorData, 1);
		if (MonitorData.CRC32 != __CRC32((uint8_t *)&MonitorData.uRecord.Data, sizeof(Monitor_RecordStruct), CRC32_START))
		{
			DBG("%u alarm error!", Cache.AlarmBuf.Offset);
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
		DBG("Alarm cache len %u", Cache.AlarmBuf.Len);
	}
#endif
	return Len;
}

uint32_t Monitor_ExtractResponse(uint8_t *Response)
{
	uint32_t Len = 0;
	Monitor_ResponseStruct MonitorRes;
	while (Cache.ResBuf.Len)
	{
		QueryRBuffer(&Cache.ResBuf, (uint8_t *)&MonitorRes, 1);
		if (MonitorRes.Len > 1024)
		{
			DBG("%u response len error!", Cache.ResBuf.Offset);
			DelRBuffer(&Cache.ResBuf, 1);
			continue;
		}
		if (MonitorRes.CRC32 != __CRC32(&MonitorRes.Data[0], MonitorRes.Len, CRC32_START))
		{
			DBG("%u response error!", Cache.ResBuf.Offset);
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
		DBG("Res cache len %u", Cache.ResBuf.Len);
	}
#endif
	return Len;
}

void Monitor_DelCache(uint8_t Type, uint8_t IsAll)
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
			DBG("Res cache len %u", Cache.ResBuf.Len);
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
			DBG("Alarm cache len %u", Cache.AlarmBuf.Len);
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
			DBG("Data cache len %u", Cache.DataBuf.Len);
		}
#endif
		break;
	}
}

uint32_t Monitor_GetCacheLen(uint8_t Type)
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
	gSys.RecordCollect.IsWork = 1;
	//gSys.RecordCollect.RunStartTime = gSys.Var[SYS_TIME] + MONITOR_RUN_TIME;
	OS_SendEvent(gSys.TaskID[MONITOR_TASK_ID], EV_MMI_MONITOR_WAKEUP, 0, 0, 0);
}

void Monitor_Heart(void)
{
	gSys.RecordCollect.HeartStartTime = gSys.Var[SYS_TIME] + gSys.RecordCollect.Param[PARAM_UPLOAD_HEART_PERIOD];
	OS_SendEvent(gSys.TaskID[MONITOR_TASK_ID], EV_MMI_MONITOR_HEART, 0, 0, 0);
}

void Monitor_Upload(void)
{
	OS_SendEvent(gSys.TaskID[MONITOR_TASK_ID], EV_MMI_MONITOR_UPLOAD, 0, 0, 0);
}

void Monitor_StateCheck(void)
{
	IO_ValueUnion IO;

	if (gSys.RecordCollect.Param[PARAM_GS_WAKEUP_MONITOR])
	{
		if (gSys.Var[GSENSOR_ALARM_VAL] >= G_POWER(gSys.RecordCollect.Param[PARAM_GS_WAKEUP_MONITOR]))
		{
			gSys.RecordCollect.WakeupFlag = 1;
		}
	}
	else
	{
		IO.Val = gSys.Var[IO_VAL];
		if (IO.IOVal.VACC)
		{
			gSys.RecordCollect.WakeupFlag = 1;
		}
	}



	if (gSys.RecordCollect.IsWork) //监控处于工作状态时
	{
		if (gSys.RecordCollect.Param[PARAM_GS_JUDGE_RUN])
		{
			if (gSys.Var[GSENSOR_ALARM_VAL] >= G_POWER(gSys.RecordCollect.Param[PARAM_GS_JUDGE_RUN]))
			{
				gSys.RecordCollect.RunStartTime = gSys.Var[SYS_TIME] + MONITOR_RUN_TIME;
			}
		}
		else
		{
			gSys.RecordCollect.RunStartTime = gSys.Var[SYS_TIME] + MONITOR_RUN_TIME;
		}

		if (gSys.Var[SYS_TIME] > gSys.RecordCollect.RunStartTime)
		{
			if (gSys.RecordCollect.IsRunMode)
			{
				gSys.RecordCollect.IsRunMode = 0;
				DBG("Entry car stop mode! %u %u", gSys.Var[SYS_TIME], gSys.RecordCollect.RunStartTime);
#ifdef __COM_SLEEP_BY_STOP__
				COM_SleepReq(1);
#endif
			}
		}
		else
		{
			if (!gSys.RecordCollect.IsRunMode)
			{
				gSys.RecordCollect.IsRunMode = 1;
				DBG("Entry car run mode %u!", gSys.RecordCollect.RecordStartTime);
				if (gSys.RecordCollect.RecordStartTime > gSys.RecordCollect.Param[PARAM_UPLOAD_STOP_PERIOD])
				{
					gSys.RecordCollect.RecordStartTime = gSys.Var[SYS_TIME];
				}
				else
				{
					gSys.RecordCollect.RecordStartTime = gSys.Var[SYS_TIME] + gSys.RecordCollect.Param[PARAM_UPLOAD_RUN_PERIOD];
				}
#ifdef __COM_SLEEP_BY_STOP__
				COM_SleepReq(0);
#endif
			}
		}

		if (gSys.Var[SYS_TIME] >= gSys.RecordCollect.RecordStartTime)
		{
			Monitor_RecordData();
			if (gSys.RecordCollect.IsRunMode)
			{
				gSys.RecordCollect.RecordStartTime = gSys.Var[SYS_TIME] + gSys.RecordCollect.Param[PARAM_UPLOAD_RUN_PERIOD];
			}
			else
			{
				gSys.RecordCollect.RecordStartTime = gSys.Var[SYS_TIME] + gSys.RecordCollect.Param[PARAM_UPLOAD_STOP_PERIOD];
			}
			Monitor_Upload();
		}
		else
		{
#ifdef __LBS_AUTO__
			if (gSys.RecordCollect.IsRunMode && gSys.Error[NO_LOCAT_ERROR])
			{
				if (gSys.RecordCollect.RecordStartTime == (gSys.Var[SYS_TIME] + 3))
				{
					LUAT_StartLBS(0);
				}
			}
#endif
		}

		if (gSys.Var[SYS_TIME] >= gSys.RecordCollect.HeartStartTime)
		{
			Monitor_Heart();

		}
	}
	else
	{
		gSys.RecordCollect.RunStartTime = gSys.Var[SYS_TIME] + MONITOR_RUN_TIME;
		if (gSys.RecordCollect.WakeupFlag)
		{
			//发出唤醒事件
			Monitor_Wakeup();
		}
	}
}
