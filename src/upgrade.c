#include "user.h"
extern uint32_t gMainVersion;


typedef struct
{
	uint32_t FilePos;
	uint32_t FileLen;
	uint8_t *FileCache;
	uint8_t MemCache[4096];
}File_CtrlStruct;

Upgrade_FileStruct __attribute__((section (".file_ram"))) FileCache;
File_CtrlStruct __attribute__((section (".usr_ram"))) FileCtrl;
static uint8_t UpgradeFlag = 0;
void __Upgrade_Config(void)
{
	FileCtrl.FilePos = 0;
	FileCtrl.FileLen = 0;
	FileCtrl.FileCache = (uint8_t *)&FileCache;
	CORE("UPGRADE %08x", FileCtrl.FileCache);
}

void __FileSet(uint32_t Len)
{
	FileCtrl.FilePos = 0;
	FileCtrl.FileLen = Len;
}

uint8_t __WriteFile(uint8_t *Data, uint32_t Len)
{
	uint32_t i;
	uint32_t CRC32;
	if ( (FileCtrl.FilePos + Len) > FileCtrl.FileLen)
	{
		CORE("UPGRADE DL too much!");
		return 1;
	}
	CORE("UPGRADE %u %u %u", FileCtrl.FileLen, FileCtrl.FilePos, Len);
	memcpy(FileCtrl.FileCache + FileCtrl.FilePos, Data, Len);
	FileCtrl.FilePos += Len;
	if ( FileCtrl.FilePos == FileCtrl.FileLen )
	{
		CORE("UPGRADE DL OK!");
		CRC32 = __CRC32(&FileCache.SectionData[0].Data[0], FileCache.Head.BinFileLen * 4096, CRC32_START);
		if (CRC32 != FileCache.Head.CRC32)
		{
			FileCache.Head.MaigcNum = 0;
			CORE("UPGRADE File CRC32 error %x %x", CRC32, FileCache.Head.CRC32);
			return 1;
		}

		if (FileCache.Head.MainVersion != gMainVersion)
		{
			FileCache.Head.MaigcNum = 0;
			CORE("UPGRADE MainVersion error %x %x", gMainVersion, FileCache.Head.MainVersion);
			return 1;
		}

		for (i = 0;i < FileCache.Head.BinFileLen; i++)
		{
			__ReadFlash(USER_CODE_START + i * 4096, FileCtrl.MemCache, 4096);
			if (memcmp(&FileCache.SectionData[i].Data[0], FileCtrl.MemCache, 4096))
			{
				CORE("UPGRADE Section %u data diff!", i);
			}
			else
			{
				CORE("UPGRADE Section %u data same!", i);
			}
		}

	}

	return 0;
}

uint8_t __UpgradeVaildCheck(void)
{
	uint32_t CRC32;

	if (RDA_UPGRADE_MAGIC_NUM != FileCache.Head.MaigcNum)
	{
		FileCache.Head.MaigcNum = 0;
		CORE("UPGRADE File magic num error %x %x", RDA_UPGRADE_MAGIC_NUM, FileCache.Head.MaigcNum);
		return 0;
	}

	CRC32 = __CRC32(&FileCache.SectionData[0].Data[0], FileCache.Head.BinFileLen * 4096, CRC32_START);
	if (CRC32 != FileCache.Head.CRC32)
	{
		FileCache.Head.MaigcNum = 0;
		CORE("UPGRADE File CRC32 error %x %x", CRC32, FileCache.Head.CRC32);
		return 0;
	}

	if (FileCache.Head.MainVersion != gMainVersion)
	{
		FileCache.Head.MaigcNum = 0;
		CORE("UPGRADE MainVersion error %x %x", gMainVersion, FileCache.Head.MainVersion);
		return 0;
	}

	if (gSys.Var[SOFTWARE_VERSION] >= FileCache.Head.AppVersion)
	{
		FileCache.Head.MaigcNum = 0;
		CORE("UPGRADE SubVersion error %u %u", gSys.Var[SOFTWARE_VERSION], FileCache.Head.AppVersion);
		return 0;
	}

	return 1;
}

