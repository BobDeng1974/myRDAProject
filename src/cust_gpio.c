#include "user.h"
GPIO_ParamStruct __attribute__((section (".usr_ram"))) PinParam[PIN_MAX];
void GPIO_Config(void)
{
	u8 i;
	for (i = 0; i < PIN_MAX; i++)
	{
		PinParam[i].IsWork = 1;
		PinParam[i].IsOut = 1;
		PinParam[i].InitValue = 0;
		PinParam[i].IsRevese = 0;
	}
	//PinParam[LED_NET_PIN].IsWork = 0;
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_configRegs->GPIO_Mode |= (1 << 4);
	hwp_configRegs->Alt_mux_select |= CFG_REGS_TCO_4_GPO_9|CFG_REGS_PWL1_GPO_6|CFG_REGS_TCO_3_GPO_8;
#if (__CUST_CODE__ == __CUST_LY__)
	PinParam[LED_NET_PIN].APO.gpioId = HAL_GPIO_15;
	PinParam[LED_GPS_PIN].APO.gpioId = HAL_GPIO_14;
	PinParam[WDG_PIN].APO.gpioId = HAL_GPIO_1;
	PinParam[GPS_POWER_PIN].APO.gpioId = HAL_GPIO_3;
	PinParam[TEST_PIN].APO.gpioId = HAL_GPIO_4;
	PinParam[GSENSOR_POWER_PIN].APO.gpoId = HAL_GPO_9;
	PinParam[I2C_SDA_PIN].APO.gpioId = HAL_GPIO_25;
	PinParam[I2C_SCL_PIN].APO.gpoId = HAL_GPIO_24;
	PinParam[VCC_DET_PIN].APO.gpioId = HAL_GPIO_5;
	PinParam[VCC_DET_PIN].IsRevese = 1;
	PinParam[VCC_DET_PIN].IsOut = 0;
	PinParam[ACC_DET_PIN].APO.gpioId = HAL_GPIO_6;
	PinParam[ACC_DET_PIN].IsRevese = 1;
	PinParam[ACC_DET_PIN].IsOut = 0;
#elif (__CUST_CODE__ == __CUST_KQ__)
	PinParam[LED_NET_PIN].APO.gpioId = HAL_GPIO_15;
	PinParam[LED_BIT0_PIN].APO.gpioId = HAL_GPIO_1;
	PinParam[WDG_PIN].APO.gpioId = HAL_GPIO_14;

	PinParam[GPS_POWER_PIN].APO.gpioId = HAL_GPIO_3;
	PinParam[GSENSOR_POWER_PIN].APO.gpioId = HAL_GPIO_24;
	PinParam[GSENSOR_POWER_PIN].IsWork = 0;
	PinParam[TEST_PIN].APO.gpoId = HAL_GPO_9;
	PinParam[LED_BIT1_PIN].APO.gpioId = HAL_GPIO_25;
	PinParam[LED_BIT2_PIN].APO.gpoId = HAL_GPIO_24;
	PinParam[BLE_REBOOT_H_PIN].APO.gpioId = HAL_GPIO_5;
	PinParam[BLE_REBOOT_L_PIN].APO.gpioId = HAL_GPIO_6;
#elif (__CUST_CODE__ == __CUST_KKS__)
	PinParam[LED_NET_PIN].APO.gpioId = HAL_GPIO_15;
	PinParam[LED_GPS_PIN].APO.gpioId = HAL_GPIO_14;
	PinParam[LED_GPS_PIN].IsWork = 0;
	PinParam[WDG_PIN].APO.gpioId = HAL_GPIO_1;
	PinParam[GPS_POWER_PIN].APO.gpioId = HAL_GPIO_3;
	PinParam[TEST_PIN].APO.gpioId = HAL_GPIO_5;
	PinParam[GSENSOR_POWER_PIN].APO.gpoId = HAL_GPO_9;
	PinParam[I2C_SDA_PIN].APO.gpioId = HAL_GPIO_25;
	PinParam[I2C_SCL_PIN].APO.gpoId = HAL_GPIO_24;
	PinParam[VCC_DET_PIN].APO.gpioId = HAL_GPIO_14;
	PinParam[VCC_DET_PIN].IsRevese = 1;
	PinParam[VCC_DET_PIN].IsOut = 0;
	PinParam[ACC_DET_PIN].APO.gpioId = HAL_GPIO_6;
	PinParam[ACC_DET_PIN].IsRevese = 1;
	PinParam[ACC_DET_PIN].IsOut = 0;
#endif
#endif

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	hwp_iomux->pad_GPIO_2_cfg = IOMUX_PAD_GPIO_2_SEL_FUN_GPIO_2_SEL;
	hwp_iomux->pad_GPIO_3_cfg = IOMUX_PAD_GPIO_3_SEL_FUN_GPIO_3_SEL;
	hwp_iomux->pad_KEYIN_3_cfg = IOMUX_PAD_KEYIN_3_SEL_FUN_GPIO_28_SEL;
	hwp_iomux->pad_KEYIN_4_cfg = IOMUX_PAD_KEYIN_4_SEL_FUN_GPIO_29_SEL;
	hwp_iomux->pad_KEYOUT_0_cfg = IOMUX_PAD_KEYOUT_0_SEL_FUN_GPIO_30_SEL;
	hwp_iomux->pad_KEYOUT_1_cfg = IOMUX_PAD_KEYOUT_1_SEL_FUN_GPIO_31_SEL;
	hwp_iomux->pad_KEYOUT_2_cfg = IOMUX_PAD_KEYOUT_2_SEL_FUN_GPIO_32_SEL;
	hwp_iomux->pad_KEYOUT_3_cfg = IOMUX_PAD_KEYOUT_3_SEL_FUN_GPIO_33_SEL;
#if (__CUST_CODE__ == __CUST_LY__)
	PinParam[LED_NET_PIN].APO.gpioId = HAL_GPIO_33;
	PinParam[LED_GPS_PIN].APO.gpioId = HAL_GPIO_31;
	PinParam[WDG_PIN].APO.gpioId = HAL_GPIO_30;
	PinParam[GPS_POWER_PIN].APO.gpioId = HAL_GPIO_29;
	PinParam[TEST_PIN].APO.gpioId = HAL_GPIO_2;
	PinParam[GSENSOR_POWER_PIN].APO.gpioId = HAL_GPIO_3;
	PinParam[I2C_SDA_PIN].APO.gpioId = HAL_GPIO_7;
	PinParam[I2C_SCL_PIN].APO.gpioId = HAL_GPIO_6;
	PinParam[VCC_DET_PIN].APO.gpioId = HAL_GPIO_32;
	PinParam[VCC_DET_PIN].IsRevese = 1;
	PinParam[VCC_DET_PIN].IsOut = 0;
	PinParam[ACC_DET_PIN].APO.gpioId = HAL_GPIO_28;
	PinParam[ACC_DET_PIN].IsRevese = 1;
	PinParam[ACC_DET_PIN].IsOut = 0;
#elif (__CUST_CODE__ == __CUST_KQ__)
	PinParam[LED_NET_PIN].APO.gpioId = HAL_GPIO_33;
	PinParam[LED_BIT0_PIN].APO.gpioId = HAL_GPIO_30;
	PinParam[WDG_PIN].APO.gpioId = HAL_GPIO_31;

	PinParam[GPS_POWER_PIN].APO.gpioId = HAL_GPIO_29;
	PinParam[GSENSOR_POWER_PIN].APO.gpioId = HAL_GPIO_3;
	PinParam[GSENSOR_POWER_PIN].IsWork = 0;
	PinParam[TEST_PIN].APO.gpioId = HAL_GPIO_2;
	PinParam[LED_BIT1_PIN].APO.gpioId = HAL_GPIO_6;
	PinParam[LED_BIT2_PIN].APO.gpioId = HAL_GPIO_7;
	PinParam[BLE_REBOOT_H_PIN].APO.gpioId = HAL_GPIO_32;
	PinParam[BLE_REBOOT_L_PIN].APO.gpioId = HAL_GPIO_28;
#elif (__CUST_CODE__ == __CUST_GLEAD__)
	PinParam[LED_NET_PIN].APO.gpioId = HAL_GPIO_33;
	PinParam[LED_GPS_PIN].APO.gpioId = HAL_GPIO_31;
	PinParam[WDG_PIN].APO.gpioId = HAL_GPIO_30;
	PinParam[GPS_POWER_PIN].APO.gpioId = HAL_GPIO_29;
	PinParam[TEST_PIN].APO.gpioId = HAL_GPIO_2;
	PinParam[GSENSOR_POWER_PIN].APO.gpioId = HAL_GPIO_3;
	PinParam[I2C_SDA_PIN].APO.gpioId = HAL_GPIO_7;
	PinParam[I2C_SCL_PIN].APO.gpioId = HAL_GPIO_6;
	PinParam[VCC_DET_PIN].APO.gpioId = HAL_GPIO_32;
	PinParam[VCC_DET_PIN].IsRevese = 1;
	PinParam[VCC_DET_PIN].IsOut = 0;
	PinParam[ACC_DET_PIN].APO.gpioId = HAL_GPIO_28;
	PinParam[ACC_DET_PIN].IsRevese = 1;
	PinParam[ACC_DET_PIN].IsOut = 0;
#endif
#endif
	for (i = 0; i < PIN_MAX; i++)
	{
		GPIO_Init(&PinParam[i]);
	}
}

