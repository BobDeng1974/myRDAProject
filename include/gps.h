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



//�Ƽ���λ��ϢRMC�����ṹ��
typedef struct
{
	Date_UserDataStruct UTCDate;//UTCʱ��
	Time_UserDataStruct UTCTime;
	uint32_t HighLevel;
	uint32_t LatDegree;//γ��
	uint32_t LatMin;//*10000
	uint32_t LgtDegree;//����
	uint32_t LgtMin;//*10000
    uint32_t Speed;//�ֶ�7:�������ʣ�0~999.9999�ڣ�*1000
    uint32_t Cog;//�ֶ�8:���溽��0 ~359.999 �ȣ����汱Ϊ�ο���׼��ǰ���0Ҳ�������䣩 *1000
    int8_t LocatStatus;//�ֶ�2:��λ״̬��1=��Ч��λ��0=��Ч��λ
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
	GPS_STOP,					//GPS�ϵ�
    GPS_V_STAGE,				//GPSδ��ȷ��λ�����׶�
    GPS_A_STAGE,				//GPS���ȶ�λ�����׶�

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
