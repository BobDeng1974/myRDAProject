#ifndef __JTT_H__
#define __JTT_H__
typedef union
{
	uint32_t dwData;
	uint8_t ucData[4];
	uint8_t *pData;
}JTT_DataUnion;

typedef struct
{
	JTT_DataUnion uData;
	uint32_t ID;
	uint32_t Len;
}JTT_ItemStruct;

#define JTT_PACK_FLAG		(0x7e)
#define JTT_PACK_CODE		(0x7d)
#define JTT_PACK_CODE_F1	(0x02)
#define JTT_PACK_CODE_F2	(0x01)
#define JTT_PACK_HEAD_LEN			(12)
#define JTT_MUILT_PACK_HEAD_LEN		(16)
#define JTT_LOCAT_INFO_BASE_LEN		(28)
enum
{
	JTT_PRO_FIND_HEAD,
	JTT_PRO_FIND_TAIL,

	JTT_STATE_CONNECT = 0,
	JTT_STATE_AUTH,
	JTT_STATE_REG,
	JTT_STATE_DATA,
	JTT_STATE_SLEEP,
	JTT_STATE_USER,
	JTT_MSG_RSA_FLAG = (1 << 10),

	JTT_PARAM_UPLOAD_HEART_PERIOD = 0x00000001,
	JTT_PARAM_TCP_NET_TO,
	JTT_PARAM_TCP_RECONNECT_MAX,
	JTT_PARAM_UDP_NET_TO,
	JTT_PARAM_UDP_RECONNECT_MAX,
	JTT_PARAM_SMS_TO,
	JTT_PARAM_SMS_RETRY_MAX,
	JTT_PARAM_APN_NAME = 0x00000010,
	JTT_PARAM_APN_USER,
	JTT_PARAM_APN_PWD,
	JTT_PARAM_MAIN_IP_URL,
	JTT_PARAM_TCP_PORT = 0x00000018,
	JTT_PARAM_UPLOAD_SLEEP_PERIOD = 0x00000027,
	JTT_PARAM_UPLOAD_ALARM_PERIOD,
	JTT_PARAM_UPLOAD_RUN_PERIOD,
	JTT_PARAM_SMS_CENTER = 0x00000043,							//�������غ���
	JTT_PARAM_CAR_OWNER,										//��������
	JTT_PARAM_ALARM_DISABLE = 0x00000050,
	JTT_PARAM_SMS_ALARM_ENABLE,
	JTT_PARAM_KEY_ALARM = 0x00000054,
	JTT_PARAM_OVERSPEED_VAL,
	JTT_PARAM_OVERSPEED_DELAY,
	JTT_PARAM_CRASH = 0x0000005D,

	JTT_TX_RESPONSE = 0x00000001,
	JTT_TX_HEART = 0x00000002,
	JTT_TX_REG = 0x00000100,
	JTT_TX_UNREG = 0x00000101,
	JTT_TX_AUTH = 0x00000102,
	JTT_TX_PARAM = 0x00000104,
	JTT_TX_UPGRADE_RES = 0x00000108,
	JTT_TX_UPLOAD = 0x00000200,
	JTT_TX_FORCE_UPLOAD = 0x00000201,
	JTT_TX_CAR_CONTROL = 0x00000500,
	JTT_TX_DIRECT_TO_MONITOR = 0x00000900,
	JTT_TX_RSA = 0x00000a00,
	JTT_TX_USER_TEXT = 0x00006006,

	JTT_RX_NORMAIL_RESPONSE = 0x00008001,
	JTT_RX_REG_RESPONSE = 0x00008100,
	JTT_RX_SET_PARAM = 0x00008103,
	JTT_RX_GET_PARAM_ALL = 0x00008104,
	JTT_RX_CONTROL_DEVICE = 0x00008105,
	JTT_RX_GET_PARAM = 0x00008106,
	JTT_RX_UPGRADE_CMD = 0x00008108,
	JTT_RX_FORCE_UPLOAD = 0x00008201,
	JTT_RX_TEXT_INFO = 0x000008300,
	JTT_RX_CAR_CONTROL = 0x00008500,
	JTT_RX_DIRECT_TO_DEV = 0x00008900,
	JTT_RX_SET_RSA = 0x00008a00,
	JTT_DEV_CTRL_SHUTDOWN = 0x03,
	JTT_DEV_CTRL_RESET = 0x04,
	JTT_DEV_CTRL_RECOVERY = 0x05,