void GPIO_Init(GPIO_ParamStruct *Pin)
{
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	if (Pin->IsWork)
	{
		if (Pin->IsOut)
		{
			hwp_gpio->gpio_oen_set_out = GPIO_OEN_SET_OUT(1 << Pin->APO.id);
	        if(Pin->InitValue)
	        {
	            hwp_gpio->gpio_set = GPIO_GPIO_SET(1 << Pin->APO.id);
	        }
	        else
	        {
	            hwp_gpio->gpio_clr = GPIO_GPIO_CLR(1 << Pin->APO.id);
	        }
		}
		else
		{
			hwp_gpio->gpio_oen_set_in = GPIO_OEN_SET_IN(1 << Pin->APO.id);
		}
	}
#endif

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)

	if (Pin->IsWork)
	{
		if (Pin->IsOut)
		{
			if (Pin->APO.gpioId < HAL_GPIO_32)
			{
				hwp_gpio->gpio_oen_set_out_l = GPIO_OEN_SET_OUT_L(1 << Pin->APO.id);
			}
			else
			{
				hwp_gpio->gpio_oen_set_out_h = GPIO_OEN_SET_OUT_H(1 << (Pin->APO.id - 32) );
			}

	        if(Pin->InitValue)
	        {
				if (Pin->APO.gpioId < HAL_GPIO_32)
				{
					hwp_gpio->gpio_set_l = GPIO_GPIO_SET_L(1 << Pin->APO.id);
				}
				else
				{
					hwp_gpio->gpio_set_h = GPIO_GPIO_SET_H(1 << (Pin->APO.id - 32));
				}
	        }
	        else
	        {
				if (Pin->APO.gpioId < HAL_GPIO_32)
				{
					hwp_gpio->gpio_clr_l = GPIO_GPIO_CLR_L(1 << Pin->APO.id);
				}
				else
				{
					hwp_gpio->gpio_clr_h = GPIO_GPIO_CLR_H(1 << (Pin->APO.id - 32));
				}
	        }
		}
		else
		{
			if (Pin->APO.gpioId < HAL_GPIO_32)
			{
				hwp_gpio->gpio_oen_set_in_l = GPIO_OEN_SET_IN_L(1 << Pin->APO.id);
			}
			else
			{
				hwp_gpio->gpio_oen_set_in_h = GPIO_OEN_SET_IN_H(1 << (Pin->APO.id - 32));
			}
		}
	}
