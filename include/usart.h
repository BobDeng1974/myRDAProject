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
	u8 RxBuf[COM_BUF_LEN];
	u8 AnalyzeBuf[COM_BUF_LEN];
	u32 RxPos;//接收长度
	u32 NeedRxLen;//剩余接收长度，最长COM_BUF_LEN
	u32 AnalyzeLen;
	u32 To;
	u32 CurrentBR;
	u8 TxBuf[COM_BUF_LEN * 2];
	u8 DMABuf[COM_BUF_LEN];
	u8 TempBuf[COM_BUF_LEN];
	u8 ProtocolType;//协议类型
	u8 TxBusy;
	u8 TxCmd;
	u8 SleepFlag;
	u8 LockFlag;
	u8 Mode485Tx;
	u8 Mode485TxDone;
}COM_CtrlStruct;

void Uart_Config(void);
void COM_TxReq(u8 *Data, u32 Len);
void COM_Tx(u8 *Data, u32 Len);
u8 COM_Send(u8 *Data, u32 Len);
void COM_Sleep(void);
void COM_Wakeup(u32 BR);
#endif
