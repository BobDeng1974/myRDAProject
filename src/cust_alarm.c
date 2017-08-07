#include "user.h"

typedef struct
{
	double MoveOrgLat;
	double MoveOrgLgt;
	uint32_t AlarmWaitTime;
	uint32_t CrashWaitTime;
	uint32_t MoveWaitTime;
	uint32_t OverspeedWaitTime;
	uint32_t CutlineWaitTime;
	uint32_t CrashCheckTimes;	//碰撞检测次数
	uint32_t MoveTimes;			//移动超出距离第N次
	uint32_t *Param1;
	uint32_t *Param2;
	uint8_t FindOrg;
	uint8_t LastACC;
}Alarm_CtrlStruct;

Alarm_CtrlStruct __attribute__((section (".usr_ram"))) AlarmCtrl;
void Alarm_CrashCheck(void);
void Alarm_CutlineCheck(uint8_t ACC, uint8_t VCC);
void Alarm_MoveCheck(void);
void Alarm_OverspeedCheck(void);

void Alarm_Config(void)
{

	AlarmCtrl.Param1 = gSys.nParam[PARAM_TYPE_ALARM1].Data.ParamDW.Param;
	AlarmCtrl.Param2 = gSys.nParam[PARAM_TYPE_ALARM2].Data.ParamDW.Param;

	AlarmCtrl.AlarmWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param1[PARAM_ALARM_ON_DELAY];
#if (__CUST_CODE__ == __CUST_LY__)
	AlarmCtrl.LastACC = 2;
#else
	AlarmCtrl.LastACC = GPIO_Read(ACC_DET_PIN);
#endif
}

void Alarm_StateCheck(void)
{
	IO_ValueUnion uIO;
	uIO.Val = gSys.Var[IO_VAL];
	Alarm_CutlineCheck(uIO.IOVal.ACC, uIO.IOVal.VCC);

	if (!AlarmCtrl.Param2[PARAM_ALARM_ENABLE])
	{
		gSys.State[CRASH_STATE] = ALARM_STATE_DISABLE;
		gSys.State[MOVE_STATE] = ALARM_STATE_DISABLE;
		gSys.State[ALARM_STATE] = 0;
		AlarmCtrl.MoveTimes = 0;
		AlarmCtrl.CrashCheckTimes = 0;
		AlarmCtrl.AlarmWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param1[PARAM_ALARM_ON_DELAY];
		goto ALARM_CHECK_END;
	}

	if (AlarmCtrl.Param1[PARAM_VACC_CTRL_ALARM])
	{
		if (uIO.IOVal.VACC)
		{
			gSys.State[ALARM_STATE] = 0;
			AlarmCtrl.AlarmWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param1[PARAM_ALARM_ON_DELAY];
			if (gSys.State[CRASH_STATE] != ALARM_STATE_DISABLE)
			{
				DBG("Crash check stop");
				gSys.State[CRASH_STATE] = ALARM_STATE_DISABLE;
			}

			if (gSys.State[MOVE_STATE] != ALARM_STATE_DISABLE)
			{
				DBG("Move check stop");
				gSys.State[MOVE_STATE] = ALARM_STATE_DISABLE;
				AlarmCtrl.MoveTimes = 0;

			}
		}
		else if (gSys.Var[SYS_TIME] >= AlarmCtrl.AlarmWaitTime)
		{
			gSys.State[ALARM_STATE] = 1;
			if (gSys.State[CRASH_STATE] == ALARM_STATE_DISABLE)
			{
				DBG("Crash check start");
				gSys.State[CRASH_STATE] = ALARM_STATE_IDLE;
			}

			if (gSys.State[MOVE_STATE] == ALARM_STATE_DISABLE)
			{
				DBG("Move check start");
				gSys.State[MOVE_STATE] = ALARM_STATE_IDLE;
				AlarmCtrl.MoveTimes = 0;
			}

		}
	}
	else
	{
		gSys.State[ALARM_STATE] = 1;
		if (gSys.State[CRASH_STATE] == ALARM_STATE_DISABLE)
		{
			DBG("Crash check start");
			gSys.State[CRASH_STATE] = ALARM_STATE_IDLE;
		}

		if (gSys.State[MOVE_STATE] == ALARM_STATE_DISABLE)
		{
			DBG("Move check start");
			gSys.State[MOVE_STATE] = ALARM_STATE_IDLE;
		}
	}
	Alarm_CrashCheck();
	Alarm_MoveCheck();
	Alarm_OverspeedCheck();
	gSys.Var[GSENSOR_ALARM_VAL] = 0;
ALARM_CHECK_END:
	if (gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param[PARAM_MONITOR_ACC_UPLOAD])
	{
		if (AlarmCtrl.LastACC != uIO.IOVal.ACC)
		{
			AlarmCtrl.LastACC = uIO.IOVal.ACC;
			if (AlarmCtrl.LastACC)
			{
				Monitor_RecordAlarm(ALARM_TYPE_ACC_ON, 0, 0);
			}
			else
			{
				Monitor_RecordAlarm(ALARM_TYPE_ACC_OFF, 0, 0);
			}
		}
	}
}