	JTT_CUST_ITEM_START = 0x000000E1,

	JTT_LOCAT_ADD_ITEM_MILEAGE = 0x01,
	JTT_LOCAT_ADD_ITEM_OIL = 0x02,
	JTT_LOCAT_ADD_ITEM_CSQ = 0x30,
	JTT_LOCAT_ADD_ITEM_GNSS_NUM = 0x31,	//��������
};

enum JTT_STATUS_ALARM_BYTE
{
    JTT_STATUS_ACC_ON = 0,						//ACC��
    JTT_STATUS_LOCAT_STATUS = 1,
    JTT_STATUS_LOCAT_NS = 2,
    JTT_STATUS_LOCAT_EW = 3,
    JTT_STATUS_CAR_OIL_LOCK = 10,
    JTT_STATUS_CAR_ELEC_LOCK = 11,
    JTT_STATUS_ALARM_ON = 12,
	JTT_STATUS_GPS_MODE = 18,
	JTT_STATUS_BD_MODE = 19,
    JTT_ALARM_OVER_SPEED_ALARM = 1,			//���ٱ���
    JTT_ALARM_GNSS_ERROR = 4,
    JTT_ALARM_ANT_BREAK = 5,
    JTT_ALARM_ANT_SHORT = 6,
    JTT_ALARM_LOW_POWER = 7,
    JTT_ALARM_POWER_DOWN = 8,
    JTT_ALARM_SENSER_ERROR = 15,
	JTT_ALARM_SIM_ERROR = 16,
	JTT_ALARM_GPRS_ERROR = 17,
    JTT_ALARM_STEAL = 26,
    JTT_ALARM_CRASH = 29,
};
uint32_t JTT_PacketHead(uint16_t MsgID, uint16_t MsgSn, uint8_t *SimID, uint16_t MsgLen, uint16_t MsgRSA, uint8_t *Buf);
uint32_t JTT_MuiltPacketHead(uint16_t MsgID, uint16_t MsgSn, uint8_t *SimID, uint16_t MsgLen, uint16_t MsgRSA, uint16_t PacketNum, uint16_t PacketSn, uint8_t *Buf);
void JTT_MakeMonitorID(Monitor_CtrlStruct *Monitor);
int32_t JTT_AnalyzeHead(uint16_t *MsgID, uint16_t *MsgSn, uint8_t *SimID, uint8_t *InBuf, uint16_t InLen, uint32_t *RxLen);
int32_t JTT_AnalyzeReg(uint16_t *MsgSn, uint8_t *Result, uint8_t *AuthCode, uint32_t *AuthLen, uint8_t *Buf, uint32_t RxLen);
int32_t JTT_AnalyzeMonitorRes(uint16_t *MsgID, uint16_t *MsgSn, uint8_t *Result, uint8_t *Buf);
uint32_t JTT_RegMsgBoby(uint16_t ProvinceID, uint16_t CityID, const int8_t *FactoryID, const int8_t *DeviceType, const int8_t *DeviceID, uint8_t Color, const int8_t *CarID, uint16_t CarIDLen, uint8_t *Buf);
uint32_t JTT_LocatBaseInfoMsgBoby(Monitor_RecordStruct *Info, uint8_t *Buf);
uint32_t JTT_DevResMsgBoby(uint16_t MsgID, uint16_t MsgSn, uint8_t Result, uint8_t *Buf);
uint32_t JTT_AddLocatMsgBoby(uint8_t AddID, uint8_t Len, uint8_t *pData, uint8_t *pBuf);
uint32_t JTT_ParamMsgBoby(uint16_t MsgSn, uint8_t Num, uint8_t *Buf);
uint32_t JTT_UpgradeMsgBoby(uint8_t Type, uint8_t Result, uint8_t *Buf);
#endif
