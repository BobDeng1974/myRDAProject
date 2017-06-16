#ifndef __CUST_USART_H__
#define __CUST_USART_H__
#define COM_BUF_LEN	(2048)
#define SLIP_ENTRY_FLAG (0x55)
enum
{
	COM_RES_NONE,
	COM_RES_TO,
	COM_RES_TX_REQ,
	COM_RES_RX,

	COM_MODE_IDLE = 0,
	COM_MODE_DEV,
	COM_MODE_TEST,
	COM_MODE_SLIP,
};

typedef struct
{
	RBuffer rTxBuf;
	u8 RxBuf[COM_BUF_LEN];
	u8 AnalyzeBuf[COM_BUF_LEN];
	u32 RxPos;//接收长度
	u32 AnalyzeLen;
	u8 TxBuf[COM_BUF_LEN * 2];
	u8 ResBuf[COM_BUF_LEN];
	u8 DMABuf[COM_BUF_LEN];
	u8 TempBuf[COM_BUF_LEN];
	u8 RxMode; //接收模式
	u8 RxDataMode;
	u8 TxBusy;
	u8 TxCmd;
	u8 SlipFlagNum;
	u8 SPFlag;
	u8 SleepFlag;
}COM_CtrlStruct;
void Uart_Config(void);
void COM_TxReq(u8 *Data, u32 Len);
void COM_Tx(u8 *Data, u32 Len);
u8 COM_Send(u8 *Data, u32 Len);
void COM_Sleep(void);
void COM_Wakeup(u32 BR);
#endif
