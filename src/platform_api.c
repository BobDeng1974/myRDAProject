#include "platform_api.h"

typedef U64 u64;
extern UINT32 g_halLpsForceNoSleep;
UINT8 gACLBStatus = FALSE;
UINT32 g_htstVdsTask;
extern MMI_Default_Value g_MMI_Default_Value;
extern HANDLE g_hCosMmiTask; // the default MMI task.
extern HANDLE g_hCosATTask; // the default MMI task.
extern UINT32 g_memdFlashBaseAddress;
extern UINT32 g_spiflash_pagesize;
static u32 CRC32Table[256];
u32 gMainVersion;
typedef void(*MainFun)(void);

extern PUBLIC MEMD_ERR_T spi_flash_sector_erase_nosuspend(UINT32 flash_addr);
extern PUBLIC MEMD_ERR_T spi_flash_write(UINT32 flash_addr, UINT8 data_array[], UINT32 data_size);
extern VOID VDS_CacheTaskEntry(void *pData);
/**
* @brief  反转数据
* @param  ref 需要反转的变量
* @param	ch 反转长度，多少位
* @retval N反转后的数据
*/
static u64 Reflect(u64 ref, u8 ch)
{
	unsigned long long value = 0;
	u32 i;
	for (i = 1; i< (ch + 1); i++)
	{
		if (ref & 1)
			value |= (u64)1 << (ch - i);
		ref >>= 1;
	}
	return value;
}

/**
* @brief  建立CRC32的查询表
* @param  Tab 表缓冲
* @param	Gen CRC32根
* @retval None
*/
void CRC32_CreateTable(u32 *Tab, u32 Gen)
{
	u32 crc;
	u32 i, j, temp, T1, T2, flag;
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
* @brief  计算buffer的crc校验码
* @param  CRC32_Table CRC32表
* @param  Buf 缓冲
* @param	Size 缓冲区长度
* @param	CRC32 初始CRC32值
* @retval 计算后的CRC32
*/
u32 CRC32_Cal(u32 *CRC32_Table, u8 *Buf, u32 Size, u32 CRC32Last)
{
	u32 i;
	for (i = 0; i < Size; i++)
	{
		CRC32Last = CRC32_Table[(CRC32Last ^ Buf[i]) & 0xff] ^ (CRC32Last >> 8);
	}
	return CRC32Last;
}

void __AppInit(void)
{

	u32 piRevision;
	MainFun __Main;
	u8 *Addr = 0x82380000;
	gMainVersion = (__BASE_VERSION__ << 16)|(__CUST_CODE__)|(CHIP_ASIC_ID << 8);
	memset(Addr, 0, 0x10000);
	__SetDeepSleep(0);
	OS_APIInit();
	CRC32_CreateTable(CRC32Table, CRC32_GEN);
	__Upgrade_Config();
	__UpgradeRun();
#ifdef __TTS_ENABLE__
	__TTS_Init();
#endif
	__Trace("MainVersion %08x Start %x", gMainVersion, __MainInit);
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

u32 __GetMainVersion(void)
{
	return gMainVersion;
}
void __SetDefaultTaskHandle(HANDLE ID)
{
	g_hCosMmiTask = ID;
	g_hCosATTask = ID;
}

void __SetDeepSleep(u32 En)
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

s32 __EraseSector(u32 Addr)
{
	volatile u8 * ptr;
	s32 Error;
	UINT32 cri_status;
	ptr = (volatile u8 *)(g_memdFlashBaseAddress + Addr);
	cri_status = hal_SysEnterCriticalSection();
	Error = spi_flash_sector_erase_nosuspend((u32)ptr);
	hal_SysExitCriticalSection(cri_status);
	__Trace("Erase Flash %x", Error);
	return Error;
}

s32 __WriteFlash(u32 Addr, u8 *Data, u32 Len)
{
	volatile u8 * ptr;
	s32 Error;
	UINT32 cri_status;
	ptr = (volatile u8 *)(g_memdFlashBaseAddress + Addr);
	Len = (Len > 256)?256:Len;
	cri_status = hal_SysEnterCriticalSection();
	Error = spi_flash_write((u32)ptr, Data, Len);
	hal_SysExitCriticalSection(cri_status);
	Error = memcmp(Data, ptr, Len);
	if (Error)
	{
		__Trace("Write Flash %x", Error);
	}
	return Error;
}

void __ReadFlash(u32 Addr, u8 *Buf, u32 Len)
{
	volatile UINT8 * ptr;
    UINT32 cri_status;
    ptr = (VOLATILE UINT8 *)(g_memdFlashBaseAddress + Addr);
    cri_status = hal_SysEnterCriticalSection();
    //ptr = (VOLATILE UINT8 *)(g_memdFlashBaseAddress + Addr);
    /* could do aligned read from flash to improve bus accesses as it is uncached */
    memcpy(Buf, (UINT8 *)ptr, Len);
    hal_SysExitCriticalSection(cri_status);
}

void __Trace(const ascii *Fmt, ...)
{
    s8 uart_buf[512];
    s32 Len;
    va_list ap;
    va_start (ap, Fmt);
    Len = vsnprintf(uart_buf, sizeof(uart_buf), Fmt, ap);
    va_end (ap);
    sxs_fprintf(_MMI | TNB_ARG(0) | TSTDOUT, uart_buf);
}

u32 __CRC32(u8 *Buf, u32 Size, u32 CRC32Last)
{
	return CRC32_Cal(CRC32Table, Buf, Size, CRC32Last);
}
