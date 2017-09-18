#ifndef __CUST_USART_H__
#define __CUST_USART_H__
#define COM_BUF_LEN	(2048)
enum
{
	COM_RES_NONE,
	COM_RES_TO,
	COM_RES_TX_REQ,
	COM_RES_RX,

	COM_PROTOCOL_NONE = 0,
	COM_PROTOCOL_LV,
	COM_PROTOCOL_DEV,
	COM_PROTOCOL_USP,
};

typedef struct
{
	RBuffer rTxBuf;
	uint8_t RxBuf[COM_BUF_LEN + 32];
	uint8_t AnalyzeBuf[COM_BUF_LEN + 32];
	uint32_t RxPos;//接收长度
	uint32_t NeedRxLen;//剩余接收长度，最长COM_BUF_LEN
	uint32_t AnalyzeLen;
	uint32_t LastRxDMALen;//上一次超时读到的长度
	uint32_t CurrentBR;
	uint8_t TxBuf[COM_BUF_LEN * 2];
	uint8_t DMABuf[COM_BUF_LEN];
	uint8_t TempBuf[COM_BUF_LEN];
	uint8_t ProtocolType;//协议类型
	uint8_t TxBusy;
	uint8_t TxCmd;
	uint8_t SleepFlag;
	uint8_t SleepWaitCnt;
	uint8_t SleepReq;
	uint8_t Mode485Tx;
	uint8_t Mode485TxDone;
	uint8_t RxDMAChannel;
}COM_CtrlStruct;

void Uart_Config(void);
void COM_TxReq(uint8_t *Data, uint32_t Len);
void COM_Tx(uint8_t *Data, uint32_t Len);
uint8_t COM_Send(uint8_t *Data, uint32_t Len);
void COM_Sleep(void);
void COM_Wakeup(uint32_t BR);
uint8_t COM_SleepReq(uint8_t);
void COM_StateCheck(void);
#endif
