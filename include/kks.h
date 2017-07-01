#ifndef __KKS_H__
#define __KKS_H__
#define KKS_SHORT_HEAD 	(0x78)
#define KKS_LONG_HEAD	(0x79)
#define KKS_TAIL1		(0x0d)
#define KKS_TAIL2		(0x0a)

#define KKS_LOGIN_TX	(0x01)
#define KKS_HEART_TX	(0x13)
#define KKS_CMD_TX		(0x21)
#define KKS_ALARM_TX	(0x26)
#define KKS_LOCAT_TX	(0x22)
#define KKS_LBS_TX		(0x28)
#define KKS_GPS_DW_TX	(0x2A)
#define KKS_LBS_DW_TX	(0x17)
#define KKS_TIME_TX		(0x8A)
#define KKS_DIRSEND_TX	(0x94)

#define KKS_LOGIN_RX	(0x01)
#define KKS_HEART_RX	(0x13)
#define KKS_CMD_RX		(0x80)
#define KKS_ALARM_RX	(0x26)
#define KKS_TIME_RX		(0x8A)
#define KKS_DW_CH_RX	(0x17)
#define KKS_DW_EN_RX	(0x97)

#define KKS_DEV_TYPE_L	(0)
#define KKS_DEV_TYPE_H	(0x70)

#define KKS_ALARM_CUTLINE	(0x02)
#define KKS_ALARM_CRASH		(0x03)
#define KKS_ALARM_OVERSPEED	(0x06)
#define KKS_ALARM_MOVE		(0x09)
#define KKS_ALARM_LOWPOWER	(0x0E)
#define KKS_ALARM_ACC_OFF	(0xff)
#define KKS_ALARM_ACC_ON	(0xfe)
enum
{
	KKS_PRO_FIND_HEAD1,
	KKS_PRO_FIND_HEAD2,
	KKS_PRO_FIND_HEAD3,
	KKS_PRO_FIND_LEN,
	KKS_PRO_FIND_TAIL1,
	KKS_PRO_FIND_TAIL2,

	KKS_UART_FIND_HEAD = 0,
	KKS_UART_FIND_LEN,
	KKS_UART_FIND_TAIL,

	KKS_STATE_AUTH = 0,
	KKS_STATE_DATA,
	KKS_STATE_SLEEP,

};

typedef struct
{
	u16 MsgSn;
	u16 MCC;
	u8 MNC;
	u8 DevInfo;
	u8 Power;
	u8 Signal;
	u8 IsAuthOK;
	u8 NoAck;
}KKS_CustDataStruct;

typedef struct
{
	u8 DevID[8];
	u8 DevType[2];
	u8 DevZone[2];
}KKS_LoginBody;

typedef struct
{
	u8 DevInfo[1];
	u8 Power[1];
	u8 Signal[1];
	u8 Language[2];
}KKS_HeartBody;

typedef struct
{
	u8 LAI[2];
	u8 CI[3];
	u8 RSSI[1];
}KKS_LBSInfo;

typedef struct
{
	u8 DateTime[6];
	u8 GPSInfo[1];
	u8 Lgt[4];
	u8 Lat[4];
	u8 Speed[1];
	u8 State[2];
	u8 MCC[2];
	u8 MNC[1];
	u8 LAI[2];
	u8 CI[3];
	u8 ACC[1];
	u8 Mode[1];
	u8 Type[1];
	u8 Mileage[4];
}KKS_LocatBody;

typedef struct
{
	u8 DateTime[6];
	u8 MCC[2];
	u8 MNC[1];
	KKS_LBSInfo LBSList[7];
	u8 Diff[1];
	u8 Language[2];
}KKS_LBSBody;

typedef struct
{
	u8 DateTime[6];
	u8 GPSInfo[1];
	u8 Lat[4];
	u8 Lgt[4];
	u8 Speed[1];
	u8 State[2];
	u8 LBSLen[1];
	u8 MCC[2];
	u8 MNC[1];
	u8 LAI[2];
	u8 CI[3];
	u8 DevInfo[1];
	u8 Power[1];
	u8 Signal[1];
	u8 Language[2];
}KKS_AlarmBody;

typedef struct
{
	u8 DateTime[6];
	u8 GPSInfo[1];
	u8 Lat[4];
	u8 Lgt[4];
	u8 Speed[1];
	u8 State[2];
	u8 Phone[21];
	u8 Language[2];
}KKS_LocatRegBody;

typedef struct
{
	u8 MCC[2];
	u8 MNC[1];
	u8 LAI[2];
	u8 CI[3];
	u8 Phone[21];
	u8 Language[2];
}KKS_LBSRegBody;



#define KKS_CHINESE	(0x01)
#define KKS_ENGLISH	(0x02)
void KKS_Config(void);
u8 KKS_CheckUartHead(u8 Data);
u32 KKS_DirSendTx(u8 *Src, u16 Len);
#endif
