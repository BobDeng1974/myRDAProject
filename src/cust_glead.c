#include "user.h"
#if (__CUST_CODE__ == __CUST_GLEAD__)
Monitor_CtrlStruct __attribute__((section (".usr_ram"))) GleadCtrl;
void GL_Task(void *pData)
{
	IP_AddrUnion IPUnion;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	COS_EVENT Event = { 0 };
	DBG("Task start! %d %d %d %d %d %d %d %d %d" ,
			GleadCtrl.Param[PARAM_GS_WAKEUP_MONITOR], GleadCtrl.Param[PARAM_GS_JUDGE_RUN],
			GleadCtrl.Param[PARAM_UPLOAD_RUN_PERIOD], GleadCtrl.Param[PARAM_UPLOAD_STOP_PERIOD],
			GleadCtrl.Param[PARAM_UPLOAD_HEART_PERIOD], GleadCtrl.Param[PARAM_MONITOR_NET_TO],
			GleadCtrl.Param[PARAM_MONITOR_KEEP_TO], GleadCtrl.Param[PARAM_MONITOR_SLEEP_TO],
			GleadCtrl.Param[PARAM_MONITOR_RECONNECT_MAX]);
    memset(&Event, 0, sizeof(COS_EVENT));
    sprintf(GleadCtrl.MonitorID.ucID, "%02d%09d", (int)MainInfo->UID[1], (int)MainInfo->UID[0]);
    DBG("monitor id %s", GleadCtrl.MonitorID.ucID);
    while (1)
    {
    	COS_WaitEvent(gSys.TaskID[MONITOR_TASK_ID], &Event, COS_WAIT_FOREVER);
    }

}

void GL_Config(void)
{
	gSys.TaskID[MONITOR_TASK_ID] = COS_CreateTask(GL_Task, NULL,
				NULL, MMI_TASK_MAX_STACK_SIZE, MMI_TASK_PRIORITY + MONITOR_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI GL Task");
	GleadCtrl.Param = gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param;

	GleadCtrl.Net.SocketID = INVALID_SOCKET;
	GleadCtrl.Net.TaskID = gSys.TaskID[MONITOR_TASK_ID];
	GleadCtrl.Net.Channel = GPRS_CH_MAIN_MONITOR;
	GleadCtrl.Net.TimerID = MONITOR_TIMER_ID;
	GleadCtrl.RxState = LB_PRO_FIND_HEAD1;
	GleadCtrl.Net.ReceiveFun = NULL;

	gSys.Monitor = &GleadCtrl;
	if (!GleadCtrl.Param[PARAM_UPLOAD_RUN_PERIOD])
	{
		GleadCtrl.Param[PARAM_UPLOAD_RUN_PERIOD] = 30;
	}
}
#endif
