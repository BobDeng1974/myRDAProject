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
	uint16_t MsgSn;
	uint16_t MCC;
	uint8_t MNC;
	uint8_t DevInfo;
	uint8_t Power;
	uint8_t Signal;
	uint8_t SatelliteNum;
	uint8_t IsAuthOK;
	uint8_t NoAck;
	uint8_t ECSData[248];
	uint8_t ECSDataLen;
	uint8_t ECSNeedResponse;	//服务器下发的透传命令
}LB_CustDataStruct;

typedef struct
{
	uint8_t DevID[8];
	uint8_t DevType[2];
	uint8_t DevZone[2];
}LB_LoginBody;

typedef struct
{
	uint8_t DevInfo[1];
	uint8_t Power[1];
	uint8_t Signal[1];
	uint8_t Language[2];
}LB_HeartBody;

typedef struct
{
	uint8_t LAI[2];
	uint8_t CI[3];
	uint8_t RSSI[1];
}LB_LBSInfo;

typedef struct
{
	uint8_t DateTime[6];
	uint8_t GPSInfo[1];
	uint8_t Lgt[4];
	uint8_t Lat[4];
	uint8_t Speed[1];
	uint8_t State[2];
	uint8_t MCC[2];
	uint8_t MNC[1];
	uint8_t LAI[2];
	uint8_t CI[3];
	uint8_t ACC[1];
	uint8_t Mode[1];
	uint8_t Type[1];
	uint8_t Mileage[4];
}LB_LocatBody;

typedef struct
{
	uint8_t DateTime[6];
	uint8_t MCC[2];
	uint8_t MNC[1];
	LB_LBSInfo LBSList[7];
	uint8_t Diff[1];
	uint8_t Language[2];
}LB_LBSBody;

typedef struct
{
	uint8_t DateTime[6];
	uint8_t GPSInfo[1];
	uint8_t Lat[4];
	uint8_t Lgt[4];
	uint8_t Speed[1];
	uint8_t State[2];
	uint8_t LBSLen[1];
	uint8_t MCC[2];
	uint8_t MNC[1];
	uint8_t LAI[2];
	uint8_t CI[3];
	uint8_t DevInfo[1];
	uint8_t Power[1];
	uint8_t Signal[1];
	uint8_t Language[2];
}LB_AlarmBody;

typedef struct
{
	uint8_t DateTime[6];
	uint8_t GPSInfo[1];
	uint8_t Lat[4];
	uint8_t Lgt[4];
	uint8_t Speed[1];
	uint8_t State[2];
	uint8_t Phone[21];
	uint8_t Language[2];
}LB_LocatRegBody;

typedef struct
{
	uint8_t MCC[2];
	uint8_t MNC[1];
	uint8_t LAI[2];
	uint8_t CI[3];
	uint8_t Phone[21];
	uint8_t Language[2];
}LB_LBSRegBody;



#define LB_CHINESE	(0x01)
#define LB_ENGLISH	(0x02)
void LB_Config(void);
uint8_t LB_CheckUartHead(uint8_t Data);
uint32_t LB_ECSToServerTx(uint8_t *Src, uint16_t Len);
uint32_t LB_ServerToECSTx(uint8_t *Src, uint16_t Len);
uint16_t LB_SendGPSInfo(uint8_t AlarmType, uint8_t *Buf);
void LB_ComAnalyze(uint8_t *Data, uint8_t Len, uint8_t TxCmd);
#endif