#endif
}

void GPIO_Write(u8 PinSn, u8 Value)
{
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	if (PinSn >= PIN_MAX)
	{
		DBG("!");
		return ;
	}
	if (PinParam[PinSn].IsWork && PinParam[PinSn].IsOut)
	{
		if (PinParam[PinSn].IsRevese)
		{
			if (Value)
			{
				if (PinParam[PinSn].APO.type == HAL_GPIO_TYPE_IO)
				{
					hwp_gpio->gpio_clr = 1 << PinParam[PinSn].APO.id;
				}
				else
				{
					hwp_gpio->gpo_clr = 1 << PinParam[PinSn].APO.id;
				}

			}
			else
			{
				if (PinParam[PinSn].APO.type == HAL_GPIO_TYPE_IO)
				{
					hwp_gpio->gpio_set = 1 << PinParam[PinSn].APO.id;
				}
				else
				{
					hwp_gpio->gpo_set = 1 << PinParam[PinSn].APO.id;
				}
			}
		}
		else
		{
			if (Value)
			{
				if (PinParam[PinSn].APO.type == HAL_GPIO_TYPE_IO)
				{
					hwp_gpio->gpio_set = 1 << PinParam[PinSn].APO.id;
				}
				else
				{
					hwp_gpio->gpo_set = 1 << PinParam[PinSn].APO.id;
				}
			}
			else
			{
				if (PinParam[PinSn].APO.type == HAL_GPIO_TYPE_IO)
				{
					hwp_gpio->gpio_clr = 1 << PinParam[PinSn].APO.id;
				}
				else
				{
					hwp_gpio->gpo_clr = 1 << PinParam[PinSn].APO.id;
				}
			}
		}
	}
#endif
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	if (PinSn >= PIN_MAX)
	{
		DBG("!");
		return ;
	}
	if (PinParam[PinSn].IsWork && PinParam[PinSn].IsOut)
	{
		if (PinParam[PinSn].IsRevese)
		{
			if (Value)
			{
				if (PinParam[PinSn].APO.gpioId < HAL_GPIO_32)
				{
					hwp_gpio->gpio_clr_l = GPIO_GPIO_CLR_L(1 << PinParam[PinSn].APO.id);
				}
				else
				{
					hwp_gpio->gpio_clr_h = GPIO_GPIO_CLR_H(1 << (PinParam[PinSn].APO.id - 32));
				}
			}
			else
			{
				if (PinParam[PinSn].APO.gpioId < HAL_GPIO_32)
				{
					hwp_gpio->gpio_set_l = GPIO_GPIO_SET_L(1 << PinParam[PinSn].APO.id);
				}
				else
				{
					hwp_gpio->gpio_set_h = GPIO_GPIO_SET_H(1 << (PinParam[PinSn].APO.id - 32));
				}
			}
		}
		else
		{
			if (Value)
			{
				if (PinParam[PinSn].APO.gpioId < HAL_GPIO_32)
				{
					hwp_gpio->gpio_set_l = GPIO_GPIO_SET_L(1 << PinParam[PinSn].APO.id);
				}
				else
				{
					hwp_gpio->gpio_set_h = GPIO_GPIO_SET_H(1 << (PinParam[PinSn].APO.id - 32));
				}
			}
			else
			{
				if (PinParam[PinSn].APO.gpioId < HAL_GPIO_32)
				{
					hwp_gpio->gpio_clr_l = GPIO_GPIO_CLR_L(1 << PinParam[PinSn].APO.id);
				}
				else
				{
					hwp_gpio->gpio_clr_h = GPIO_GPIO_CLR_H(1 << (PinParam[PinSn].APO.id - 32));
				}
			}
		}
	}
#endif
}

