#ifndef __CUST_IO_H__
#define __CUST_IO_H__
#include "platform_api.h"
typedef struct
{
	HAL_APO_ID_T APO;
	uint8_t IsOut;
	uint8_t IsWork;
	uint8_t InitValue;
	uint8_t IsRevese;
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
	VCC_DET_PIN,
	ACC_DET_PIN,
	I2C_SDA_PIN,
	I2C_SCL_PIN,
	TEST_PIN,
	PFG_PIN,
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
#if (__CUST_CODE__ == __CUST_LB_V2__ || __CUST_CODE__ == __CUST_LB_V3__)
	DIR_485_PIN = TEST_PIN,
	USER_IO_PIN = GSENSOR_POWER_PIN,
#endif
#if (__CUST_CODE__ == __CUST_LY_IOTDEV__)
	ADC_SELECT_0_PIN = GPS_POWER_PIN,
	ADC_SELECT_1_PIN = GSENSOR_POWER_PIN,
	CRASH_DET_PIN = TEST_PIN,
#endif
};

void GPIO_Config(void);
void GPIO_Init(const GPIO_ParamStruct *);
void GPIO_Write(uint8_t PinSn, uint8_t Value);
uint8_t GPIO_Read(uint8_t PinSn);
#endif
