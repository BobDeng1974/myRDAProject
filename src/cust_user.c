#include "user.h"
//#define __TTS_TEST__
#define BLE_REBOOT_TIME	(16)

User_CtrlStruct __attribute__((section (".usr_ram"))) UserCtrl;
extern Update_FileStruct __attribute__((section (".file_ram"))) FileCache;
s32 User_TTSCb(void *pData)
{
	DBG("TTS Done!");
}

s32 User_PCMCb(void *pData)
{
	DBG("PCM Done!");
	UserCtrl.PlayTime++;
	if ( (UserCtrl.PlayTime < UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Repeat) || (0xff == UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Repeat) )
	{
		if (UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Interval)
		{
			OS_StartTimer(gSys.TaskID[USER_TASK_ID], TTS_TIMER_ID, COS_TIMER_MODE_SINGLE, UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Interval * SYS_TICK);
		}
		else
		{
			OS_StartTimer(gSys.TaskID[USER_TASK_ID], TTS_TIMER_ID, COS_TIMER_MODE_SINGLE, SYS_TICK / 64);
		}
		DBG("play %d", UserCtrl.PlayTime);
	}
	else
	{
#ifndef __TTS_TEST__
		UserCtrl.VoiceCode = 0xff;
#endif
	}
#ifdef __TTS_TEST__
	UserCtrl.VoiceCode++;
	if (UserCtrl.VoiceCode < TTS_TEST_CODE_MAX)
	{

	}
	else
	{
		UserCtrl.VoiceCode = 1;
	}
	OS_StartTimer(gSys.TaskID[USER_TASK_ID], TTS_TIMER_ID, COS_TIMER_MODE_SINGLE, SYS_TICK);
#endif
}



#define CUST_BUF_LEN	(256)

#if (__CUST_CODE__ == __CUST_KQ__)
extern Monitor_CtrlStruct __attribute__((section (".usr_ram"))) KQCtrl;
#elif (__CUST_CODE__ == __CUST_LY__)
extern Monitor_CtrlStruct __attribute__((section (".usr_ram"))) LYCtrl;
#else
#endif
void User_DevDeal(u32 nParam1, u32 nParam2, u32 nParam3, s32 *Result)
{
#if (__CUST_CODE__ == __CUST_KQ__)
	u8 Buf[128];
	u32 TxLen;
	if (nParam1)				//需要发送用户的UART协议
	{
		TxLen = KQ_ComTxPack(nParam1, (u8 *)nParam2, nParam3, Buf);
		DBG("!");
		__HexTrace(Buf, TxLen);
		COM_TxReq(Buf, TxLen);
	}
	else							//需要解析用户的UART协议
	{
		TxLen = KQ_ComAnalyze((u8 *)nParam2, nParam3, Buf, CUST_BUF_LEN, Result);
		if (TxLen)
		{
			DBG("!");
			__HexTrace(Buf, TxLen);
			COM_TxReq(Buf, TxLen);
		}

	}
#elif (__CUST_CODE__ == __CUST_LY__)
	if (nParam1)
	{


	}
	else
	{
		DBG("uart rx %d", nParam3);
		__HexTrace((u8 *)nParam2, nParam3);
		LY_ComAnalyze((u8 *)nParam2, nParam3, Result);

	}
#elif (__CUST_CODE__ == __CUST_KKS__)
	DBG("uart rx %d", nParam3);
	__HexTrace((u8 *)nParam2, nParam3);
	KKS_DirSendTx((u8 *)nParam2, nParam3);
#endif
}


