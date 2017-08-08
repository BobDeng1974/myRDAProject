#ifndef __CUST_GPS_H__
#define __CUST_GPS_H__
#include "cs_types.h"
#include "CApi.h"
#define GPS_LEN_MAX (256)

#define GPS_DIFF_MODE	(0x10)
enum
{
	GN_GN_MODE,
	GN_GPS_MODE,
	GN_BD_MODE,
	GN_OTHER_MODE,
};

enum
{
	RMC_HEAD,
	RMC_TIME,
	RMC_STATUS,
	RMC_LAT,
	RMC_NS,
	RMC_LGT,
	RMC_EW,
	RMC_SPEED,
	RMC_COG,
	RMC_DATE,
	RMC_MV,
	RMC_MVE,
	RMC_LOCAT_MODE,
	RMC_SECTOR_MAX,

	GSV_HEAD = 0,
	GSV_MSG_NUM,
	GSV_MSG_NO,
	GSV_SATE_NUM,
	GSV_PRN1,
	GSV_ELV1,
	GSV_AZ1,
	GSV_CN1,
	GSV_PRN2,
	GSV_ELV2,
	GSV_AZ2,
	GSV_CN2,
	GSV_PRN3,
	GSV_ELV3,
	GSV_AZ3,
	GSV_CN3,
	GSV_PRN4,
	GSV_ELV4,
	GSV_AZ4,
	GSV_CN4,
	GSV_SECTOR_MAX,
};



//推荐定位信息RMC解析结构体
typedef struct
{
	Date_UserDataStruct UTCDate;//UTC时间
	Time_UserDataStruct UTCTime;
	uint32_t HighLevel;
	uint32_t LatDegree;//纬度
	uint32_t LatMin;//*10000
	uint32_t LgtDegree;//经度
	uint32_t LgtMin;//*10000
    uint32_t Speed;//字段7:地面速率（0~999.9999节）*1000
    uint32_t Cog;//字段8:地面航向（0 ~359.999 度，以真北为参考基准，前面的0也将被传输） *1000
    int8_t LocatStatus;//字段2:定位状态，1=有效定位，0=无效定位
    int8_t LatNS;
    int8_t LgtEW;
    uint8_t LocatMode;
}RMC_InfoStruct;

typedef struct
{
	Date_Union uDate;
	Time_Union uTime;
	uint32_t Lat;
	uint32_t Lgt;
}LBS_LocatInfoStruct;

typedef struct
{
    uint8_t PRN[2][48];
    uint8_t CN[2][48];
    uint8_t Pos[2];
    uint8_t IsVaild[2];
}GSV_InfoStruct;

#define GPS_HIGH_CN		(44)
#define GPS_NORMAL_CN	(40)
#define GPS_LOW_CN		(35)
#define GPS_BASE_CN		(28)

enum gps_status_sn
{
	GPS_STOP,					//GPS断电
    GPS_V_STAGE,				//GPS未精确定位工作阶段
    GPS_A_STAGE,				//GPS精度定位工作阶段

};


void GPS_Receive(void *pData, uint8_t Data);
double GPS_Distance(double lat1, double lat2, double lgt1, double lgt2);
void GPS_Analyze(int8_t *Data, uint32_t len);
void GPS_StateCheck(void);
void GPS_Config(void);
void GPS_RemotePrint(void);
void GPS_Sleep(void);
void WGS84ToGCJ02(double wglat, double wglgt, double *china_lat, double *china_lgt);
#endif
