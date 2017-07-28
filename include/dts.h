#ifndef __DTS_H__
#define __DTS_H__

const GPIO_ParamStruct PinParam[PIN_MAX] =
{
#if (__BOARD__ == __AIR200__)
#if (__CUST_CODE__ == __CUST_LY__ || __CUST_CODE__ == __CUST_GLEAD__)
	//LED_NET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_15,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//LED_GPS_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_14,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//WDG_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_1,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//GPS_POWER_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_3
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//GSENSOR_POWER_PIN
	{
		.APO =
		{
				.gpoId = HAL_GPO_9,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//VCC_DET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_5,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 1,

	},
	//ACC_DET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_6,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 1,

	},
	//I2C_SDA_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_25,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//I2C_SCL_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_24,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//TEST_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_4,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,
	}
#elif (__CUST_CODE__ == __CUST_KQ__)
	//LED_NET_PIN BLE_UPGRADE_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_15,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//LED_GPS_PIN
	{
		.APO =
		{
				.gpioId = 0,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//WDG_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_14,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//GPS_POWER_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_3
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//GSENSOR_POWER_PIN
	{
		.APO =
		{
				.gpioId = 0,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//VCC_DET_PIN BLE_REBOOT_H_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_5,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 1,

	},
	//ACC_DET_PIN BLE_REBOOT_L_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_6,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 1,

	},
	//I2C_SDA_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_7,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//I2C_SCL_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_6,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//TEST_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_2,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	}
#endif
#endif
#if (__BOARD__ == __AIR201__)

#if (__CUST_CODE__ == __CUST_KQ__)
	//LED_NET_PIN BLE_UPGRADE_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_33,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//LED_GPS_PIN
	{
		.APO =
		{
				.gpioId = 0,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//WDG_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_31,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//GPS_POWER_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_29
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//GSENSOR_POWER_PIN
	{
		.APO =
		{
				.gpioId = 0,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//VCC_DET_PIN BLE_REBOOT_H_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_32,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 1,

	},
	//ACC_DET_PIN BLE_REBOOT_L_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_28,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 1,

	},
	//I2C_SDA_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_7,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//I2C_SCL_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_6,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//TEST_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_2,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	}
#elif (__CUST_CODE__ == __CUST_GLEAD__ || __CUST_CODE__ == __CUST_LY__)
	//LED_NET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_33,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//LED_GPS_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_31,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//WDG_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_30,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//GPS_POWER_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_29
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//GSENSOR_POWER_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_3,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//VCC_DET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_32,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 1,

	},
	//ACC_DET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_28,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 1,

	},
	//I2C_SDA_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_7,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//I2C_SCL_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_6,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//TEST_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_2,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,
	}
#elif (__CUST_CODE__ == __CUST_LB__)
	//LED_NET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_33,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//LED_GPS_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_2,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//WDG_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_30,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//GPS_POWER_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_29
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//GSENSOR_POWER_PIN USER_IO_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_3,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//VCC_DET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_31,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 1,

	},
	//ACC_DET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_28,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 1,

	},
	//I2C_SDA_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_7,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//I2C_SCL_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_6,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//TEST_PIN DIR_485_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_32,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	}
#endif
#endif

#if (__BOARD__ == __AIR202__)
#if (__CUST_CODE__ == __CUST_LY_IOTDEV__)

	//LED_NET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_33,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//LED_GPS_PIN
	{
		.APO =
		{
				.gpioId = 0,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//WDG_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_1,
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//GPS_POWER_PIN ADC_SELECT_0_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_29
		},
		.IsOut = 1,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//GSENSOR_POWER_PIN ADC_SELECT_1_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_31,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//VCC_DET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_6,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 1,

	},
	//ACC_DET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_6,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 1,

	},
	//I2C_SDA_PIN
	{
		.APO =
		{
				.gpioId = 0,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//I2C_SCL_PIN
	{
		.APO =
		{
				.gpioId = 0,
		},
		.IsOut = 1,
		.IsWork = 0,
		.InitValue = 0,
		.IsRevese = 0,

	},
	//TEST_PIN CRASH_DET_PIN
	{
		.APO =
		{
				.gpioId = HAL_GPIO_7,
		},
		.IsOut = 0,
		.IsWork = 1,
		.InitValue = 0,
		.IsRevese = 0,
	}
#endif
#endif
};


#endif
