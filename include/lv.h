#ifndef __CUST_LV_H__
#define __CUST_LV_H__

typedef struct
{
	s8 *Cmd;
	s8 *DataIn;
	s8 *DataOut;
	s8 CmdLen;
	u8 Sn;
	s8 pad[2];
	s32 Result;
}LV_AnalyzeStruct;
void LV_Print(u8 *Buf);
u8 LV_CheckHead(u8 Data);
u8 LV_Receive(COM_CtrlStruct *COM, u8 Data);
s32 LV_ComAnalyze(COM_CtrlStruct *COM);
void LV_SMSAnalyze(u8 *InBuf, u32 InLen, u8 *OutBuf, u32 *OutLen);
#endif
