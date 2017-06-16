#ifndef __CUST_ALARM_H__
#define __CUST_ALARM_H__

enum ALARM_STATE_ENUM
{
	ALARM_STATE_DISABLE,				//报警无效阶段
	ALARM_STATE_IDLE,					//报警准备
	ALARM_STATE_CHECK,					//报警检测
	ALARM_STATE_READY,					//报警准备上传
	ALARM_STATE_UPLOAD,					//报警上传
	ALARM_STATE_WAIT_FLUSH,				//报警等待刷新
};

void Alarm_Config(void);
void Alarm_StateCheck(void);
#endif
