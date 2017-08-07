#ifndef __KQ_H__
#define __KQ_H__
#define KQ_JTT_COLOR_GPRS_BLE (0x03)
#define KQ_TTS_MAGIC_NUM	(0x04)
#define KQ_LED_MAGIC_NUM	(0x01)
#define KQ_TTS_LEN_MAX		(56)

enum
{
	TTS_CODE_START = 1,
	TTS_CODE_CONNECTED = TTS_CODE_START,
	//TTS_CODE_UNLOCK_FAIL,
	TTS_CODE_LOCK_OPEN_OK,
	TTS_CODE_LOCK_CLOSE_OK,
	TTS_CODE_DISCONNECTED,
	TTS_TEST_CODE_MAX,
	TTS_CODE_MAX = 16,

	LED_CODE_START = 1,
	LED_CODE_FIND_BIKE = LED_CODE_START,
	LED_CODE_CHECK_BIKE,
	LED_CODE_SENSOR_ALARM,
	LED_CODE_MAX,
};

enum
{
	//BLE<->GPRS
	KQ_CMD_SYS_ON = 1,				//唤醒成功
	KQ_CMD_SYS_DOWN,			//进入休眠
	KQ_CMD_DOWNLOAD_BT,			//下载蓝牙升级包
	KQ_CMD_DOWNLOAD_GPRS,		//下载GPS升级包
	KQ_CMD_UPGRADE_GPRS,		//GPS自升级开始
	KQ_CMD_SET_TIME_PARAM,		//汇报时间设置
	KQ_CMD_BT_808,				//BLE上报
	KQ_CMD_SET_IP,				//服务器IP设置
	KQ_CMD_SET_BT,				//透传信息设置
	KQ_CMD_PLAY_VOICE,			//固定语音，BLE主动
	KQ_CMD_PLAY_TTS,			//动态语音，BLE主动

	KQ_PRO_HEAD_COMM = 0xFE,
	KQ_PRO_HEAD_EVENT = 0xFD,
	KQ_PRO_HEAD_SET_IP = 0xFC,
	KQ_PRO_HEAD_SET_BT = 0xFB,
	KQ_PRO_HEAD_PLAY = 0xFA,
	KQ_PRO_CMD_SYS_ON = 0x01,
	KQ_PRO_CMD_SYS_DOWN,			//进入休眠
	KQ_PRO_CMD_DOWNLOAD_BT,			//下载蓝牙升级包
	KQ_PRO_CMD_DOWNLOAD_GPRS,		//下载GPS升级包
	KQ_PRO_CMD_UPGRADE_GPRS,		//GPS自升级开始
	KQ_PRO_CMD_SET_TIME_PARAM,		//汇报时间设置

	KQ_PRO_END = 0xCB,

	KQ_DT_SET_RSA = 0x01,
	KQ_DT_SET_SENSOR_EN,
	KQ_DT_SET_SENSOR_TOGGLE,
	KQ_DT_SET_LED,
	KQ_DT_SET_UNLOCK,
	KQ_DT_SET_OTHER,


	KQ_USER_STATE_IDLE = 0,
	KQ_USER_STATE_UPGRADE_BT,
	KQ_USER_STATE_PLAY,

	KQ_JTT_LOCK_OPEN_TIMES = 0,
	KQ_JTT_LOCK_STATE,
	KQ_JTT_VBAT_STATE,
	KQ_JTT_GSM_INFO,
	KQ_JTT_GSM_VERSION,
	KQ_JTT_BLE_VERSION,
	KQ_JTT_NO_CHARGE_TIME,
	KQ_JTT_CUST_ITEM_MAX,

	KQ_BLE_LOCK_OPEN_TIMES = 0xE1,
	KQ_BLE_LOCK_STATE = 0xE2,
	KQ_BLE_VBAT_STATE = 0xE3,
	KQ_BLE_UPOLOAD_INFO = 0xE5,
	KQ_BLE_VERSION = 0xE6,
	KQ_BLE_NO_CHARGE_TIME = 0xE7,
	KQ_BLE_IP_INFO = 0xE8,
	KQ_BLE_MAC = 0xE9,
	KQ_BLE_CTRL_INFO = 0xEA,

	KQ_BLE_STATE_LOCK = 0x01,
	KQ_BLE_MODE_LOCK_UPLOAD = (0x01) << 1,
	KQ_BLE_MODE_PREIOD_UPLOAD = (0x02) << 1,
	KQ_BLE_MODE_FORCE_UPLOAD = (0x03) << 1,
	KQ_BLE_MODE_GSENSOR_UPLOAD = (0x04) << 1,
	KQ_BLE_STATE_WORK = (1 << 5),
	KQ_BLE_STATE_CHARGE = (1 << 6),

