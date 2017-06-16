#ifndef __CUST_ALARM_H__
#define __CUST_ALARM_H__

enum ALARM_STATE_ENUM
{
	ALARM_STATE_DISABLE,				//������Ч�׶�
	ALARM_STATE_IDLE,					//����׼��
	ALARM_STATE_CHECK,					//�������
	ALARM_STATE_READY,					//����׼���ϴ�
	ALARM_STATE_UPLOAD,					//�����ϴ�
	ALARM_STATE_WAIT_FLUSH,				//�����ȴ�ˢ��
};

void Alarm_Config(void);
void Alarm_StateCheck(void);
#endif
