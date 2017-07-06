#ifndef __USER_H__
#define __USER_H__

#include "platform_api.h"

#define __CUST_LAT_DEGREE__		(0)
#define __CUST_LAT_MIN__		(0)
#define __CUST_LAT_NS__			'N'
#define __CUST_LGT_DEGREE__		(0)
#define __CUST_LGT_MIN__		(0)
#define __CUST_LGT_EW__			'E'
#if (__CUST_CODE__ == __CUST_LY__)
#define __CUST_IP_ADDR1__		(0)
#define __CUST_IP_ADDR2__ 		(0)
#define __CUST_IP_ADDR3__ 		(0)
#define __CUST_IP_ADDR4__		(0)
#define __CUST_URL__			"auth.lyiot.top"
#define __CUST_TCP_PORT__		(5006)
#define __CUST_UDP_PORT__		(0)
#define DEV_VER					"GTDM_LY_1.00"
#elif (__CUST_CODE__ == __CUST_KQ__)
#define __CUST_IP_ADDR1__		(115)
#define __CUST_IP_ADDR2__		(231)
#define __CUST_IP_ADDR3__		(73)
#define __CUST_IP_ADDR4__		(229)
#define __CUST_URL__			""
#define __CUST_TCP_PORT__		(18888)
#define __CUST_UDP_PORT__		(0)
#elif (__CUST_CODE__ == __CUST_GLEAD__)
#define __CUST_IP_ADDR1__		(0)
#define __CUST_IP_ADDR2__		(0)
#define __CUST_IP_ADDR3__		(0)
#define __CUST_IP_ADDR4__		(0)
#define __CUST_URL__			"www.bdclw.net"
#define __CUST_TCP_PORT__		(8083)
#define __CUST_UDP_PORT__		(0)

#undef __CUST_LAT_DEGREE__
#undef __CUST_LAT_MIN__
#undef __CUST_LAT_NS__
#undef __CUST_LGT_DEGREE__
#undef __CUST_LGT_MIN__
#undef __CUST_LGT_EW__

#define __CUST_LAT_DEGREE__		(30)
#define __CUST_LAT_MIN__		(473947)
#define __CUST_LAT_NS__			'N'
#define __CUST_LGT_DEGREE__		(120)
#define __CUST_LGT_MIN__		(458447)
#define __CUST_LGT_EW__			'E'
#elif (__CUST_CODE__ == __CUST_LB__)
#define __CUST_IP_ADDR1__ (221)
#define __CUST_IP_ADDR2__ (6)
#define __CUST_IP_ADDR3__ (104)
#define __CUST_IP_ADDR4__ (178)
#define __CUST_URL__		""
#define __CUST_TCP_PORT__ (7911)
#define __CUST_UDP_PORT__ (0)
#endif


#include "os.h"
#include "CApi.h"
#include "io.h"
#include "detect.h"
#include "gprs.h"
#include "gps.h"
#include "usart.h"
#include "setting.h"
#include "net.h"
#include "monitor.h"
#include "alarm.h"
#include "usp.h"
#include "ftp.h"
#include "ly.h"
#include "sms.h"
#include "lv.h"
#include "jtt.h"
#include "kq.h"
#include "lb.h"
#include "cc2541.h"
#include "myprotocol.h"
#include "AES.h"
#include "led.h"
#include "ntp.h"
#define PRINT_NORMAL	(0)
#define PRINT_GPS		(1)
#define PRINT_TEST		(2)
enum
{
	SYSTEM_POWER_STOP,
	SYSTEM_POWER_LOW,
	SYSTEM_POWER_ON,
};

enum EV_MMI_ENUM
{
	EV_MMI_REBOOT = EV_MMI_EV_BASE,
	EV_MMI_GPRS_READY,
	EV_MMI_GPRS_GET_HOST,
	EV_MMI_NET_CONNECT_OK,
	EV_MMI_NET_CLOSED,
	EV_MMI_NET_REMOTE_CLOSE,
	EV_MMI_NET_SEND_OK,
	EV_MMI_NET_REC,
	EV_MMI_NET_ERROR,
	EV_MMI_MONITOR_UPLOAD,
	EV_MMI_MONITOR_HEART,
	EV_MMI_MONITOR_WAKEUP,
	EV_MMI_GPS_ANALYZE,
	EV_MMI_GPS_REBOOT,
	EV_MMI_AGPS_FILE_OK,
	EV_MMI_COM_ANALYZE,
	EV_MMI_COM_TX_REQ,
	EV_MMI_COM_NEW_BR,
	EV_MMI_COM_TO_USER,
	EV_MMI_FTP_START,
	EV_MMI_FTP_DATA_START,
	EV_MMI_FTP_DATA_CONNECT,
	EV_MMI_FTP_DATA_STOP,
	EV_MMI_FTP_DATA_FINISH,
	EV_MMI_FTP_FINISH,
	EV_MMI_ALARM_ON,
	EV_MMI_ALARM_OFF,
	EV_MMI_USER_REQ,
	EV_MMI_START_REMOTE,
};

enum TASK_ID_ENUM
{
	//ÈÎÎñ¾ä±ú
	MAIN_TASK_ID,
	GPRS_TASK_ID,
	GPS_TASK_ID,
	MONITOR_TASK_ID,
	FTP_CTRL_TASK_ID,
	FTP_DATA_TASK_ID,
	COM_TASK_ID,
	USER_TASK_ID,
	REMOTE_TASK_ID,
	NTP_TASK_ID,
	TASK_ID_MAX,
};

