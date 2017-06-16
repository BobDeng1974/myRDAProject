#ifndef __CUST_IO_H__
#define __CUST_IO_H__
#include "platform_api.h"
typedef struct
{
	HAL_APO_ID_T APO;
	u8 IsOut;
	u8 IsWork;
	u8 InitValue;
	u8 IsRevese;
}GPIO_ParamStruct;

enum
{
	//GPIO∑÷≈‰
	PIN_START = 0,
	LED_NET_PIN = PIN_START,
	LED_GPS_PIN,
	WDG_PIN,
	GPS_POWER_PIN,
	GSENSOR_POWER_PIN,
	TEST_PIN,
	I2C_SDA_PIN,
	I2C_SCL_PIN,
	VCC_DET_PIN,
	ACC_DET_PIN,
	PIN_MAX,
#if (__CUST_CODE__ == __CUST_KQ__)
	BLE_UPGRADE_PIN = LED_NET_PIN,
	BLE_REBOOT_H_PIN = VCC_DET_PIN,
	BLE_REBOOT_L_PIN = ACC_DET_PIN,
	LED_BIT0_PIN = LED_GPS_PIN,
	LED_BIT1_PIN = I2C_SDA_PIN,
	LED_BIT2_PIN = I2C_SCL_PIN,
	LP_PA_POWER = GPS_POWER_PIN,
#endif

};

void GPIO_Config(void);
void GPIO_Init(GPIO_ParamStruct *);
void GPIO_Write(u8 PinSn, u8 Value);
u8 GPIO_Read(u8 PinSn);
#endif