void Alarm_CutlineCheck(uint8_t ACC, uint8_t VCC)
{
	switch (gSys.State[CUTLINE_STATE]) {
	case ALARM_STATE_DISABLE:
		/*电源恢复时则允许剪线报警再次检测*/
		if (VCC)
		{
			gSys.State[CUTLINE_STATE] = ALARM_STATE_IDLE;
			AlarmCtrl.CutlineWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param2[PARAM_VACC_WAKEUP_CUTLINE_TO];
			DBG("Cutline check recovery!");
		}
		break;

	case ALARM_STATE_IDLE:
		/*剪线报警准备时，如果ACC先关闭了，则需要等待一段时间，避免用户主动拔电瓶误报*/
		if (gSys.Var[SYS_TIME] >= AlarmCtrl.CutlineWaitTime)
		{
			DBG("Cutline check start!");
			gSys.State[CUTLINE_STATE] = ALARM_STATE_CHECK;
		}

		if (ACC)
		{
			AlarmCtrl.CutlineWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param2[PARAM_VACC_WAKEUP_CUTLINE_TO] + 1;
		}
		else
		{
			if (!VCC)
			{
				DBG("Cutline before acc off");
				gSys.State[CUTLINE_STATE] = ALARM_STATE_DISABLE;
			}
		}
		break;

	case ALARM_STATE_CHECK:
		if (!VCC)
		{
			gSys.State[CUTLINE_STATE] = ALARM_STATE_READY;
			AlarmCtrl.CutlineWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param2[PARAM_CUTLINE_ALARM_DELAY];
			DBG("Cutline detect");
		}
		if ( AlarmCtrl.Param2[PARAM_VACC_WAKEUP_CUTLINE_TO] && ACC )
		{
			gSys.State[CUTLINE_STATE] = ALARM_STATE_IDLE;
			AlarmCtrl.CutlineWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param2[PARAM_VACC_WAKEUP_CUTLINE_TO] + 1;
		}
		break;

	case ALARM_STATE_READY:
		if (gSys.Var[SYS_TIME] >= AlarmCtrl.CutlineWaitTime)
		{
			gSys.State[CUTLINE_STATE] = ALARM_STATE_UPLOAD;
		}
		else if (VCC)
		{
			DBG("Cutline check recovery!");
			gSys.State[CUTLINE_STATE] = ALARM_STATE_CHECK;
		}
		break;

	case ALARM_STATE_UPLOAD:
		Monitor_RecordAlarm(ALARM_TYPE_CUTLINE, 0, 0);
		if (AlarmCtrl.Param2[PARAM_CUTLINE_ALARM_FLUSH_TO])
		{
			AlarmCtrl.CutlineWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param2[PARAM_CUTLINE_ALARM_FLUSH_TO];
			gSys.State[CUTLINE_STATE] = ALARM_STATE_WAIT_FLUSH;
		}
		else
		{
			gSys.State[CUTLINE_STATE] = ALARM_STATE_DISABLE;
		}
		break;

	case ALARM_STATE_WAIT_FLUSH:
		if ( (gSys.Var[SYS_TIME] >= AlarmCtrl.CutlineWaitTime) || VCC)
		{
			gSys.State[CUTLINE_STATE] = ALARM_STATE_CHECK;
		}
		break;
	default:
		gSys.State[CUTLINE_STATE] = ALARM_STATE_DISABLE;
		break;
	}
}

