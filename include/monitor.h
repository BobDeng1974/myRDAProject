#ifndef __CUST_MONITOR_H__
#define __CUST_MONITOR_H__
#define ALARM_CACHE_MAX		(16)
#define DATA_CACHE_MAX		(1024)
#define RES_CACHE_MAX		(16)
#define MONITOR_RUN_TIME	(120)
#define MONITOR_RXBUF_LEN	(4096)
#define MONITOR_TXBUF_LEN	(1600)

enum
{
	MONITOR_STATUS_ALARM_ON,
	MONITOR_STATUS_SIGNAL,
	MONITOR_STATUS_ANT_SHORT,
	MONITOR_STATUS_ANT_BREAK,
	MONITOR_STATUS_GPS_ERROR,
	MONITOR_STATUS_LOWPOWER,
	MONITOR_STATUS_SENSOR_ERROR,
	MONITOR_STATUS_SIM_ERROR,
	MONITOR_STATUS_GPRS_ERROR,
	MONITOR_STATUS_OVERSPEED,
	MONITOR_STATUS_MAX,

	ALARM_TYPE_CRASH = 1,
	ALARM_TYPE_MOVE,
	ALARM_TYPE_CUTLINE,
	ALARM_TYPE_OVERSPEED,
	ALARM_TYPE_LOWPOWER,
	ALARM_TYPE_NOPOWER,
	ALARM_TYPE_ROB,
	ALARM_TYPE_ACC_ON,
	ALARM_TYPE_ACC_OFF,

	CACHE_TYPE_RES = 0,
	CACHE_TYPE_ALARM,
	CACHE_TYPE_DATA,
	CACHE_TYPE_ALL,

	MONITOR_DATA_AUTH = 0, //鉴权，绿源，JTT需要
	MONITOR_DATA_REG,		//注册，JTT需要
	MONITOR_DATA_LOGIN,	//登录，绿源需要
	MONITOR_DATA_UPLOAD,	//定时上传，所有协议均需要
	MONITOR_DATA_LOGOUT,	//登出
};

typedef union
{
	u8 ucID[16];
	u32 dwID;
}Monitor_IDUnion;


typedef struct
{
	Date_Union uDate;
	Time_Union uTime;
	RMC_InfoStruct RMC;
	u32 MileageKM;
	u32 MileageM;
	IO_ValueUnion IOValUnion;
	Cell_InfoUnion CellInfoUnion;
	Cell_InfoUnion NearCellInfoUnion[2];
	u8 NearSingal[2];
	u16 Vbat;
	u16 GsensorVal;
	u16 CrashCNT;
	u16 MoveCNT;
	u8 CN[4];
	u8 DevStatus[MONITOR_STATUS_MAX];
	u8 Alarm;
}Monitor_RecordStruct;

typedef struct
{
	Monitor_RecordStruct Record;
	Net_CtrlStruct Net;
	Monitor_IDUnion MonitorID;
	u32 *Param;
	u32 RecordStartTime;
	u32 HeartStartTime;
	u32 RunStartTime;
	u32 RxLen;
	u32 RxNeedLen;
	u32 AnalzeLen;
	u8 AnalyzeBuf[MONITOR_RXBUF_LEN];
	u8 RecBuf[MONITOR_RXBUF_LEN];
	u8 SendBuf[MONITOR_TXBUF_LEN];
	u8 TempBuf[MONITOR_TXBUF_LEN];
	u8 WakeupFlag;
	u8 ReConnCnt;						//重连次数
	u8 RxState;
	u8 DevCtrlStatus;
	u8 IsRunMode;						//骑行/停止
	u8 IsWork;							//工作/休眠
	void *CustData;						//平台自定义数据
}Monitor_CtrlStruct;


typedef union
{
	Monitor_RecordStruct Data;
	u8 pad[124];
}Monitor_RecordUnion;

typedef struct
{
	Monitor_RecordUnion uRecord;
	u32 CRC32;
}Monitor_DataStruct;

typedef struct
{
	u8 Data[1024];
	u32 Len;
	u32 CRC32;
}Monitor_ResponseStruct;

typedef struct
{
	RBuffer DataBuf;
	RBuffer AlarmBuf;
	RBuffer ResBuf;
	Monitor_DataStruct AlarmCache[ALARM_CACHE_MAX];
	Monitor_DataStruct DataCache[DATA_CACHE_MAX];
	Monitor_ResponseStruct ResCache[RES_CACHE_MAX];
}Monitor_CacheStruct;
void Monitor_InitCache(void);
void Monitor_Record(Monitor_RecordStruct *Record);
void Monitor_RecordData(void);
void Monitor_RecordAlarm(u8 Type, u16 CrashCNT, u16 MoveCNT);
void Monitor_RecordResponse(u8 *Data, u32 Len);
u8 Monitor_ExtractData(Monitor_RecordStruct *Data);
u8 Monitor_ExtractAlarm(Monitor_RecordStruct *Alarm);
u32 Monitor_ExtractResponse(u8 *Response);
void Monitor_DelCache(u8 Type, u8 IsAll);
u32 Monitor_GetCacheLen(u8 Type);
void Monitor_StateCheck(void);
void Monitor_Wakeup(void);
void Monitor_Upload(void);
#endif