s32 User_WaitUartReceive(uint32 To)
{
	s32 Result;
	COS_EVENT Event;
	OS_StartTimer(gSys.TaskID[USER_TASK_ID], CUST_TIMER_ID, COS_TIMER_MODE_SINGLE, To * SYS_TICK);
	UserCtrl.ReceiveBuf = 0;
#if (__CUST_CODE__ == __CUST_KQ__)
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
#endif
	while (1)
	{
		COS_WaitEvent(gSys.TaskID[USER_TASK_ID], &Event, COS_WAIT_FOREVER);
		switch (Event.nEventId)
		{
    	case EV_TIMER:
    		switch (Event.nParam1)
    		{
    		case CUST_TIMER_ID:
    			DBG("To!");
    			return -1;
    			break;
    		case BLE_TIMER_ID:
#if (__CUST_CODE__ == __CUST_KQ__)
    			DBG("ble shutdown gprs");
    			if ( (KQ->UploadInfo[2] == 0) && (KQ->UploadInfo[3] == 5) )
    			{
    				OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_PERIODIC, 10 * SYS_TICK);
    			}
    			else if ( gSys.Var[SYS_TIME] >= gSys.Var[SHUTDOWN_TIME] )
				{
					User_DevDeal(KQ_CMD_SYS_DOWN, 0, 0, &Result);
					GPIO_Write(WDG_PIN, 0);
					OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_PERIODIC, 10 * SYS_TICK);
				}
#endif
				break;
    		case TTS_TIMER_ID:
    			if (UserCtrl.VoiceCode < TTS_CODE_MAX)
    			{
    				DBG("play code %d", UserCtrl.VoiceCode);
    				__TTS_Play(UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Data, UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Len, User_PCMCb, User_TTSCb);
    			}
    			break;
    		}
    		break;
		case EV_MMI_COM_TO_USER:
			if (!Event.nParam1)
			{
				UserCtrl.ReceiveBuf = (u8 *)Event.nParam2;
				return Event.nParam3;
			}
    		break;
		}
	}
}