enum SYS_STATE_ENUM
{
	//×´Ì¬
	SYSTEM_STATE,
	SIM_STATE,
	SMS_STATE,
	REG_STATE,
	GPRS_STATE,
	GPRS_ATTACH_STATE,
	GPRS_ACT_STATE,			//bit0~bit6, pdp1~7
	MONITOR_STATE,
	RSSI_STATE,
	GPS_STATE,
	ALARM_STATE,
	CRASH_STATE,
	MOVE_STATE,
	CUTLINE_STATE,
	OVERSPEED_STATE,
	PRINT_STATE,
	FIRST_LOCAT_STATE,
	TRACE_STATE,
	WDG_STATE,
	LED_STATE,
	LED_STATE_MAX = LED_STATE + LED_TYPE_MAX - 1,
	REBOOT_STATE,
	STATE_MAX,
};

enum SYS_ERROR_ENUM
{
	//¹ÊÕÏ
	SIM_ERROR,
	GPRS_ERROR,
	GPS_ERROR,
	ANT_SHORT_ERROR,
	ANT_BREAK_ERROR,
	SENSOR_ERROR,
	LOW_POWER_ERROR,
	ERROR_MAX,
};

enum SYS_VAR_ENUM
{
	SYS_TIME,
	VBAT,
	UTC_DATE,
	UTC_TIME,
	IO_VAL,
	GSENSOR_VAL,
	GSENSOR_ALARM_VAL,
	GSENSOR_MONITOR_VAL,
	GSENSOR_KEEP_VAL,
	CELL_ID,
	SHUTDOWN_TIME,
	SOFTWARE_VERSION,
	MAIN_FREQ,
	TRACE_TO,
	VAR_MAX,
};

enum TIMER_ID_ENUM
{
	TRACE_TIMER_ID,
	DETECT_TIMER_ID,
	COM_MODE_TIMER_ID,
	COM_RX_TIMER_ID,
	MONITOR_TIMER_ID,
	CUST_TIMER_ID,
	FTP_CTRL_TIMER_ID,
	FTP_DATA_TIMER_ID,
	USER_TIMER_ID,
	TTS_TIMER_ID,
	REMOTE_TIMER_ID,
	NTP_TIMER_ID,
	LED_TIMER_ID,
	LED_TIMER_ID_MAX = LED_TIMER_ID + LED_TYPE_MAX - 1,

};

typedef struct
{
	KalmanFilter_Struct VbatFilter;
	HANDLE TaskID[TASK_ID_MAX];
	u32 Var[VAR_MAX];
	RMC_InfoStruct *RMCInfo;
	GSV_InfoStruct GSVInfo;
	GSV_InfoStruct GSVInfoSave;
	CFW_TSM_CURR_CELL_INFO CurrentCell;
	CFW_TSM_ALL_NEBCELL_INFO NearbyCell;
	CFW_SMS_PARAMETER SMSParam;
	Param_Byte64Struct nParam[PARAM_TYPE_MAX];
	IP_AddrUnion LocalIP;
	IP_AddrUnion DNS;
	u32 ErrorCRC32;
	Date_Union uDateSave;
	Time_Union uTimeSave;
	Monitor_CtrlStruct *Monitor;
	u8 IMEI[IMEI_LEN];
	u8 IMSI[IMSI_LEN];
	u8 ICCID[ICCID_LEN];
	u8 State[STATE_MAX];
	u8 Error[ERROR_MAX];
	u32 FlashBuf[FLASH_SECTOR_LEN / 4];
	u8 IMEIStr[20];
	RBuffer TraceBuf;
	u8 TraceData[16 * 1024];
}SysVar_Struct;

typedef struct
{
	u8 *ReceiveBuf;
	u8 GPRSUpgradeFlag;
	u8 DevUpgradeFlag;
	u8 AGPSFlag;
	u8 DevUpgradeOK;
	u8 ErrorCnt;
	u8 VoiceCode;
	u8 LEDCode;
	u8 PlayTime;
	RBuffer ReqList;
	COS_EVENT Event[16];
	TTS_CodeDataStruct TTSCodeData[TTS_CODE_MAX];
}User_CtrlStruct;

#define POWER2(x)	((x) * (x))
#define POWER3(x)	((x) * (x) * (x))
#define G_POWER		POWER2
extern SysVar_Struct __attribute__((section (".usr_ram"))) gSys;
void SYS_PowerStateBot(void);
void SYS_Error(u8 Sn, u8 Val);
void SYS_Reset(void);
void SYS_CheckTime(Date_UserDataStruct *Date, Time_UserDataStruct *Time);
void SYS_Waketup(void);
void SYS_Debug(const ascii *Fmt, ...);
void __HexTrace(u8 *Data, u32 Len);
void __DecTrace(u8 *Data, u8 Len);
void User_Config(void);

void User_AGPSStart(void);
void User_DevUpgradeStart(void);
void User_GPRSUpgradeStart(void);
void User_Req(u32 Param1, u32 Param2, u32 Param3);

//#define DBG(X, Y...) __Trace("%s %d:"X, __FUNCTION__, __LINE__, ##Y)
#define DBG(X, Y...) SYS_Debug("%s %d:"X, __FUNCTION__, __LINE__, ##Y)

#endif
