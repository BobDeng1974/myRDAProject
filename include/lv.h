#ifndef __CUST_LV_H__
#define __CUST_LV_H__

typedef struct
{
	int8_t *Cmd;
	int8_t *DataIn;
	int8_t *DataOut;
	int8_t CmdLen;
	uint8_t Sn;
	int8_t pad[2];
	int32_t Result;
}LV_AnalyzeStruct;
void LV_Print(uint8_t *Buf);
uint8_t LV_CheckHead(uint8_t Data);
uint8_t LV_Receive(COM_CtrlStruct *COM, uint8_t Data);
int32_t LV_ComAnalyze(COM_CtrlStruct *COM);
void LV_SMSAnalyze(uint8_t *InBuf, uint32_t InLen, uint8_t *OutBuf, uint32_t *OutLen);
#endif