#if (__CUST_CODE__ == __CUST_KQ__)
void BLE_Upgrade(void)
{
	u8 Buf[80];
	u8 Retry;
	u8 AllRetry;
	u8 SendOK;
	u8 AllSendOK;
	u32 AddrPos;
	u16 AddrIndex;
	u8 BLEStatus;
	u8 *BinBuf;
	s32 Result;
	u32 TxLen;

	GPIO_Write(BLE_UPGRADE_PIN, 1);
	GPIO_Write(BLE_REBOOT_H_PIN, 1);
	GPIO_Write(BLE_REBOOT_L_PIN, 0);
	OS_Sleep(SYS_TICK/BLE_REBOOT_TIME);
	GPIO_Write(BLE_REBOOT_H_PIN, 0);
	GPIO_Write(BLE_REBOOT_L_PIN, 1);
	OS_Sleep(SYS_TICK/BLE_REBOOT_TIME);
	//发送握手包
	TxLen = CC2541_UpgradeTx(CC2541_UPGRADE_CMD_SHAKEHAND, Buf, NULL, NULL);
	COM_TxReq(Buf, TxLen);
	Result = User_WaitUartReceive(1);
	if (Result < 0)
	{
		DBG("receive data error %d", Result);
		goto BLE_ERROR;
	}
	if (Result > 72)
	{
		DBG("too much data %d", Result);
		goto BLE_ERROR;
	}
	__HexTrace(UserCtrl.ReceiveBuf, Result);
	BLEStatus = CC2541_UpgradeRx(UserCtrl.ReceiveBuf, Result, NULL);
	if (BLEStatus)
	{
		DBG("shake hand fail %02x", BLEStatus);
		goto BLE_ERROR;
	}
	BinBuf = (u8 *)&FileCache.SectionData[0].Data[0];

	//发送数据包

	AllSendOK = 1;
	AddrIndex = 0;
	for (AddrPos = 0; AddrPos < CC2541_UPGRADE_BIN_LEN; AddrPos += CC2541_UPGRADE_DATA_PACK_LEN)
	{

		SendOK = 0;
		for (Retry = 0; Retry < 2; Retry++)
		{
			DBG("index %d pos %d", AddrIndex, AddrPos);

			TxLen = CC2541_UpgradeTx(CC2541_UPGRADE_CMD_WRITE, Buf, BinBuf + AddrPos, AddrIndex);
			COM_TxReq(Buf, TxLen);
			Result = User_WaitUartReceive(1);
			if (Result < 0)
			{
				DBG("receive data error %d", Result);
				continue;
			}
			if (Result > 80)
			{
				DBG("too much data %d", Result);
				OS_Sleep(SYS_TICK/32);
				continue;
			}
			__HexTrace(UserCtrl.ReceiveBuf, 6);
			BLEStatus = CC2541_UpgradeRx(UserCtrl.ReceiveBuf, Result, NULL);
			if (BLEStatus)
			{
				DBG("send data fail %02x", BLEStatus);
				OS_Sleep(SYS_TICK/32);
				continue;
			}
			OS_Sleep(SYS_TICK/64);
			TxLen = CC2541_UpgradeTx(CC2541_UPGRADE_CMD_READ, Buf, NULL, AddrIndex);
			COM_TxReq(Buf, TxLen);
			Result = User_WaitUartReceive(1);
			if (Result < 0)
			{
				DBG("receive data error %d", Result);
				continue;
			}
			if (Result > 80)
			{
				DBG("too much data %d", Result);
				OS_Sleep(SYS_TICK/32);
				continue;
			}
			__HexTrace(UserCtrl.ReceiveBuf, 6);
			BLEStatus = CC2541_UpgradeRx(UserCtrl.ReceiveBuf, Result, BinBuf + AddrPos);
			if (BLEStatus)
			{
				DBG("read data fail %02x", BLEStatus);
				OS_Sleep(SYS_TICK/32);
				continue;
			}
			SendOK = 1;
			break;
		}

		if (!SendOK && (AddrIndex < (0xF00 * 0x10)))
		{
			AllSendOK = 0;
			break;
		}
		AddrIndex += 0x10;
	}

	if (AllSendOK )
	{
		TxLen = CC2541_UpgradeTx(CC2541_UPGRADE_CMD_CHECK, Buf, NULL, NULL);
		COM_TxReq(Buf, TxLen);
		Result = User_WaitUartReceive(1);
		if (Result < 0)
		{
			DBG("receive data error %d", Result);
			goto BLE_ERROR;
		}
		if (Result > 80)
		{
			DBG("too much data %d", Result);
			goto BLE_ERROR;
		}
		__HexTrace(UserCtrl.ReceiveBuf, Result);
		BLEStatus = CC2541_UpgradeRx(UserCtrl.ReceiveBuf, Result, NULL);
		if (BLEStatus)
		{
			DBG("check data fail %02x", BLEStatus);
			goto BLE_ERROR;
		}
		DBG("Confirm OK!");
		DBG("BLE UpgradeOK!");
		UserCtrl.DevUpgradeOK = 1;
	}

BLE_ERROR:
	GPIO_Write(BLE_UPGRADE_PIN, 0);
	GPIO_Write(BLE_REBOOT_H_PIN, 1);
	GPIO_Write(BLE_REBOOT_L_PIN, 0);
	OS_Sleep(SYS_TICK/BLE_REBOOT_TIME);
	GPIO_Write(BLE_REBOOT_H_PIN, 0);
	GPIO_Write(BLE_REBOOT_L_PIN, 1);
	OS_Sleep(SYS_TICK/BLE_REBOOT_TIME);
	//GPIO_Write(WDG_PIN, 0);

}
#endif

