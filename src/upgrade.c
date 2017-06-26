#include "user.h"
extern u32 gMainVersion;


typedef struct
{
	u32 FilePos;
	u32 FileLen;
	u8 *FileCache;
	u8 MemCache[4096];
}File_CtrlStruct;

Update_FileStruct __attribute__((section (".file_ram"))) FileCache;
File_CtrlStruct __attribute__((section (".usr_ram"))) FileCtrl;
static u8 UpgradeFlag = 0;
void __Upgrade_Config(void)
{
	FileCtrl.FilePos = 0;
	FileCtrl.FileLen = 0;
	FileCtrl.FileCache = (u8 *)&FileCache;
	__Trace("UPGRADE %08x", FileCtrl.FileCache);
}

void __FileSet(u32 Len)
{
	FileCtrl.FilePos = 0;
	FileCtrl.FileLen = Len;
}

u8 __WriteFile(u8 *Data, u32 Len)
{
	u32 i;
	u32 CRC32;
	if ( (FileCtrl.FilePos + Len) > FileCtrl.FileLen)
	{
		__Trace("UPGRADE DL too much!");
		return 1;
	}
	__Trace("UPGRADE %d %d %d", FileCtrl.FileLen, FileCtrl.FilePos, Len);
	memcpy(FileCtrl.FileCache + FileCtrl.FilePos, Data, Len);
	FileCtrl.FilePos += Len;
	if ( FileCtrl.FilePos == FileCtrl.FileLen )
	{
		__Trace("UPGRADE DL OK!");
		CRC32 = __CRC32(&FileCache.SectionData[0].Data[0], FileCache.Head.BinFileLen * 4096, CRC32_START);
		if (CRC32 != FileCache.Head.CRC32)
		{
			FileCache.Head.MaigcNum = 0;
			__Trace("UPGRADE File CRC32 error %x %x", CRC32, FileCache.Head.CRC32);
			return 1;
		}

		if (FileCache.Head.MainVersion != gMainVersion)
		{
			FileCache.Head.MaigcNum = 0;
			__Trace("UPGRADE MainVersion error %d %d", gMainVersion, FileCache.Head.MainVersion);
			return 1;
		}

		for (i = 0;i < FileCache.Head.BinFileLen; i++)
		{
			__ReadFlash(USER_CODE_START + i * 4096, FileCtrl.MemCache, 4096);
			if (memcmp(&FileCache.SectionData[i].Data[0], FileCtrl.MemCache, 4096))
			{
				__Trace("UPGRADE Section %d data diff!", i);
			}
			else
			{
				__Trace("UPGRADE Section %d data same!", i);
			}
		}

	}

	return 0;
}

u8 __UpgradeVaildCheck(void)
{
	u32 CRC32;

	if (RDA_UPGRADE_MAGIC_NUM != FileCache.Head.MaigcNum)
	{
		FileCache.Head.MaigcNum = 0;
		__Trace("UPGRADE File magic num error %x %x", RDA_UPGRADE_MAGIC_NUM, FileCache.Head.MaigcNum);
		return 0;
	}

	CRC32 = __CRC32(&FileCache.SectionData[0].Data[0], FileCache.Head.BinFileLen * 4096, CRC32_START);
	if (CRC32 != FileCache.Head.CRC32)
	{
		FileCache.Head.MaigcNum = 0;
		__Trace("UPGRADE File CRC32 error %x %x", CRC32, FileCache.Head.CRC32);
		return 0;
	}

	if (FileCache.Head.MainVersion != gMainVersion)
	{
		FileCache.Head.MaigcNum = 0;
		__Trace("UPGRADE MainVersion error %d %d", gMainVersion, FileCache.Head.MainVersion);
		return 0;
	}

	if (gSys.Var[SOFTWARE_VERSION] >= FileCache.Head.UpdateVersion)
	{
		FileCache.Head.MaigcNum = 0;
		__Trace("UPGRADE SubVersion error %d %d", gSys.Var[SOFTWARE_VERSION], FileCache.Head.UpdateVersion);
		return 0;
	}

	return 1;
}

void __UpgradeRun(void)
{
	u32 i,j;
	u32 CRC32;
	int Error;
	File_HeadStruct Head;
	u8 *SectorData = COS_MALLOC(4096);
	UpgradeFlag = 0;
	__ReadFlash(BACK_CODE_PARAM, (u8 *)&Head, sizeof(Head));
	switch (Head.MaigcNum)
	{
	case RDA_UPGRADE_MAGIC_NUM:
		CRC32 = __CRC32((u8 *)FLASH_BASE + BACK_CODE_START, Head.BinFileLen * 4096, CRC32_START);
		if (CRC32 != Head.CRC32)
		{
			__Trace("UPGRADE File CRC32 error %x %x", CRC32, Head.CRC32);
			__Trace("UPGRADE upgrade fail!");
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
				Error = memcmp(FLASH_BASE + USER_CODE_START + i * 4096, FLASH_BASE + BACK_CODE_START + i * 4096, 4096);
				if (Error)
				{
					DM_Reset();
				}
			}
			CRC32 = __CRC32((u8 *)FLASH_BASE + USER_CODE_START, Head.BinFileLen * 4096, CRC32_START);
			if (CRC32 != Head.CRC32)
			{
				DM_Reset();
			}
			__Trace("UPGRADE upgrade done!");
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
			__Trace("UPGRADE File len error %d", FileCache.Head.BinFileLen);
			FileCache.Head.MaigcNum = 0;
			UpgradeFlag = RDA_UPGRADE_FAIL;
			__EraseSector(BACK_CODE_PARAM);
			break;
		}
		CRC32 = __CRC32(&FileCache.SectionData[0].Data[0], FileCache.Head.BinFileLen * 4096, CRC32_START);
		if (CRC32 != FileCache.Head.CRC32)
		{
			__Trace("UPGRADE File CRC32 error %x %x", CRC32, FileCache.Head.CRC32);
			FileCache.Head.MaigcNum = 0;
			UpgradeFlag = RDA_UPGRADE_FAIL;
			__EraseSector(BACK_CODE_PARAM);
			break;

		}

		if (FileCache.Head.MainVersion != gMainVersion)
		{
			__Trace("UPGRADE MainVersion error %d %d", gMainVersion, FileCache.Head.MainVersion);
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
				__WriteFlash(BACK_CODE_START + i * 4096 + j * 256, (u8 *)&FileCache.SectionData[i].Data[0] + j * 256, 256);
			}
			Error = memcmp((u8 *)&FileCache.SectionData[i].Data[0], FLASH_BASE + BACK_CODE_START + i * 4096, 4096);
			if (Error)
			{
				UpgradeFlag = RDA_UPGRADE_FAIL;
				__EraseSector(BACK_CODE_PARAM);
				FileCache.Head.MaigcNum = 0;
				__Trace("UPGRADE upgrade fail!");
				break;
			}
		}
		CRC32 = __CRC32((u8 *)FLASH_BASE + BACK_CODE_START, FileCache.Head.BinFileLen * 4096, CRC32_START);
		if (CRC32 != FileCache.Head.CRC32)
		{
			__Trace("UPGRADE upgrade fail!");
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

u8 __GetUpgradeState(void)
{
	return UpgradeFlag;
}

void __ClearUpgradeState(void)
{
	UpgradeFlag = 0;
}
