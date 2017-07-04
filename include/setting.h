#ifndef __CUST_SETTING_H__
#define __CUST_SETTING_H__

enum
{
	PARAM_DETECT_PERIOD,
	PARAM_SENSOR_EN,
	PARAM_COM_BR,
	PARAM_GPS_BR,
	PARAM_STOP_VBAT,
	PARAM_LOW_VBAT,
	PARAM_NORMAL_VBAT,

	PARAM_SMS_ALARM,			//���ű���ʹ��
	PARAM_CALL_AUTO_GET,		//�Զ�����ʹ��
	PARAM_SIM_TO,				//SIM��ʶ��ʱ
	PARAM_GPRS_TO,				//GPRS�������賬ʱ�Ļ�����Ĭ��60,
	PARAM_SYS_MAX,

	PARAM_GS_WAKEUP_GPS = 0,
	PARAM_VACC_WAKEUP_GPS,
	PARAM_GPS_NODATA_TO,		//������ʱ��
	PARAM_GPS_V_TO,				//��Ч��λʱ��
	PARAM_GPS_KEEP_TO,			//GPS��������ʱ��
	PARAM_GPS_SLEEP_TO,			//GPS����ʱ��
	PARAM_AGPS_EN,
	PARAM_GPS_ONLY_ONCE,		//GPS��λ�͹رգ�ֱ�����߽���
	PARAM_GPS_MAX,

	PARAM_GS_WAKEUP_MONITOR = 0,	//�𶯻���ͨѶ�����Ϊ0����ΪVACC����
	PARAM_GS_JUDGE_RUN,		//��ȷ������
	PARAM_UPLOAD_RUN_PERIOD,		//���лش�����
	PARAM_UPLOAD_STOP_PERIOD,		//ͣ���ش�����
	PARAM_UPLOAD_HEART_PERIOD,	//��������
	PARAM_MONITOR_NET_TO,
	PARAM_MONITOR_KEEP_TO,
	PARAM_MONITOR_SLEEP_TO,
	PARAM_MONITOR_RECONNECT_MAX,
	PARAM_MONITOR_ADD_MILEAGE,
	PARAM_MONITOR_ACC_UPLOAD,	//ACC�仯�ϴ�
	PARAM_MONITOR_MAX,

	PARAM_VACC_CTRL_ALARM = 0,
	PARAM_ALARM_ON_DELAY,		//ACC�����𶯺��ƶ���������ʱ
	PARAM_CRASH_GS,				//����ֵ
	PARAM_CRASH_JUDGE_TO,		//�𶯱����ж�ʱ�䴰
	PARAM_CRASH_JUDGE_CNT,		//�𶯱����ж�����
	PARAM_CRASH_ALARM_WAIT_TO,
	PARAM_CRASH_ALARM_FLUSH_TO,
	PARAM_CRASH_ALARM_REPEAT,
	PARAM_CRASH_WAKEUP_MOVE,	//�ƶ�����ǰ�Ƿ��������
	PARAM_MOVE_RANGE,
	PARAM_MOVE_JUDGE_TO,
	PARAM_MOVE_ALARM_FLUSH_TO,
	PARAM_MOVE_ALARM_REPEAT,		//�ƶ������Ƿ���������
	PARAM_ALARM1_MAX,

	PARAM_VACC_WAKEUP_CUTLINE_TO = 0,
	PARAM_CUTLINE_ALARM_DELAY,
	PARAM_CUTLINE_ALARM_FLUSH_TO,	//���߱���ˢ��ʱ�䣬���Ϊ0������������
	PARAM_OVERSPEED_ALARM_VAL,		//���ٱ�����ֵ�����Ϊ0���򲻼��
	PARAM_OVERSPEED_ALARM_DELAY,	//���ٱ����ӳ٣����Ϊ0��һ���������̱���
	PARAM_ALARM_ENABLE,			//������뱨������
	PARAM_ALARM2_MAX,

	PARAM_TYPE_MAIN = 0,
	PARAM_TYPE_SYS,
	PARAM_TYPE_GPS,
	PARAM_TYPE_MONITOR,
	PARAM_TYPE_ALARM1,
	PARAM_TYPE_ALARM2,
	PARAM_TYPE_APN,
	PARAM_TYPE_LOCAT,
	PARAM_TYPE_UPGRADE,
	PARAM_TYPE_NUMBER,
	PARAM_TYPE_USER,
	PARAM_TYPE_MAX,
};

typedef struct
{
	u32 UID[3];
	u32 MainIP;
	u16 TCPPort;	//���Ϊ0����ʾʹ��UDP
	u16 UDPPort;	//���Ϊ0����ʾʹ��TCP
	u16 CustCode;
	s8 MainURL[URL_LEN_MAX - 10];
}Param_MainStruct;

typedef struct
{
	s8 APNName[20];
	s8 APNUser[20];
	s8 APNPassword[20];
}Param_APNStruct;

typedef struct
{
	s8 Url[URL_LEN_MAX - 12];
	s8 Usr[12];
	s8 Pwd[12];
}Param_FtpStruct;

typedef struct
{
	u8 Num[8];
}Phone_NumberStruct;

typedef struct
{
	Phone_NumberStruct Phone[7];
}Param_NumberStruct;

typedef struct
{
	u32 Param[15];
}Param_DWStruct;

typedef struct
{

	u32 UpgradeIP;
	s8 UpgradeURL[URL_LEN_MAX];
	u16 TCPPort;	//���Ϊ0����ʾʹ��UDP
	u16 UDPPort;	//���Ϊ0����ʾʹ��TCP
}Param_UpgradeStruct;

typedef union
{
	Param_FtpStruct Ftp;
	Param_UpgradeStruct Upgrade;
	u8 pad[60];
}Param_UpgradeUnion;

typedef struct
{
	u32 LYIP;
	u16 LYTCPPort;
	u16 LYUDPPort;
}Param_LYStruct;

typedef struct
{
	u8 AuthCode[AUTH_CODE_LEN];
	u16 ProvinceID;
	u16 CityID;
	u8 PlateID;
	u8 AuthCodeLen;
}Param_KQStruct;

typedef union
{
	Param_LYStruct LY;
	Param_KQStruct KQ;
}Param_UserStruct;

typedef struct
{
	RMC_InfoStruct RMCSave;
	u32 MileageKM;
	u32 MileageM;
}Param_LocatStruct;

typedef union
{
	Param_MainStruct MainInfo;
	Param_APNStruct APN;
	Param_UpgradeUnion Upgrade;
	Param_DWStruct ParamDW;
	Param_UserStruct UserInfo;
	Param_NumberStruct Number;
	Param_LocatStruct LocatInfo;
	u8 pad[60];
}Param_Byte60Union;

typedef struct
{
	Param_Byte60Union Data;
	u32 CRC32;
}Param_Byte64Struct;


void Param_Config(void);
s32 Param_Save(u8 Type);
s32 Param_Format(u8 Type);
void Locat_CacheSave(void);
#endif