void Alarm_CrashCheck(void)
{
	if (!AlarmCtrl.Param1[PARAM_CRASH_GS] && !AlarmCtrl.Param1[PARAM_CRASH_JUDGE_CNT])
		return ;
	switch (gSys.State[CRASH_STATE]) {
	case ALARM_STATE_DISABLE:
		break;

	case ALARM_STATE_IDLE:
		/*检测到第一次大幅度震动后，开启一段时间内震动次数检测*/
		if (gSys.Var[GSENSOR_ALARM_VAL] >= G_POWER(AlarmCtrl.Param1[PARAM_CRASH_GS]))
		{
			gSys.State[CRASH_STATE] = ALARM_STATE_CHECK;
			AlarmCtrl.CrashCheckTimes = 1;
			AlarmCtrl.CrashWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param1[PARAM_CRASH_JUDGE_TO];
			DBG("detect fisrt crash %u %u", gSys.Var[GSENSOR_ALARM_VAL], gSys.Var[SYS_TIME]);
		}
		break;

	case ALARM_STATE_CHECK:
		if (gSys.Var[SYS_TIME] >= AlarmCtrl.CrashWaitTime)
		{
			DBG("in %usec crash %utimes, quit!", gSys.Var[SYS_TIME], AlarmCtrl.CrashCheckTimes);
			AlarmCtrl.CrashCheckTimes = 0;
			gSys.State[CRASH_STATE] = ALARM_STATE_IDLE;
		}

		if (gSys.Var[GSENSOR_ALARM_VAL] >= G_POWER(AlarmCtrl.Param1[PARAM_CRASH_GS]))
		{
			AlarmCtrl.CrashCheckTimes++;
			DBG("detect crash %u %u", gSys.Var[GSENSOR_ALARM_VAL], AlarmCtrl.CrashCheckTimes);
		}

		if (AlarmCtrl.CrashCheckTimes >= AlarmCtrl.Param1[PARAM_CRASH_JUDGE_CNT])
		{

			DBG("in %usec crash %utimes, confirm!", gSys.Var[SYS_TIME], AlarmCtrl.CrashCheckTimes);
			AlarmCtrl.CrashCheckTimes = 0;
			if (AlarmCtrl.Param1[PARAM_CRASH_ALARM_WAIT_TO])
			{
				gSys.State[CRASH_STATE] = ALARM_STATE_UPLOAD;
				AlarmCtrl.CrashWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param1[PARAM_CRASH_ALARM_WAIT_TO];
			}
			else
			{
				Monitor_RecordAlarm(ALARM_TYPE_CRASH, AlarmCtrl.Param1[PARAM_CRASH_JUDGE_CNT], 0);
				AlarmCtrl.CrashWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param1[PARAM_CRASH_ALARM_FLUSH_TO];
				gSys.State[CRASH_STATE] = ALARM_STATE_WAIT_FLUSH;
			}

		}
		break;

	case ALARM_STATE_UPLOAD:
		if (gSys.Var[SYS_TIME] >= AlarmCtrl.CrashWaitTime)
		{
			Monitor_RecordAlarm(ALARM_TYPE_CRASH, AlarmCtrl.Param1[PARAM_CRASH_JUDGE_CNT], 0);
			AlarmCtrl.CrashWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param1[PARAM_CRASH_ALARM_FLUSH_TO];
			gSys.State[CRASH_STATE] = ALARM_STATE_WAIT_FLUSH;
		}
		break;

	case ALARM_STATE_WAIT_FLUSH:
		if (AlarmCtrl.Param1[PARAM_CRASH_ALARM_REPEAT])
		{
			if (gSys.Var[SYS_TIME] >= AlarmCtrl.CrashWaitTime)
			{

				DBG("crash wait next!");
				gSys.State[CRASH_STATE] = ALARM_STATE_IDLE;
			}
		}
		else
		{
			if (gSys.Var[GSENSOR_ALARM_VAL] >= G_POWER(AlarmCtrl.Param1[PARAM_CRASH_GS]))
			{
				AlarmCtrl.CrashWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param1[PARAM_CRASH_ALARM_FLUSH_TO];
			}
			if (gSys.Var[SYS_TIME] >= AlarmCtrl.CrashWaitTime)
			{
				DBG("crash wait recovery!");
				gSys.State[CRASH_STATE] = ALARM_STATE_IDLE;
			}

		}
		break;

	default:
		gSys.State[CRASH_STATE] = ALARM_STATE_DISABLE;
		break;
	}

}

