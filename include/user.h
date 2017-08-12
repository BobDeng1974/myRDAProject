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
#define __G_SENSOR_ENABLE__
#elif (__CUST_CODE__ == __CUST_LY_IOTDEV__)
#define __CUST_IP_ADDR1__		(0)
#define __CUST_IP_ADDR2__ 		(0)
#define __CUST_IP_ADDR3__ 		(0)
#define __CUST_IP_ADDR4__		(0)
#define __CUST_URL__			"auth.lyiot.top"
#define __CUST_TCP_PORT__		(5006)
#define __CUST_UDP_PORT__		(0)
#define __AD_ENABLE__
#define __CRASH_ENABLE__
#define __UART_AUTO_SLEEP_BY_RUN__
#undef __BOARD__
#undef __TTS_ENABLE__
#define __BOARD__		__AIR202__
#define __NO_GPS__
#elif (__CUST_CODE__ == __CUST_KQ__)
#define __CUST_IP_ADDR1__		(115)
#define __CUST_IP_ADDR2__		(231)
#define __CUST_IP_ADDR3__		(73)
#define __CUST_IP_ADDR4__		(229)
#define __CUST_URL__			""
#define __CUST_TCP_PORT__		(18888)
#define __CUST_UDP_PORT__		(0)
#define __TTS_ENABLE__
#elif (__CUST_CODE__ == __CUST_GLEAD__ || __CUST_CODE__ == __CUST_NONE__)
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

#if (__CUST_CODE__ == __CUST_NONE__)
#define __MINI_SYSTEM__
#define __NO_GPS__
#else
#define __G_SENSOR_ENABLE__
#define __UART_AUTO_SLEEP_BY_RUN__
#endif

#elif (__CUST_CODE__ == __CUST_LB__)
#define __CUST_IP_ADDR1__ (221)
#define __CUST_IP_ADDR2__ (6)
#define __CUST_IP_ADDR3__ (104)
#define __CUST_IP_ADDR4__ (178)
#define __CUST_URL__		""
#define __CUST_TCP_PORT__ (7911)
#define __CUST_UDP_PORT__ (0)

//#define __AD_ENABLE__
#define __G_SENSOR_ENABLE__
#define __UART_AUTO_SLEEP_BY_VACC__
#define __UART_485_MODE__

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
#include "led.h"
#include "ntp.h"
#include "mqtt.h"
#include "remote.h"
#include "luat.h"
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
	EV_MMI_REBOOT = EV_MMI_EV_BASE + 1,
	EV_MMI_GET_RTC_ENABLE,
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
	EV_MMI_COM_485_DONE,
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
	EV_MMI_START_LBS,
};

enum TASK_ID_ENUM
{
	//ÈÎÎñ¾ä±ú
	MAIN_TASK_ID,
	GPS_TASK_ID,
	MONITOR_TASK_ID,
	FTP_CTRL_TASK_ID,
	FTP_DATA_TASK_ID,
	COM_TASK_ID,
	USER_TASK_ID,
	REMOTE_TASK_ID,
	NTP_TASK_ID,
	LUAT_TASK_ID,
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
	NO_LOCAT_ERROR,
	ERROR_MAX,
};

enum SYS_VAR_ENUM
{
	SYS_TIME,
	VBAT,
	ADC0_VAL,
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
	G_SENSOR_TIMER_ID,
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
	LUAT_TIMER_ID,
	LED_TIMER_ID,
	LED_TIMER_ID_MAX = LED_TIMER_ID + LED_TYPE_MAX - 1,

};

enum PORT_ENUM
{
	UDP_NTP_PORT = 10000,
	UDP_LUAT_PORT,
};

typedef struct
{
	HANDLE TaskID[TASK_ID_MAX];
	uint32_t Var[VAR_MAX];
	RMC_InfoStruct *RMCInfo;
	GSV_InfoStruct GSVInfo;
	GSV_InfoStruct GSVInfoSave;
	CFW_TSM_CURR_CELL_INFO CurrentCell;
	CFW_TSM_ALL_NEBCELL_INFO NearbyCell;
	LBS_InfoStruct LBSInfo;
	CFW_SMS_PARAMETER SMSParam;
	Param_Byte64Struct nParam[PARAM_TYPE_MAX];
	IP_AddrUnion LocalIP;
	IP_AddrUnion DNS;
	uint32_t ErrorCRC32;
	Date_Union uDateSave;
	Time_Union uTimeSave;
	Monitor_RecordCollectStruct RecordCollect;
	LBS_LocatInfoStruct LBSLocat;
	uint8_t IMEI[IMEI_LEN];
	uint8_t IMSI[IMSI_LEN];
	uint8_t ICCID[ICCID_LEN];
	uint8_t State[STATE_MAX];
	uint8_t Error[ERROR_MAX];
	uint32_t FlashBuf[FLASH_SECTOR_LEN / 4];
	RBuffer TraceBuf;
	uint8_t TraceData[8 * 1024];
}SysVar_Struct;

typedef struct
{
	uint8_t *ReceiveBuf;
	uint8_t GPRSUpgradeFlag;
	uint8_t DevUpgradeFlag;
	uint8_t AGPSFlag;
	uint8_t DevUpgradeOK;
	uint8_t ErrorCnt;
	uint8_t VoiceCode;
	uint8_t LEDCode;
	uint8_t PlayTime;
	uint8_t FTPDone;
	uint32_t FTPResult;
	RBuffer ReqList;
	COS_EVENT Event[16];
	TTS_CodeDataStruct TTSCodeData[TTS_CODE_MAX];
}User_CtrlStruct;

#define POWER2(x)	((x) * (x))
#define POWER3(x)	((x) * (x) * (x))
#define __MXC622X__	0
#define __LIS3DH__	1
#define __G_SENSOR__ __MXC622X__
#if (__G_SENSOR__ == __MXC622X__)
#define G_POWER		POWER2
#define G_SENSOR_READFIRST	MXC622X_ReadFirst
#define G_SENSOR_READ	MXC622X_Read
#endif
#if (__G_SENSOR__ == __LIS3DH__)
#define G_POWER		POWER2
#define G_SENSOR_READFIRST	LIS3DH_ReadFirst
#define G_SENSOR_READ	LIS3DH_Read
#endif

extern SysVar_Struct __attribute__((section (".usr_ram"))) gSys;
void SYS_PowerStateBot(void);
void SYS_Error(uint8_t Sn, uint8_t Val);
void SYS_Reset(void);
void SYS_CheckTime(Date_UserDataStruct *Date, Time_UserDataStruct *Time);
void SYS_Waketup(void);
void SYS_Debug(const int8_t *Fmt, ...);
void HexTrace(uint8_t *Data, uint32_t Len);
void DecTrace(uint8_t *Data, uint8_t Len);
void User_Config(void);

void User_AGPSStart(void);
void User_DevUpgradeStart(void);
void User_GPRSUpgradeStart(void);
void User_Req(uint32_t Param1, uint32_t Param2, uint32_t Param3);

//#define DBG(X, Y...) __Trace("%s %d:"X, __FUNCTION__, __LINE__, ##Y)
#define DBG(X, Y...) SYS_Debug("%s %d:"X, __FUNCTION__, __LINE__, ##Y)

#endif