void __UpgradeRun(void)
{
	uint32_t i,j;
	uint32_t CRC32;
	int32_t Error;
	File_HeadStruct Head;
	uint8_t *SectorData = COS_MALLOC(4096);
	UpgradeFlag = 0;
	__ReadFlash(BACK_CODE_PARAM, (uint8_t *)&Head, sizeof(Head));
	switch (Head.MaigcNum)
	{
	case RDA_UPGRADE_MAGIC_NUM:
		CRC32 = __CRC32((uint8_t *)FLASH_BASE + BACK_CODE_START, Head.BinFileLen * 4096, CRC32_START);
		if (CRC32 != Head.CRC32)
		{
			CORE("UPGRADE File CRC32 error %x %x", CRC32, Head.CRC32);
			CORE("UPGRADE upgrade fail!");
			__EraseSector(BACK_CODE_PARAM);
			UpgradeFlag = RDA_UPGRADE_FAIL;
			FileCache.Head.MaigcNum = 0;
		}
		else
		{
			for(i = 0; i < Head.BinFileLen; i++)
			{
				__EraseSector(USER_CODE_START + i * 4096);
				__ReadFlash(BACK_CODE_START + i * 4096, SectorData, 4096);
				for(j = 0; j < 16; j++)
				{
					__WriteFlash(USER_CODE_START + i * 4096 + j * 256, SectorData + j * 256, 256);
				}
				Error = memcmp((uint8_t *)(FLASH_BASE + USER_CODE_START + i * 4096), (uint8_t *)(FLASH_BASE + BACK_CODE_START + i * 4096), 4096);
				if (Error)
				{
					DM_Reset();
				}
			}
			CRC32 = __CRC32((uint8_t *)FLASH_BASE + USER_CODE_START, Head.BinFileLen * 4096, CRC32_START);
			if (CRC32 != Head.CRC32)
			{
				DM_Reset();
			}
			CORE("UPGRADE upgrade done!");
			__EraseSector(BACK_CODE_PARAM);
			UpgradeFlag = RDA_UPGRADE_OK;
			FileCache.Head.MaigcNum = 0;
		}
		break;

	}

	switch (FileCache.Head.MaigcNum)
	{
	case RDA_UPGRADE_MAGIC_NUM:
		if (FileCache.Head.BinFileLen > 31)
		{
			CORE("UPGRADE File len error %u", FileCache.Head.BinFileLen);
			FileCache.Head.MaigcNum = 0;
			UpgradeFlag = RDA_UPGRADE_FAIL;
			__EraseSector(BACK_CODE_PARAM);
			break;
		}
		CRC32 = __CRC32(&FileCache.SectionData[0].Data[0], FileCache.Head.BinFileLen * 4096, CRC32_START);
		if (CRC32 != FileCache.Head.CRC32)
		{
			CORE("UPGRADE File CRC32 error %x %x", CRC32, FileCache.Head.CRC32);
			FileCache.Head.MaigcNum = 0;
			UpgradeFlag = RDA_UPGRADE_FAIL;
			__EraseSector(BACK_CODE_PARAM);
			break;

		}

		if (FileCache.Head.MainVersion != gMainVersion)
		{
			CORE("UPGRADE MainVersion error %x %x", gMainVersion, FileCache.Head.MainVersion);
			FileCache.Head.MaigcNum = 0;
			UpgradeFlag = RDA_UPGRADE_FAIL;
			__EraseSector(BACK_CODE_PARAM);
			break;
		}

		for(i = 0; i < FileCache.Head.BinFileLen; i++)
		{
			__EraseSector(BACK_CODE_START + i * 4096);

			for(j = 0; j < 16; j++)
			{
				__WriteFlash(BACK_CODE_START + i * 4096 + j * 256, (uint8_t *)&FileCache.SectionData[i].Data[0] + j * 256, 256);
			}
			Error = memcmp((uint8_t *)&FileCache.SectionData[i].Data[0], (uint8_t *)(FLASH_BASE + BACK_CODE_START + i * 4096), 4096);
			if (Error)
			{
				UpgradeFlag = RDA_UPGRADE_FAIL;
				__EraseSector(BACK_CODE_PARAM);
				FileCache.Head.MaigcNum = 0;
				CORE("UPGRADE upgrade fail!");
				break;
			}
		}
		CRC32 = __CRC32((uint8_t *)FLASH_BASE + BACK_CODE_START, FileCache.Head.BinFileLen * 4096, CRC32_START);
		if (CRC32 != FileCache.Head.CRC32)
		{
			CORE("UPGRADE upgrade fail!");
			UpgradeFlag = RDA_UPGRADE_FAIL;
			__EraseSector(BACK_CODE_PARAM);
			FileCache.Head.MaigcNum = 0;
			break;
		}
		__EraseSector(BACK_CODE_PARAM);
		__WriteFlash(BACK_CODE_PARAM, &FileCache.Head, sizeof(File_HeadStruct));
		DM_Reset();
		break;

	default:

		break;
	}
	COS_FREE(SectorData);
}

uint8_t __GetUpgradeState(void)
{
	return UpgradeFlag;
}

void __ClearUpgradeState(void)
{
	UpgradeFlag = 0;
}
