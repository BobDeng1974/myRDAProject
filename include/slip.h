#ifndef __CUST_SLIP_H__
#define __CUST_SLIP_H__
typedef struct
{
	u8 Type;
	u8 Cmd;
	u16 Len;
	u8 From;
	u8 To;
	u16 CRC16;
}SLIP_HeadStruct;

typedef struct
{
	u8 *InBuf;
	u8 *OutBuf;
	u32 InLen;
	u32 OutLen;
	u8 Cmd;
}SLIP_AnalyzeStruct;

u8 SLIP_CheckHead(u8 Data);
u8 SLIP_Receive(COM_CtrlStruct *COM, u8 Data);
u32 SLIP_Analyze(u8 *InBuf, u32 Len, u8 *OutBuf);
void SLIP_SetHead(SLIP_AnalyzeStruct *SLIP);
#endif