u8 GPIO_Read(u8 PinSn)
{
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	if (PinSn >= PIN_MAX)
	{
		DBG("!");
		return 0;
	}
	if (PinParam[PinSn].IsWork && !PinParam[PinSn].IsOut)
	{
		if (PinParam[PinSn].IsRevese)
		{
		    if (hwp_gpio->gpio_val & (1 << PinParam[PinSn].APO.id))
		    {
		        return 0;
		    }
		    else
		    {
		        return 1;
		    }
		}
		else
		{
		    if (hwp_gpio->gpio_val & (1 << PinParam[PinSn].APO.id))
		    {
		        return 1;
		    }
		    else
		    {
		        return 0;
		    }
		}
	}
#endif

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	u8 Val = 0;
	if (PinSn >= PIN_MAX)
	{
		DBG("!");
		return 0;
	}
	if (PinParam[PinSn].IsWork && !PinParam[PinSn].IsOut)
	{
		if(PinParam[PinSn].APO.gpioId < HAL_GPIO_32 )
		{
			if (hwp_gpio->gpio_val_l & (1 << PinParam[PinSn].APO.id))
			{
				Val = 1;
			}
			else
			{
				Val = 0;
			}
		}
		else
		{
			if (hwp_gpio->gpio_val_h & (1 << (PinParam[PinSn].APO.id -32)))
			{
				Val = 1;
			}
			else
			{
				Val = 0;
			}

		}

		if (PinParam[PinSn].IsRevese)
		{
			Val = !Val;
		}
		return Val;
	}
#endif
	return 0;
}

