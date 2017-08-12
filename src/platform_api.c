#include "platform_api.h"

extern UINT32 g_halLpsForceNoSleep;
UINT8 gACLBStatus = FALSE;
UINT32 g_htstVdsTask;
extern MMI_Default_Value g_MMI_Default_Value;
extern HANDLE g_hCosMmiTask; // the default MMI task.
extern HANDLE g_hCosATTask; // the default MMI task.
extern UINT32 g_memdFlashBaseAddress;
extern UINT32 g_spiflash_pagesize;
static uint32_t CRC32Table[256];
uint32_t gMainVersion;
typedef void(*MainFun)(void);
extern void OS_APIInit(void);
extern PUBLIC MEMD_ERR_T spi_flash_sector_erase_nosuspend(UINT32 flash_addr);
extern PUBLIC MEMD_ERR_T spi_flash_write(UINT32 flash_addr, UINT8 data_array[], UINT32 data_size);
extern VOID VDS_CacheTaskEntry(void *pData);
/**
* @brief  ��ת����
* @param  ref ��Ҫ��ת�ı���
* @param	ch ��ת���ȣ�����λ
* @retval N��ת�������
*/
static uint64_t Reflect(uint64_t ref, uint8_t ch)
{
	unsigned long long value = 0;
	uint32_t i;
	for (i = 1; i< (ch + 1); i++)
	{
		if (ref & 1)
			value |= (uint64_t)1 << (ch - i);
		ref >>= 1;
	}
	return value;
}

/**
* @brief  ����CRC32�Ĳ�ѯ��
* @param  Tab ����
* @param	Gen CRC32��
* @retval None
*/
void CRC32_CreateTable(uint32_t *Tab, uint32_t Gen)
{
	uint32_t crc;
	uint32_t i, j, temp, T1, T2, flag;
	if (Tab[1] != 0)
		return;
	for (i = 0; i < 256; i++)
	{
		temp = Reflect(i, 8);
		Tab[i] = temp << 24;
		for (j = 0; j < 8; j++)
		{
			flag = Tab[i] & 0x80000000;
			T1 = Tab[i] << 1;
			if (0 == flag)
			{
				T2 = 0;
			}
			else
			{
				T2 = Gen;
			}
			Tab[i] = T1 ^ T2;
		}
		crc = Tab[i];
		Tab[i] = Reflect(crc, 32);
	}
}


/**
* @brief  ����buffer��crcУ����
* @param  CRC32_Table CRC32��
* @param  Buf ����
* @param	Size ����������
* @param	CRC32 ��ʼCRC32ֵ
* @retval ������CRC32
*/
uint32_t CRC32_Cal(uint32_t *CRC32_Table, uint8_t *Buf, uint32_t Size, uint32_t CRC32Last)
{
	uint32_t i;
	for (i = 0; i < Size; i++)
	{
		CRC32Last = CRC32_Table[(CRC32Last ^ Buf[i]) & 0xff] ^ (CRC32Last >> 8);
	}
	return CRC32Last;
}

void __AppInit(void)
{
	MainFun __Main;
	uint8_t *Addr = (uint8_t *)0x82380000;
	gMainVersion = (__BASE_VERSION__ << 16)|(__CUST_CODE__)|(CHIP_ASIC_ID << 8);
	TS_Open(TRUE);
	TS_Close();
	memset(Addr, 0, 0x10000);
	__SetDeepSleep(0);
	OS_APIInit();
	CRC32_CreateTable(CRC32Table, CRC32_GEN);
	__Upgrade_Config();
	__UpgradeRun();
#ifdef __TTS_ENABLE__
	__TTS_Init();
#endif
	CORE("Chip %08x Build %02x", hwp_configRegs->CHIP_ID, hwp_configRegs->Build_Version);
	CORE("MainVersion %08x Start %x", __GetMainVersion(), __MainInit);
	__Main = (MainFun)(USER_CODE_START + FLASH_BASE + 1);
	__Main();
#ifdef __VDS_QUICK_FLUSH__
    g_htstVdsTask = COS_CreateTask(VDS_CacheTaskEntry,
                                   NULL, NULL,
                                   1024,
								   COS_MMI_TASKS_PRIORITY_BASE + COS_PRIORITY_NUM  - 2,
                                   COS_CREATE_DEFAULT, 0, "VDS Flush Task");
    ASSERT(g_htstVdsTask != HNULL);
#endif
}

BOOL __GetACLBStatus()
{
    return gACLBStatus;
}

UINT32 __SetACLBStatus(UINT8 nACLBNWStatus)
{
    gACLBStatus = nACLBNWStatus;
    return ERR_SUCCESS;
}

void __SetMMIDefaultValue(void)
{
    g_MMI_Default_Value.nMinVol     = 3200;
    g_MMI_Default_Value.nMemorySize = 50 * 1024;
}

uint32_t __GetMainVersion(void)
{
	return gMainVersion;
}
void __SetDefaultTaskHandle(HANDLE ID)
{
	g_hCosMmiTask = ID;
	g_hCosATTask = ID;
}