void User_ReqRun(void)
{
	s32 Result;
	u32 RxLen;
	COS_EVENT Event;
	u8 RunOK;
	u32 TxLen;

#if (__CUST_CODE__ == __CUST_KQ__)
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	RunOK = 1;
	while (UserCtrl.ReqList.Len)
	{
		ReadRBuffer(&UserCtrl.ReqList, &Event, 1);
		User_DevDeal(Event.nParam1, Event.nParam2, Event.nParam3, &Result);
		Result = User_WaitUartReceive(2);
		if (Result < 0)
		{
			DBG("receive data error %d", Result);
			RunOK = 0;
			if (Event.nParam1 == KQ_CMD_DOWNLOAD_BT)
			{
				UserCtrl.DevUpgradeFlag = 0;
				KQ->UpgradeType = 1;
				KQ->UpgradeResult = 1;
				TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
				Monitor_RecordResponse(Monitor->TempBuf, TxLen);
				Monitor_Wakeup();
				OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_SINGLE, 100 * SYS_TICK);
			}
			else if (Event.nParam1 == KQ_CMD_DOWNLOAD_GPRS)
			{

				UserCtrl.GPRSUpgradeFlag = 0;
				KQ->UpgradeType = 0;
				KQ->UpgradeResult = 1;
				TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
				Monitor_RecordResponse(Monitor->TempBuf, TxLen);
				Monitor_Wakeup();
				OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_SINGLE, 100 * SYS_TICK);
			}
			continue;
		}
		RxLen = Result;
		__HexTrace(UserCtrl.ReceiveBuf, RxLen);
		User_DevDeal(0, (u32)UserCtrl.ReceiveBuf, RxLen, &Result);
		if (Result)
		{
			DBG("error %d", Result);
			RunOK = 0;
		}
	}
	if (KQ->WaitFlag)
	{
		KQ->IsWaitOk = RunOK;
		DBG("%d %d", KQ->WaitFlag, KQ->IsWaitOk);
		Monitor_Wakeup();
	}
#elif (__CUST_CODE__ == __CUST_LY__)
	LY_CustDataStruct *LY = (LY_CustDataStruct *)LYCtrl.CustData;
	while (UserCtrl.ReqList.Len)
	{
		ReadRBuffer(&UserCtrl.ReqList, &Event, 1);
		switch (Event.nParam1)
		{
		case LY_USER_TO_ECU:
			DBG("To ECU %d", LY->ToECUBuf.Pos);
			__HexTrace(LY->ToECUBuf.Data, LY->ToECUBuf.Pos);
			COM_TxReq(LY->ToECUBuf.Data, LY->ToECUBuf.Pos);
			if (Event.nParam2)
			{
				Result = User_WaitUartReceive(Event.nParam2);
				if (Result < 0)
				{
					DBG("receive data error %d", Result);
					TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 1, LY_RS232TC_VERSION, NULL, 0);
					Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
				}
				else
				{
					RxLen = Result;
					User_DevDeal(0, (u32)UserCtrl.ReceiveBuf, RxLen, &Result);
					if (Result)
					{
						DBG("receive data error %d", Result);
						TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 1, LY_RS232TC_VERSION, NULL, 0);
						Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
					}
				}
			}
			break;

		default:
			break;
		}
	}

#endif
}

void User_Task(void *pData)
{
	s32 Result;
	u32 TxLen;
	COS_EVENT Event;
//	CFW_DIALNUMBER sDailNumber;
//	UINT8 iBcdLen;
	UINT32 iRet;
//	UINT8 uDialNum[22] = { 0, };
//	UINT8 uParaDialNum[22] = { 0, };
//	UINT16 uBCDlen;
	UINT8 uIndex = 0;
	UINT16 uLen = 22;
	UINT8 uOpType;
	UINT8 iLen;
//	UINT32 uSimErr; // [changyg,new, for LD operation
//	CFW_SIM_PBK_ENTRY_INFO sEntryToBeAdded = { 0 }; // [changyg,new, for LD operation
	u8 Retry;

#if (__CUST_CODE__ == __CUST_KQ__)
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_PERIODIC, 100 * SYS_TICK);
#endif
#ifdef __TTS_TEST__
	OS_StartTimer(gSys.TaskID[USER_TASK_ID], TTS_TIMER_ID, COS_TIMER_MODE_SINGLE, SYS_TICK / 256);
#endif
//	strcpy(KQ->FTPCmd, "ftp://www.bdclw.net/ble.bin:21@gleadftp:glead123");
//	User_Req(KQ_CMD_DOWNLOAD_BT, 0, 0);
//	OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_USER_REQ, 0, 0, 0);

