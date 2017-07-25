#include "user.h"

void Led_Flush(u8 Led, u8 NewType)
{
//	DBG("%d %d", Led, NewType);
	switch (NewType)
	{
	case LED_OFF:
		OS_StopTimer(gSys.TaskID[MAIN_TASK_ID], LED_TIMER_ID + Led);
		GPIO_Write(LED_NET_PIN + Led, 0);
		gSys.State[LED_STATE + Led] = 0;
		break;
	case LED_ON:
		OS_StopTimer(gSys.TaskID[MAIN_TASK_ID], LED_TIMER_ID + Led);
		GPIO_Write(LED_NET_PIN + Led, 1);
		gSys.State[LED_STATE + Led] = 1;
		break;
	case LED_FLUSH_SLOW:
		OS_StartTimer(gSys.TaskID[MAIN_TASK_ID], LED_TIMER_ID + Led, COS_TIMER_MODE_PERIODIC, SYS_TICK/2);

		break;
	case LED_FLUSH_FAST:
		OS_StartTimer(gSys.TaskID[MAIN_TASK_ID], LED_TIMER_ID + Led, COS_TIMER_MODE_PERIODIC, SYS_TICK/16);
		break;
	}
}
