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
	uint8_t ucID[16];
	uint32_t dwID;
}Monitor_IDUnion;


typedef struct
{
	Date_Union uDate;
	Time_Union uTime;
	RMC_InfoStruct RMC;
	uint32_t MileageKM;
	uint32_t MileageM;
	IO_ValueUnion IOValUnion;
	Cell_InfoUnion CellInfoUnion;
	Cell_InfoUnion NearCellInfoUnion[2];
	uint8_t NearSingal[2];
	uint16_t Vbat;
	uint16_t GsensorVal;
	uint16_t CrashCNT;
	uint16_t MoveCNT;
	uint8_t CN[4];
	uint8_t DevStatus[MONITOR_STATUS_MAX];
	uint8_t Alarm;
}Monitor_RecordStruct;

typedef struct
{
	Monitor_RecordStruct Record;
	Net_CtrlStruct Net;
	Monitor_IDUnion MonitorID;
	uint32_t *Param;
	uint32_t RecordStartTime;
	uint32_t HeartStartTime;
	uint32_t RunStartTime;
	uint32_t RxLen;
	uint32_t RxNeedLen;
	uint32_t AnalzeLen;
	uint8_t AnalyzeBuf[MONITOR_RXBUF_LEN];
	uint8_t RxBuf[MONITOR_RXBUF_LEN];
	uint8_t TxBuf[MONITOR_TXBUF_LEN];
	uint8_t TempBuf[MONITOR_TXBUF_LEN];
	uint8_t WakeupFlag;
	uint8_t ReConnCnt;						//重连次数
	uint8_t RxState;
	uint8_t DevCtrlStatus;
	uint8_t IsRunMode;						//骑行/停止
	uint8_t IsWork;							//工作/休眠
	void *CustData;						//平台自定义数据
}Monitor_CtrlStruct;


typedef union
{
	Monitor_RecordStruct Data;
	uint8_t pad[124];
}Monitor_RecordUnion;

typedef struct
{
	Monitor_RecordUnion uRecord;
	uint32_t CRC32;
}Monitor_DataStruct;

typedef struct
{
	uint8_t Data[1024];
	uint32_t Len;
	uint32_t CRC32;
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
void Monitor_RecordAlarm(uint8_t Type, uint16_t CrashCNT, uint16_t MoveCNT);
void Monitor_RecordResponse(uint8_t *Data, uint32_t Len);
uint8_t Monitor_ExtractData(Monitor_RecordStruct *Data);
uint8_t Monitor_ExtractAlarm(Monitor_RecordStruct *Alarm);
uint32_t Monitor_ExtractResponse(uint8_t *Response);
void Monitor_DelCache(uint8_t Type, uint8_t IsAll);
uint32_t Monitor_GetCacheLen(uint8_t Type);
void Monitor_StateCheck(void);
void Monitor_Wakeup(void);
void Monitor_Upload(void);
#endif
