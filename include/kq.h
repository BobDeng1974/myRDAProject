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
	u32 AuthCodeLen;
	u16 Port;
	u16 LastTxMsgID;
	u16 LastRxMsgID;
	u16 LastTxMsgSn;
	u16 LastRxMsgSn;
	u8 FTPCmd[128];
	u8 AuthCode[AUTH_CODE_LEN];
	u8 CtrlInfo[2];
	u8 UploadInfo[4];
	u8 IP[6];
	u8 Mac[6];
	u8 IsLastTxCmdOK;
	u8 IsRegOK;							//注册成功/失败
	u8 ParamUpload[KQ_PARAM_MAX];
	u8 WaitFlag;
	u8 BLECmd;
	u8 BLECmdLen;
	u8 BLECmdData[128];
	u8 IsWaitOk;
	u8 UpgradeType;
	u8 UpgradeResult;
	u8 BLEUpgradeStart;
	u8 BLEReportFlag;
	u8 BLEAddr[12];

}KQ_CustDataStruct;

typedef struct
{
	u8 Len;
	u8 Repeat;
	u8 Interval;
	u8 Data[57];
}TTS_CodeDataStruct;

typedef union
{
	TTS_CodeDataStruct TTSData;
	u8 Pad[60];
}TTS_CodeDataUnion;

typedef struct
{
	TTS_CodeDataUnion uTTSData;
	u8 MagicNum;
	u8 Code;
	u16 CRC16;
}TTS_CodeSaveStruct;

typedef struct
{
	u8 Color;
	u8 FlushTime;
	u8 KeepTime;
	u8 Pad;
}LED_CodeDataStruct;

typedef union
{
	LED_CodeDataStruct LEDData;
	u8 Pad[4];
}LED_CodeDataUnion;

typedef struct
{
	LED_CodeDataUnion uLEDData;
	u8 MagicNum;
	u8 Code;
	u16 CRC16;
}LED_CodeSaveStruct;

u8 KQ_CheckUartHead(u8 Data);
u32 KQ_ComTxPack(u8 KQCmd, u8 *Data, u32 Len, u8 *Buf);
u32 KQ_ComAnalyze(u8 *RxBuf, u32 RxLen, u8 *TxBuf, u32 TxBufLen, s32 *Result);
u32 KQ_JTTUpgradeCmdTx(u8 *Buf);
void KQ_Config(void);
void KQ_StartTTSCode(u8 Code, u8 Time, u32 Delay);
TTS_CodeDataStruct *KQ_GetTTSCodeData(void);
s32 KQ_SaveTTSCode(TTS_CodeDataStruct *TTSCodeData, u8 Code);
void KQ_StartLEDCode(u8 Code);
LED_CodeDataStruct *KQ_GetLEDCodeData(void);
s32 KQ_SaveLEDCode(LED_CodeDataStruct *LEDCodeData, u8 Code);
u32 KQ_BLEReport(u8 *TxBuf, u32 TxBufLen);
void KQ_TTSInit(void);
#endif
