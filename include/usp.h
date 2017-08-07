#ifndef __CUST_USP_H__
#define __CUST_USP_H__
typedef struct
{
	uint16_t MagicNum;
	uint16_t DataSize;
	uint16_t Cmd;
	uint16_t CRC16;
	uint8_t Qos;
	uint8_t Xor;

}USP_HeadStruct;

typedef struct
{
	uint8_t *InBuf;
	uint8_t *OutBuf;
	uint32_t InLen;
	uint32_t OutLen;
	uint8_t Qos;
}USP_AnalyzeStruct;

uint32_t USP_CheckHead(uint8_t Data);
uint32_t USP_CheckLen(uint8_t *Data);
uint32_t USP_Analyze(uint8_t *InBuf, uint32_t Len, uint8_t *OutBuf);
void USP_SetHead(USP_AnalyzeStruct *USP, uint16_t Cmd, uint8_t Qos);
#endif
