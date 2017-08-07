#ifndef __LUAT_H__
#define __LUAT_H__

typedef struct
{
	uint32_t Lat;
	uint32_t lgt;
	uint32_t LocationLen;
	int8_t Location[512];
}LBS_Struct;

void LUAT_Config(void);
void LUAT_StartLBS(uint8_t IsRepeat);
LBS_Struct *LUAT_GetLBS(void);
#endif