void Alarm_MoveCheck(void)
{
	uint32_t Range;
	double Lat;
	double Lgt;
	double R;
	if (!AlarmCtrl.Param1[PARAM_MOVE_RANGE])
		return ;

	if ((gSys.State[MOVE_STATE] != ALARM_STATE_DISABLE) && !AlarmCtrl.FindOrg)
	{
		if ((GPS_A_STAGE == gSys.State[GPS_STATE]) && (gSys.Var[GSENSOR_ALARM_VAL] >= 100))
		{
			AlarmCtrl.FindOrg = 1;
			AlarmCtrl.MoveOrgLat = (gSys.RMCInfo->LatDegree - 0) * 1000000 + gSys.RMCInfo->LatMin * 100 / 60;
			AlarmCtrl.MoveOrgLat = AlarmCtrl.MoveOrgLat/1000000;
			AlarmCtrl.MoveOrgLgt = (gSys.RMCInfo->LgtDegree - 0) * 1000000 + gSys.RMCInfo->LgtMin * 100 / 60;
			AlarmCtrl.MoveOrgLgt = AlarmCtrl.MoveOrgLgt/1000000;
		}
	}
	switch (gSys.State[MOVE_STATE]) {
	case ALARM_STATE_DISABLE:
		AlarmCtrl.FindOrg = 0;
		break;

	case ALARM_STATE_IDLE:
		AlarmCtrl.FindOrg = 0;
		/*震动触发位移检测使能时，当震动报警状态处于检测状态时，位移检测开始*/
		if (AlarmCtrl.Param1[PARAM_CRASH_WAKEUP_MOVE])
		{
			if (gSys.Var[GSENSOR_ALARM_VAL] >= G_POWER(AlarmCtrl.Param1[PARAM_CRASH_GS]))
			{
				gSys.State[MOVE_STATE] = ALARM_STATE_CHECK;
				AlarmCtrl.MoveWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param1[PARAM_MOVE_JUDGE_TO];
				DBG("entry move check");
			}
		}
		else
		{
			gSys.State[MOVE_STATE] = ALARM_STATE_CHECK;
			AlarmCtrl.MoveWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param1[PARAM_MOVE_JUDGE_TO];
			DBG("entry move check");
		}

		break;

	case ALARM_STATE_CHECK:
		/*检测到有效点后，确定位移检测原点*/

		if ( (GPS_A_STAGE == gSys.State[GPS_STATE]) &&
				(gSys.Var[GSENSOR_ALARM_VAL] >= 100) &&
				AlarmCtrl.FindOrg)
		{
			Lat = gSys.RMCInfo->LatDegree * 1000000 + gSys.RMCInfo->LatMin * 100 / 60;
			Lat = Lat/1000000;
			Lgt = gSys.RMCInfo->LgtDegree * 1000000 + gSys.RMCInfo->LgtMin * 100 / 60;
			Lgt = Lgt/1000000;
			R = GPS_Distance(Lat, AlarmCtrl.MoveOrgLat, Lgt, AlarmCtrl.MoveOrgLgt);
			Range = R;
			if (Range >= (AlarmCtrl.Param1[PARAM_MOVE_RANGE] * (AlarmCtrl.MoveTimes + 1)))
			{
				DBG("detect range %u %u %u %u %u %u", (uint32_t)(AlarmCtrl.MoveOrgLat * 1000000), (uint32_t)(AlarmCtrl.MoveOrgLgt * 1000000),
						(uint32_t)(Lat * 1000000), (uint32_t)(Lgt * 1000000),
						Range, AlarmCtrl.MoveTimes + 1);
				AlarmCtrl.MoveTimes++;
				Monitor_RecordAlarm(ALARM_TYPE_MOVE, 0, AlarmCtrl.MoveTimes);
				AlarmCtrl.MoveWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param1[PARAM_MOVE_ALARM_FLUSH_TO];
				gSys.State[MOVE_STATE] = ALARM_STATE_WAIT_FLUSH;
			}

		}

		if (AlarmCtrl.Param1[PARAM_MOVE_JUDGE_TO])
		{
			if ( gSys.Var[SYS_TIME] >= AlarmCtrl.MoveWaitTime)
			{
				DBG("move alarm timeout, quit!\r\n");
				gSys.State[MOVE_STATE] = ALARM_STATE_IDLE;
			}
		}
		break;

	case ALARM_STATE_WAIT_FLUSH:
		if (AlarmCtrl.Param1[PARAM_MOVE_ALARM_REPEAT])
		{
			if (AlarmCtrl.Param1[PARAM_MOVE_ALARM_FLUSH_TO])
			{
				if (gSys.Var[SYS_TIME] >= AlarmCtrl.MoveWaitTime)
				{
					DBG("move alarm wait next");
					//重新选点，车卫士
					gSys.State[MOVE_STATE] = ALARM_STATE_IDLE;
				}
			}
			else
			{
				DBG("move alarm wait next");
				//不重新选点，绿源
				gSys.State[MOVE_STATE] = ALARM_STATE_CHECK;
			}
		}
		else
		{
			if (gSys.Var[GSENSOR_ALARM_VAL] >= G_POWER(AlarmCtrl.Param1[PARAM_CRASH_GS]))
			{
				AlarmCtrl.MoveWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param1[PARAM_MOVE_ALARM_FLUSH_TO];
			}
			if (gSys.Var[SYS_TIME] >= AlarmCtrl.CrashWaitTime)
			{
				DBG("move wait recovery!");
				gSys.State[MOVE_STATE] = ALARM_STATE_IDLE;
			}
		}
		break;

	default:
		gSys.State[MOVE_STATE] = ALARM_STATE_DISABLE;
		break;
	}
}