	KQ_PARAM_HEART_TIME_SN = 0,
	KQ_PARAM_APN_SN,					//主服务器APN
	KQ_PARAM_IP_SN,							//主服务器地址（IP/域名）
	KQ_PARAM_PORT_SN,						//服务器TCP端口
	KQ_PARAM_LOCK_UPLOAD_TIME_SN,			//关锁后上传时间间隔
	KQ_PARAM_LOCK_UPLOAD_EN_SN,				//表示关锁后是否要上传定位信息
	KQ_PARAM_UPLOAD_TIME_SN,				//上报时间间隔
	KQ_PARAM_GSENSOR_EN_SN,					//Gsonser功能使能开关
	KQ_PARAM_GSENSOR_UPLOAD_SN,				//Gsonser振动唤醒是否上传位置数据开关
	KQ_PARAM_VOICE_SN,						//语音
	KQ_PARAM_LED_SN,						//LED闪烁
	KQ_PARAM_MAX,

	KQ_PARAM_GSENSOR_EN = 0x0000e000,					//Gsonser功能使能开关
	KQ_PARAM_GSENSOR_UPLOAD,				//Gsonser振动唤醒是否上传位置数据开关
	KQ_PARAM_VOICE = 0x0000f001,						//语音
	KQ_PARAM_LED,						//LED闪烁
};

typedef struct
{
	JTT_ItemStruct CustItem[KQ_JTT_CUST_ITEM_MAX];
	JTT_ItemStruct ParamItem[KQ_PARAM_MAX];
	uint32_t AuthCodeLen;
	uint16_t Port;
	uint16_t LastTxMsgID;
	uint16_t LastRxMsgID;
	uint16_t LastTxMsgSn;
	uint16_t LastRxMsgSn;
	uint8_t FTPCmd[128];
	uint8_t AuthCode[AUTH_CODE_LEN];
	uint8_t CtrlInfo[2];
	uint8_t UploadInfo[4];
	uint8_t IP[6];
	uint8_t Mac[6];
	uint8_t IsLastTxCmdOK;
	uint8_t IsRegOK;							//注册成功/失败
	uint8_t ParamUpload[KQ_PARAM_MAX];
	uint8_t WaitFlag;
	uint8_t BLECmd;
	uint8_t BLECmdLen;
	uint8_t BLECmdData[128];
	uint8_t IsWaitOk;
	uint8_t UpgradeType;
	uint8_t UpgradeResult;
	uint8_t BLEUpgradeStart;
	uint8_t BLEReportFlag;
	uint8_t BLEAddr[12];

}KQ_CustDataStruct;

typedef struct
{
	uint8_t Len;
	uint8_t Repeat;
	uint8_t Interval;
	uint8_t Data[57];
}TTS_CodeDataStruct;

typedef union
{
	TTS_CodeDataStruct TTSData;
	uint8_t Pad[60];
}TTS_CodeDataUnion;

typedef struct
{
	TTS_CodeDataUnion uTTSData;
	uint8_t MagicNum;
	uint8_t Code;
	uint16_t CRC16;
}TTS_CodeSaveStruct;

typedef struct
{
	uint8_t Color;
	uint8_t FlushTime;
	uint8_t KeepTime;
	uint8_t Pad;
}LED_CodeDataStruct;

typedef union
{
	LED_CodeDataStruct LEDData;
	uint8_t Pad[4];
}LED_CodeDataUnion;

typedef struct
{
	LED_CodeDataUnion uLEDData;
	uint8_t MagicNum;
	uint8_t Code;
	uint16_t CRC16;
}LED_CodeSaveStruct;

uint8_t KQ_CheckUartHead(uint8_t Data);
uint32_t KQ_ComTxPack(uint8_t KQCmd, uint8_t *Data, uint32_t Len, uint8_t *Buf);
uint32_t KQ_ComAnalyze(uint8_t *RxBuf, uint32_t RxLen, uint8_t *TxBuf, uint32_t TxBufLen, int32_t *Result);
uint32_t KQ_JTTUpgradeCmdTx(uint8_t *Buf);
void KQ_Config(void);
void KQ_StartTTSCode(uint8_t Code, uint8_t Time, uint32_t Delay);
TTS_CodeDataStruct *KQ_GetTTSCodeData(void);
int32_t KQ_SaveTTSCode(TTS_CodeDataStruct *TTSCodeData, uint8_t Code);
void KQ_StartLEDCode(uint8_t Code);
LED_CodeDataStruct *KQ_GetLEDCodeData(void);
int32_t KQ_SaveLEDCode(LED_CodeDataStruct *LEDCodeData, uint8_t Code);
uint32_t KQ_BLEReport(uint8_t *TxBuf, uint32_t TxBufLen);
void KQ_TTSInit(void);
#endif