//	strcpy(KQ->FTPCmd, "ftp://www.bdclw.net/gl.bin:21@gleadftp:glead123");
//	User_Req(KQ_CMD_DOWNLOAD_GPRS, 0, 0);
//	OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_USER_REQ, 0, 0, 0);
	while (1)
	{
		COS_WaitEvent(gSys.TaskID[USER_TASK_ID], &Event, COS_WAIT_FOREVER);
		switch (Event.nEventId)
		{
    	case EV_TIMER:
    		switch (Event.nParam1)
    		{
    		case BLE_TIMER_ID:
#if (__CUST_CODE__ == __CUST_KQ__)
    			DBG("ble shutdown gprs");
    			if ( (KQ->UploadInfo[2] == 0) && (KQ->UploadInfo[3] == 5) )
    			{
    				OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_PERIODIC, 10 * SYS_TICK);
    			}
    			else if ( gSys.Var[SYS_TIME] >= gSys.Var[SHUTDOWN_TIME] )
				{
					User_DevDeal(KQ_CMD_SYS_DOWN, 0, 0, &Result);
					GPIO_Write(WDG_PIN, 0);
					OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_PERIODIC, 10 * SYS_TICK);
				}
#endif
				break;
    		case TTS_TIMER_ID:
    			//DM_PlayTone(DM_TONE_DTMF_1, DM_TONE_m3dB, 200, DM_TONE_m15dB);
    			if (UserCtrl.VoiceCode < TTS_CODE_MAX)
    			{
    				__HexTrace(UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Data, UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Len);
    				__TTS_Play(UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Data, UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Len, User_PCMCb, User_TTSCb);
    			}
    			else
    			{
    				DBG("%d",UserCtrl.VoiceCode);
    			}
    			break;
    		default:
    			OS_StopTimer(gSys.TaskID[USER_TASK_ID], Event.nParam1);
    			break;
    		}
    		break;
		case EV_MMI_COM_TO_USER:
			User_DevDeal(Event.nParam1, Event.nParam2, Event.nParam3, &Result);
    		break;
		case EV_MMI_USER_REQ:
			User_ReqRun();
			break;
		case EV_MMI_FTP_FINISH:
			if (Event.nParam1 == FTP_FINISH_OK)
			{
				if (UserCtrl.GPRSUpgradeFlag)
				{
#if (__CUST_CODE__ == __CUST_KQ__)
					User_DevDeal(KQ_CMD_UPGRADE_GPRS, 0, 0, &Result);
					OS_Sleep(1 * SYS_TICK);
					SYS_Reset();
#else
					if (__UpgradeVaildCheck())
					{
						SYS_Reset();
					}
#endif
				}
				else if(UserCtrl.DevUpgradeFlag)
				{
#if (__CUST_CODE__ == __CUST_KQ__)
					KQ->BLEUpgradeStart = 1;
					OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_SINGLE, 600 * SYS_TICK);
#endif
				}
				else if (UserCtrl.AGPSFlag)
				{
					OS_SendEvent(gSys.TaskID[GPS_TASK_ID], EV_MMI_AGPS_FILE_OK, 0, 0, 0);
				}
			}
			else
			{
				if (UserCtrl.DevUpgradeFlag)
				{
					UserCtrl.DevUpgradeFlag = 0;
#if (__CUST_CODE__ == __CUST_KQ__)
					KQ->UpgradeType = 1;
					KQ->UpgradeResult = 1;
					TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
					Monitor_RecordResponse(Monitor->TempBuf, TxLen);

					Monitor_Wakeup();
					OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_SINGLE, 100 * SYS_TICK);
#endif
				}
				else if (UserCtrl.GPRSUpgradeFlag)
				{
					UserCtrl.GPRSUpgradeFlag = 0;
#if (__CUST_CODE__ == __CUST_KQ__)
					KQ->UpgradeType = 0;
					KQ->UpgradeResult = 1;
					TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
					Monitor_RecordResponse(Monitor->TempBuf, TxLen);
					Monitor_Wakeup();
					OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_SINGLE, 100 * SYS_TICK);
#endif
				}
			}
    		break;
		}