void __SetDeepSleep(uint32_t En)
{
#ifdef CHIP_XTAL_CLK_IS_52M
	if (En)
	{
		hal_SysRequestFreq((HAL_SYS_FREQ_USER_ID_T)(HAL_SYS_FREQ_APP_USER_0 + CSW_LP_RESOURCE_UNUSED_1), (HAL_SYS_FREQ_T)CSW_SYS_FREQ_32K, NULL);
	}
	else
	{
		hal_SysRequestFreq((HAL_SYS_FREQ_USER_ID_T)(HAL_SYS_FREQ_APP_USER_0 + CSW_LP_RESOURCE_UNUSED_1), (HAL_SYS_FREQ_T)CSW_SYS_FREQ_104M, NULL);
	}
#endif
}

int32_t __EraseSector(uint32_t Addr)
{
	volatile uint8_t * ptr;
	int32_t Error;
	UINT32 cri_status;
	ptr = (volatile uint8_t *)(g_memdFlashBaseAddress + Addr);
	cri_status = hal_SysEnterCriticalSection();
	Error = spi_flash_sector_erase_nosuspend((uint32_t)ptr);
	hal_SysExitCriticalSection(cri_status);
	CORE("Erase Flash %x", Error);
	return Error;
}

int32_t __WriteFlash(uint32_t Addr, void *Src, uint32_t Len)
{
	volatile uint8_t *ptr;
	int32_t Error;
	UINT32 cri_status;
	uint8_t *Data = (uint8_t *)Src;
	ptr = (volatile uint8_t *)(g_memdFlashBaseAddress + Addr);
	Len = (Len > 256)?256:Len;
	cri_status = hal_SysEnterCriticalSection();
	Error = spi_flash_write((uint32_t)ptr, Data, Len);
	hal_SysExitCriticalSection(cri_status);
	Error = memcmp(Data, (uint8_t *)ptr, Len);
	if (Error)
	{
		CORE("Write Flash %x", Error);
	}
	return Error;
}

void __ReadFlash(uint32_t Addr, void *Dst, uint32_t Len)
{
	volatile uint8_t * ptr;
    UINT32 cri_status;
    ptr = (VOLATILE UINT8 *)(g_memdFlashBaseAddress + Addr);
    cri_status = hal_SysEnterCriticalSection();
    //ptr = (VOLATILE UINT8 *)(g_memdFlashBaseAddress + Addr);
    /* could do aligned read from flash to improve bus accesses as it is uncached */
    memcpy(Dst, (UINT8 *)ptr, Len);
    hal_SysExitCriticalSection(cri_status);
}

void __Trace(const ascii *Fmt, ...)
{
    int8_t uart_buf[512];
    int32_t Len;
    va_list ap;
    va_start (ap, Fmt);
    Len = vsnprintf(uart_buf, sizeof(uart_buf), Fmt, ap);
    va_end (ap);
    sxs_fprintf(_MMI | TNB_ARG(0) | TSTDOUT, uart_buf);
}

void __HexTrace(uint8_t *Data, uint32_t Len)
{
    char *uart_buf = COS_MALLOC(Len * 3 + 2);
    uint32_t i,j, Temp;
    j = 0;
    if (!uart_buf)
    	return;
    for (i = 0; i < Len; i++)
    {
    	Temp = Data[i] >> 4;
    	if (Temp < 10 )
    	{
    		uart_buf[j++] = Temp + '0';
    	}
    	else
    	{
    		uart_buf[j++] = Temp + 'A' - 10;
    	}
    	Temp = Data[i] & 0x0f;
    	if (Temp < 10 )
    	{
    		uart_buf[j++] = Temp + '0';
    	}
    	else
    	{
    		uart_buf[j++] = Temp + 'A' - 10;
    	}
    	uart_buf[j++] = ' ';
    }
    uart_buf[j++] = 0;
    sxs_fprintf(_MMI | TNB_ARG(0) | TSTDOUT, uart_buf);
    COS_FREE(uart_buf);
}

void __DecTrace(uint8_t *Data, uint8_t Len)
{
	char *uart_buf = COS_MALLOC(Len * 4 + 2);
    uint8_t i,j, Temp;
    j = 0;
    if (!uart_buf)
    	return ;
    for (i = 0; i < Len; i++)
    {
    	Temp = Data[i] / 100;
    	uart_buf[j++] = Temp + '0';
    	Temp = (Data[i] % 100) / 10;
    	uart_buf[j++] = Temp + '0';
    	Temp = (Data[i] % 10);
    	uart_buf[j++] = Temp + '0';
    	uart_buf[j++] = ' ';
    }
    uart_buf[j++] = 0;
    sxs_fprintf(_MMI | TNB_ARG(0) | TSTDOUT, uart_buf);
    COS_FREE(uart_buf);
}

uint32_t __CRC32(void *Src, uint32_t Size, uint32_t CRC32Last)
{
	uint8_t *Buf = (uint8_t *)Src;
	return CRC32_Cal(CRC32Table, Buf, Size, CRC32Last);
}
