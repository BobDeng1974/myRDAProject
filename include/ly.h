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

#define LY_TX_LOCAT_CMD					(0x60)//定位包信令
#define LY_TX_HEART_CMD					(0x63)//心跳包信令
#define LY_TX_AUTH_CMD					(0x70)//鉴权包信令
#define LY_TX_START_CMD					(0x01)//启动包信令
#define LY_TX_DEVICE_RES_CMD			(0x85)//终端应答信令
#define LY_TX_ECU_CMD					(0x65)//ECU主动上报

#define LY_RX_AUTH_RES_CMD				(0x9F)//鉴权应答信令
#define LY_RX_START_RES_CMD				(0x86)//启动包应答信令
#define LY_RX_MONITOR_RES_CMD			(0x21)//后台应答信令
#define LY_RX_SET_PID_CMD				(0x45)//设置终端ID信令
#define LY_RX_SET_APN_CMD				(0x48)//设置终端APN信令
#define LY_RX_SET_AUTH_CMD				(0x06)//设置终端鉴权的地址
#define LY_RX_SET_HEART_INTERVAL_CMD	(0x42)//设置终端心跳的回传间隔
#define LY_RX_SET_NORMAL_INTERVAL_CMD	(0x40)//设置终端的回传间隔
#define LY_RX_SET_SLEEP_INTERVAL_CMD	(0x41)//设置终端心跳的回传间隔
#define LY_RX_SET_CRASH_LEVEL_CMD		(0x47)//设置震动灵敏度
#define LY_RX_SET_MILEAGE_CMD			(0x54)//设置终端里程
#define LY_RX_SET_OWNER_CMD				(0x46)//设置终端用户手机号
#define LY_RX_SET_RESTART_CMD			(0x50)//设置终端重启
#define LY_RX_TO_ECU_CMD				(0x55)//服务器发给ECU
#define LY_RX_UPLOAD_LOCATION			(0x62)

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
	LY_IOT_ADC_CH_BAT_TEMP = 0,
	LY_IOT_ADC_CH_ENV_TEMP,
	LY_IOT_ADC_CH_BAT_VOL,
	LY_IOT_ADC_CH_UNUSE,
};

typedef struct
{
	Buffer_Struct ToECUBuf;
	uint32_t BufLen;
	uint8_t LastRx[8];
	uint8_t ECUAck[8];
	uint8_t IsAuthOK;
	uint8_t NeedReAuth;
	uint8_t NoAck;
}LY_CustDataStruct;

uint16_t LY_PackData(uint8_t *Dest, uint8_t *Src, uint16_t Len, uint8_t Version, uint8_t Cmd);
uint16_t LY_AuthData(uint8_t *Dest);
uint16_t LY_LogInData(uint8_t *Dest);
uint16_t LY_LocatData(uint8_t *Dest, Monitor_RecordStruct *Record);
uint16_t LY_HeartData(uint8_t *Dest);
uint16_t LY_ResponseData(uint8_t *Dest, uint8_t Result, uint8_t IsECU, uint8_t Vesion, uint8_t *Data, uint16_t Len);
uint16_t LY_ECUData(uint8_t *Dest, uint8_t *Data, uint16_t Len, uint8_t Version);
int32_t LY_ReceiveAnalyze(void *pData);
void LY_Config(void);
uint32_t LY_ComAnalyze(uint8_t *RxBuf, uint32_t RxLen, int32_t *Result);
uint8_t LY_CheckUartHead(uint8_t Data);
#endif
