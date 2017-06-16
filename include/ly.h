#ifndef __CUST_LY_H__
#define __CUST_LY_H__

#define LY_HEAD_FLAG 					(0x29)
#define LY_TAIL_FLAG					(0x0d)
#define LY_NO_ALARM 					(0xff)
#define LY_MOVE_ALARM					(0x00)
#define LY_CRASH_ALARM					(0x0B)
#define LY_CUT_ALARM					(0x0C)
#define LY_SIM_ALARM					(0x0F)
#define LY_GNSS_ALARM					(0x10)
#define LY_SENSOR_ALARM					(0x20)
#define LY_LOWPOWER_ALARM				(0x30)
#define LY_ACC_ON_ALARM 				(0x1A)
#define LY_ACC_OFF_ALARM 				(0x1B)

#define LY_MONITOR_VERSION				(0x96)
#define LY_RS232TC_VERSION				(0x97)
#define LY_RS485TC_VERSION				(0x98)
#define LY_UART_VERSION					(0x31)

#define LY_TX_LOCAT_CMD					(0x60)//��λ������
#define LY_TX_HEART_CMD					(0x63)//����������
#define LY_TX_AUTH_CMD					(0x70)//��Ȩ������
#define LY_TX_START_CMD					(0x01)//����������
#define LY_TX_DEVICE_RES_CMD			(0x85)//�ն�Ӧ������
#define LY_TX_ECU_CMD					(0x65)//ECU�����ϱ�

#define LY_RX_AUTH_RES_CMD				(0x9F)//��ȨӦ������
#define LY_RX_START_RES_CMD				(0x86)//������Ӧ������
#define LY_RX_MONITOR_RES_CMD			(0x21)//��̨Ӧ������
#define LY_RX_SET_PID_CMD				(0x45)//�����ն�ID����
#define LY_RX_SET_APN_CMD				(0x48)//�����ն�APN����
#define LY_RX_SET_AUTH_CMD				(0x06)//�����ն˼�Ȩ�ĵ�ַ
#define LY_RX_SET_HEART_INTERVAL_CMD	(0x42)//�����ն������Ļش����
#define LY_RX_SET_NORMAL_INTERVAL_CMD	(0x40)//�����ն˵Ļش����
#define LY_RX_SET_SLEEP_INTERVAL_CMD	(0x41)//�����ն������Ļش����
#define LY_RX_SET_CRASH_LEVEL_CMD		(0x47)//������������
#define LY_RX_SET_MILEAGE_CMD			(0x54)//�����ն����
#define LY_RX_SET_OWNER_CMD				(0x46)//�����ն��û��ֻ���
#define LY_RX_SET_RESTART_CMD			(0x50)//�����ն�����
#define LY_RX_TO_ECU_CMD				(0x55)//����������ECU

#define LY_PACK_HEAD				(0)
#define LY_PACK_VER					(2)
#define LY_PACK_CMD					(3)
#define LY_PACK_LEN					(4)
#define LY_PACK_PID					(6)
#define LY_PACK_TAMP				(10)
#define LY_HEAD_LEN					(15)
#define LY_PACK_DATA				(16)

#define LY_UART_HEAD_4A				(0x4A)
#define LY_UART_HEAD_5A				(0x5A)
#define LY_UART_HEAD_4B				(0x4B)
#define LY_UART_HEAD_5B				(0x5B)
enum
{
	LY_PRO_FIND_HEAD1,
	LY_PRO_FIND_HEAD2,
	LY_PRO_FIND_LEN,
	LY_PRO_FIND_TAIL,

	LY_UART_FIND_HEAD = 0,
	LY_UART_FIND_LEN,
	LY_UART_FIND_TAIL,

	LY_STATE_AUTH = 0,
	LY_STATE_LOGIN,
	LY_STATE_DATA,
	LY_STATE_SLEEP,

	LY_USER_TO_ECU = 1,
};

typedef struct
{
	Buffer_Struct ToECUBuf;
	u32 BufLen;
	u8 LastRx[8];
	u8 ECUAck[8];
	u8 IsAuthOK;
	u8 NeedReAuth;
	u8 NoAck;
}LY_CustDataStruct;

u16 LY_PackData(u8 *Dest, u8 *Src, u16 Len, u8 Version, u8 Cmd);
u16 LY_AuthData(u8 *Dest);
u16 LY_LogInData(u8 *Dest);
u16 LY_LocatData(u8 *Dest, Monitor_RecordStruct *Record);
u16 LY_HeartData(u8 *Dest);
u16 LY_ResponseData(u8 *Dest, u8 Result, u8 IsECU, u8 Vesion, u8 *Data, u16 Len);
u16 LY_ECUData(u8 *Dest, u8 *Data, u16 Len, u8 Version);
s32 LY_ReceiveAnalyze(void *pData);
void LY_Config(void);
u32 LY_ComAnalyze(u8 *RxBuf, u32 RxLen, s32 *Result);
u8 LY_CheckUartHead(u8 Data);
#endif