#if (__CUST_CODE__ == __CUST_KQ__)
		if (KQ->BLEUpgradeStart && UserCtrl.DevUpgradeFlag)
		{
			UserCtrl.DevUpgradeOK = 0;
			DBG("beign upgrade cc2541");
			if (FileCache.Head.MaigcNum == RDA_UPGRADE_MAGIC_NUM)
			{
				DBG("file is gprs, not ble");
				goto BLE_UPGRADE_END;
			}

			if (FileCache.Head.BinFileLen > 248 * 1024)
			{
				DBG("file len error %d", FileCache.Head.BinFileLen);
				goto BLE_UPGRADE_END;
			}

			if (FileCache.Head.CRC32 != __CRC32((u8 *)&FileCache.SectionData[0].Data[0], FileCache.Head.BinFileLen, CRC32_START))
			{
				DBG("file crc32 error");
				goto BLE_UPGRADE_END;
			}
			GPIO_Write(WDG_PIN, 1);


			for(Retry = 0; Retry < 3; Retry++)
			{
				BLE_Upgrade();
				if (UserCtrl.DevUpgradeOK)
				{
					break;
				}
			}
BLE_UPGRADE_END:
			DBG("end upgrade cc2541");
			UserCtrl.DevUpgradeFlag = 0;
			KQ->BLEUpgradeStart = 0;
			OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_PERIODIC, 30 * SYS_TICK);
			gSys.Var[SHUTDOWN_TIME] = 100;
			if (UserCtrl.DevUpgradeOK)
			{
				UserCtrl.DevUpgradeOK = 0;
				KQ->UpgradeType = 1;
				KQ->UpgradeResult = 0;
				TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
				Monitor_RecordResponse(Monitor->TempBuf, TxLen);
				Monitor_Wakeup();
				OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_SINGLE, 100 * SYS_TICK);
			}
			else
			{
				KQ->UpgradeType = 1;
				KQ->UpgradeResult = 1;
				TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
				Monitor_RecordResponse(Monitor->TempBuf, TxLen);
				Monitor_Wakeup();
				OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_SINGLE, 100 * SYS_TICK);

			}
		}
#endif

	}
}

void User_Config(void)
{

	gSys.TaskID[USER_TASK_ID] = COS_CreateTask(User_Task, NULL,
				NULL, MMI_TASK_MIN_STACK_SIZE, MMI_TASK_PRIORITY + USER_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI User Task");
	gSys.Var[SHUTDOWN_TIME] = 100;
	InitRBuffer(&UserCtrl.ReqList, (u8 *)&UserCtrl.Event[0], 16, sizeof(COS_EVENT));
#if (__CUST_CODE__ == __CUST_KQ__)
	KQ_TTSInit();
#endif
#ifdef __TTS_TEST__
	UserCtrl.VoiceCode = 1;
#endif
}

void User_GPRSUpgradeStart(void)
{
	s32 Result;
	UserCtrl.GPRSUpgradeFlag = 1;

#if (__CUST_CODE__ == __CUST_KQ__)
	User_DevDeal(KQ_CMD_DOWNLOAD_GPRS, 0, 0, &Result);
#endif
}

void User_DevUpgradeStart(void)
{
#if (__CUST_CODE__ == __CUST_KQ__)
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	KQ->BLEUpgradeStart = 0;
#endif
	UserCtrl.DevUpgradeFlag = 1;

	OS_StartTimer(gSys.TaskID[USER_TASK_ID], BLE_TIMER_ID, COS_TIMER_MODE_SINGLE, 600 * SYS_TICK);
}

void User_AGPSStart(void)
{
	UserCtrl.AGPSFlag = 1;
}

void User_Req(u32 Param1, u32 Param2, u32 Param3)
{
	COS_EVENT Event;
	Event.nEventId = 0;
	Event.nParam1 = Param1;
	Event.nParam2 = Param2;
	Event.nParam3 = Param3;
	WriteRBufferForce(&UserCtrl.ReqList, &Event, 1);
	//OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_USER_REQ, 0, 0, 0);
}

