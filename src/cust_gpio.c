#include "user.h"
#include "dts.h"
void GPIO_Config(void)
{
	uint8_t i;

#if (__BOARD__ == __AIR200__)
	hwp_configRegs->GPIO_Mode |= (1 << 4);
	hwp_configRegs->Alt_mux_select |= CFG_REGS_TCO_4_GPO_9|CFG_REGS_PWL1_GPO_6|CFG_REGS_TCO_3_GPO_8;
#endif

#if (__BOARD__ == __AIR201__)
	hwp_iomux->pad_GPIO_2_cfg = IOMUX_PAD_GPIO_2_SEL_FUN_GPIO_2_SEL;
	hwp_iomux->pad_GPIO_3_cfg = IOMUX_PAD_GPIO_3_SEL_FUN_GPIO_3_SEL;
	hwp_iomux->pad_KEYIN_3_cfg = IOMUX_PAD_KEYIN_3_SEL_FUN_GPIO_28_SEL;
	hwp_iomux->pad_KEYIN_4_cfg = IOMUX_PAD_KEYIN_4_SEL_FUN_GPIO_29_SEL;
	hwp_iomux->pad_KEYOUT_0_cfg = IOMUX_PAD_KEYOUT_0_SEL_FUN_GPIO_30_SEL;
	hwp_iomux->pad_KEYOUT_1_cfg = IOMUX_PAD_KEYOUT_1_SEL_FUN_GPIO_31_SEL;
	hwp_iomux->pad_KEYOUT_2_cfg = IOMUX_PAD_KEYOUT_2_SEL_FUN_GPIO_32_SEL;
	hwp_iomux->pad_KEYOUT_3_cfg = IOMUX_PAD_KEYOUT_3_SEL_FUN_GPIO_33_SEL;

#endif
#if (__BOARD__ == __AIR202__)
	hwp_iomux->pad_GPIO_2_cfg = IOMUX_PAD_GPIO_2_SEL_FUN_GPIO_2_SEL;
	hwp_iomux->pad_GPIO_3_cfg = IOMUX_PAD_GPIO_3_SEL_FUN_GPIO_3_SEL;
	hwp_iomux->pad_SDMMC_CLK_cfg = IOMUX_PAD_SDMMC_CLK_SEL_FUN_GPIO_8_SEL;
	hwp_iomux->pad_SDMMC_DATA_0_cfg = IOMUX_PAD_SDMMC_DATA_0_SEL_FUN_GPIO_10_SEL;
	hwp_iomux->pad_SDMMC_DATA_1_cfg = IOMUX_PAD_SDMMC_DATA_1_SEL_FUN_GPIO_11_SEL;
	hwp_iomux->pad_SDMMC_DATA_2_cfg = IOMUX_PAD_SDMMC_DATA_2_SEL_FUN_GPIO_12_SEL;
	hwp_iomux->pad_KEYIN_4_cfg = IOMUX_PAD_KEYIN_4_SEL_FUN_GPIO_29_SEL;
	hwp_iomux->pad_KEYOUT_0_cfg = IOMUX_PAD_KEYOUT_0_SEL_FUN_GPIO_30_SEL;
	hwp_iomux->pad_KEYOUT_1_cfg = IOMUX_PAD_KEYOUT_1_SEL_FUN_GPIO_31_SEL;
	hwp_iomux->pad_KEYOUT_3_cfg = IOMUX_PAD_KEYOUT_3_SEL_FUN_GPIO_33_SEL;
#endif

	for (i = 0; i < PIN_MAX; i++)
	{
		GPIO_Init(&PinParam[i]);
	}
}

void GPIO_Init(const GPIO_ParamStruct *Pin)
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

void GPIO_Write(uint8_t PinSn, uint8_t Value)
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
			Value = !Value;
		}

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
			Value = !Value;
		}

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
#endif
}

uint8_t GPIO_Read(uint8_t PinSn)
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
	uint8_t Val = 0;
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