void Alarm_OverspeedCheck(void)
{
	uint32_t Speed;
	if (!AlarmCtrl.Param2[PARAM_OVERSPEED_ALARM_VAL])
	{
		return ;
	}

	if (gSys.State[GPS_STATE] != GPS_A_STAGE)
	{
		gSys.State[OVERSPEED_STATE] = ALARM_STATE_DISABLE;
		return ;
	}

	switch (gSys.State[OVERSPEED_STATE])
	{
	case ALARM_STATE_DISABLE:
		Speed = gSys.RMCInfo->Speed * 1852 / 1000000;
		if (Speed > AlarmCtrl.Param2[PARAM_OVERSPEED_ALARM_VAL])
		{
			gSys.State[OVERSPEED_STATE] = ALARM_STATE_IDLE;
			AlarmCtrl.OverspeedWaitTime = gSys.Var[SYS_TIME] + AlarmCtrl.Param2[PARAM_OVERSPEED_ALARM_DELAY];
		}
		break;
	case ALARM_STATE_IDLE:
		if (gSys.Var[SYS_TIME] >= AlarmCtrl.OverspeedWaitTime)
		{
			Monitor_RecordAlarm(ALARM_TYPE_OVERSPEED, 0, 0);
			gSys.State[OVERSPEED_STATE] = ALARM_STATE_WAIT_FLUSH;
		}
		else
		{
			Speed = gSys.RMCInfo->Speed * 1852 / 1000000;
			if (Speed <= AlarmCtrl.Param2[PARAM_OVERSPEED_ALARM_VAL])
			{
				gSys.State[OVERSPEED_STATE] = ALARM_STATE_DISABLE;
			}
		}
		break;

	case ALARM_STATE_WAIT_FLUSH:
		Speed = gSys.RMCInfo->Speed * 1852 / 1000000;
		if (Speed <= AlarmCtrl.Param2[PARAM_OVERSPEED_ALARM_VAL])
		{
			gSys.State[OVERSPEED_STATE] = ALARM_STATE_DISABLE;
		}
		break;

	default:
		gSys.State[OVERSPEED_STATE] = ALARM_STATE_DISABLE;
		break;
	}
}
