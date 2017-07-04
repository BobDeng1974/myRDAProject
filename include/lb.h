#ifndef __LB_H__
#define __LB_H__
#define LB_SHORT_HEAD 	(0x78)
#define LB_LONG_HEAD	(0x79)
#define LB_TAIL1		(0x0d)
#define LB_TAIL2		(0x0a)

#define LB_LOGIN_TX	(0x01)
#define LB_HEART_TX	(0x13)
#define LB_CMD_TX		(0x21)
#define LB_ALARM_TX	(0x26)
#define LB_LOCAT_TX	(0x22)
#define LB_LBS_TX		(0x28)
#define LB_GPS_DW_TX	(0x2A)
#define LB_LBS_DW_TX	(0x17)
#define LB_TIME_TX		(0x8A)


#define LB_LOGIN_RX	(0x01)
#define LB_HEART_RX	(0x13)
#define LB_CMD_RX		(0x80)
#define LB_ALARM_RX	(0x26)
#define LB_TIME_RX		(0x8A)
#define LB_DW_CH_RX	(0x17)
#define LB_DW_EN_RX	(0x97)

#define LB_ECS_TO_SERV	(0x9C)
#define LB_SERV_TO_ECS	(0x9B)

#define LB_DEV_TYPE_L	(0)
#define LB_DEV_TYPE_H	(0x70)

#define LB_ALARM_CUTLINE	(0x02)
#define LB_ALARM_CRASH		(0x03)
#define LB_ALARM_OVERSPEED	(0x06)
#define LB_ALARM_MOVE		(0x09)
#define LB_ALARM_LOWPOWER	(0x19)
#define LB_ALARM_NOPOWER	(0x20)
#define LB_ALARM_ACC_OFF	(0xff)
#define LB_ALARM_ACC_ON	(0xfe)
#define LB_485_HEAD		(0xaa)
#define LB_485_DEV_INFO	(0x05)
#define LB_485_DIR_SEND	(0x1e)
#define LB_LB_CTRL		(0x02)
#define LB_BA_CTRL		(0x01)
enum
{
	LB_PRO_FIND_HEAD1,
	LB_PRO_FIND_HEAD2,
	LB_PRO_FIND_HEAD3,
	LB_PRO_FIND_LEN,
	LB_PRO_FIND_TAIL1,
	LB_PRO_FIND_TAIL2,

	LB_UART_FIND_HEAD = 0,
	LB_UART_FIND_LEN,
	LB_UART_FIND_TAIL,

	LB_STATE_AUTH = 0,
	LB_STATE_DATA,
	LB_STATE_SLEEP,

	LB_485_HEAD_POS = 0,
	LB_485_CMD_POS,
	LB_485_LEN_POS,
	LB_485_DATA_POS,

};

typedef struct
{
	u16 MsgSn;
	u16 MCC;
	u8 MNC;
	u8 DevInfo;
	u8 Power;
	u8 Signal;
	u8 SatelliteNum;
	u8 IsAuthOK;
	u8 NoAck;
	u8 ECSData[248];
	u8 ECSDataLen;
}LB_CustDataStruct;

typedef struct
{
	u8 DevID[8];
	u8 DevType[2];
	u8 DevZone[2];
}LB_LoginBody;

typedef struct
{
	u8 DevInfo[1];
	u8 Power[1];
	u8 Signal[1];
	u8 Language[2];
}LB_HeartBody;

typedef struct
{
	u8 LAI[2];
	u8 CI[3];
	u8 RSSI[1];
}LB_LBSInfo;

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
}LB_LocatBody;

typedef struct
{
	u8 DateTime[6];
	u8 MCC[2];
	u8 MNC[1];
	LB_LBSInfo LBSList[7];
	u8 Diff[1];
	u8 Language[2];
}LB_LBSBody;

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
}LB_AlarmBody;

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
}LB_LocatRegBody;

typedef struct
{
	u8 MCC[2];
	u8 MNC[1];
	u8 LAI[2];
	u8 CI[3];
	u8 Phone[21];
	u8 Language[2];
}LB_LBSRegBody;



#define LB_CHINESE	(0x01)
#define LB_ENGLISH	(0x02)
void LB_Config(void);
u8 LB_CheckUartHead(u8 Data);
u32 LB_ECSToServerTx(u8 *Src, u16 Len);
u16 LB_SendGPSInfo(u8 AlarmType, u8 *Buf);
void LB_ComAnalyze(u8 *Data, u8 Len);
#endif
