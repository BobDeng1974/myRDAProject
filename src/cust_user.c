#include "user.h"
//#define __TTS_TEST__
#define BLE_REBOOT_TIME	(16)

User_CtrlStruct __attribute__((section (".usr_ram"))) UserCtrl;
extern Upgrade_FileStruct __attribute__((section (".file_ram"))) FileCache;
#ifdef __TTS_ENABLE__
int32_t User_TTSCb(void *pData)
{
	DBG("TTS Done!");
	return 0;
}

int32_t User_PCMCb(void *pData)
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
		DBG("play %u", UserCtrl.PlayTime);
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
	return 0;
}

#endif

#define CUST_BUF_LEN	(256)

#if (__CUST_CODE__ == __CUST_KQ__)
extern Monitor_CtrlStruct __attribute__((section (".usr_ram"))) KQCtrl;
#elif (__CUST_CODE__ == __CUST_LY__ || __CUST_CODE__ == __CUST_LY_IOTDEV__)
extern Monitor_CtrlStruct __attribute__((section (".usr_ram"))) LYCtrl;
#elif (__CUST_CODE__ == __CUST_LB__ || __CUST_CODE__ == __CUST_LB_V2__)
extern Monitor_CtrlStruct __attribute__((section (".usr_ram"))) LBCtrl;
#endif
void User_DevDeal(uint32_t nParam1, uint32_t nParam2, uint32_t nParam3, int32_t *Result)
{
#if (__CUST_CODE__ == __CUST_KQ__)
	uint8_t Buf[128];
	uint32_t TxLen;
	if (nParam1)				//需要发送用户的UART协议
	{
		TxLen = KQ_ComTxPack(nParam1, (uint8_t *)nParam2, nParam3, Buf);
		DBG("!");
		HexTrace(Buf, TxLen);
		COM_TxReq(Buf, TxLen);
	}
	else							//需要解析用户的UART协议
	{
		TxLen = KQ_ComAnalyze((uint8_t *)nParam2, nParam3, Buf, CUST_BUF_LEN, Result);
		if (TxLen)
		{
			DBG("!");
			HexTrace(Buf, TxLen);
			COM_TxReq(Buf, TxLen);
		}

	}
#elif (__CUST_CODE__ == __CUST_LY__)
	if (nParam1)
	{


	}
	else
	{
		DBG("uart rx %u", nParam3);
		HexTrace((uint8_t *)nParam2, nParam3);
		LY_ComAnalyze((uint8_t *)nParam2, nParam3, Result);

	}
#endif
}


int32_t User_WaitUartReceive(uint32 To)
{
#if (__CUST_CODE__ == __CUST_KQ__)
	int32_t Result;
#endif
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
    		case USER_TIMER_ID:
#if (__CUST_CODE__ == __CUST_KQ__)
    			DBG("ble shutdown gprs");
    			if ( (KQ->UploadInfo[2] == 0) && (KQ->UploadInfo[3] == 5) )
    			{
    				OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_PERIODIC, 10 * SYS_TICK);
    			}
    			else if ( gSys.Var[SYS_TIME] >= gSys.Var[SHUTDOWN_TIME] )
				{
					User_DevDeal(KQ_CMD_SYS_DOWN, 0, 0, &Result);
					GPIO_Write(WDG_PIN, 0);
					OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_PERIODIC, 10 * SYS_TICK);
				}
#endif
				break;
#if (__CUST_CODE__ == __CUST_KQ__)
    		case TTS_TIMER_ID:

    			if (UserCtrl.VoiceCode < TTS_CODE_MAX)
    			{
    				DBG("play code %u", UserCtrl.VoiceCode);
    				__TTS_Play(UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Data, UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Len, User_PCMCb, User_TTSCb);
    			}
    			break;
#endif
    		}
    		break;
		case EV_MMI_COM_TO_USER:
			if (!Event.nParam1)
			{
				UserCtrl.ReceiveBuf = (uint8_t *)Event.nParam2;
				return Event.nParam3;
			}
    		break;
		case EV_MMI_FTP_FINISH:
			UserCtrl.FTPDone = 1;
			UserCtrl.FTPResult = Event.nParam1;
    		break;
		}
	}
}

#if (__CUST_CODE__ == __CUST_KQ__)
void BLE_Upgrade(void)
{
	uint8_t Buf[80];
	uint8_t Retry;
	uint8_t SendOK;
	uint8_t AllSendOK;
	uint32_t AddrPos;
	uint16_t AddrIndex;
	uint8_t BLEStatus;
	uint8_t *BinBuf;
	int32_t Result;
	uint32_t TxLen;

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
		DBG("too much data %u", Result);
		goto BLE_ERROR;
	}
	HexTrace(UserCtrl.ReceiveBuf, Result);
	BLEStatus = CC2541_UpgradeRx(UserCtrl.ReceiveBuf, Result, NULL);
	if (BLEStatus)
	{
		DBG("shake hand fail %02x", BLEStatus);
		goto BLE_ERROR;
	}
	BinBuf = (uint8_t *)&FileCache.SectionData[0].Data[0];

	//发送数据包

	AllSendOK = 1;
	AddrIndex = 0;
	for (AddrPos = 0; AddrPos < CC2541_UPGRADE_BIN_LEN; AddrPos += CC2541_UPGRADE_DATA_PACK_LEN)
	{

		SendOK = 0;
		for (Retry = 0; Retry < 2; Retry++)
		{
			DBG("index %u pos %u", AddrIndex, AddrPos);

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
				DBG("too much data %u", Result);
				OS_Sleep(SYS_TICK/32);
				continue;
			}
			HexTrace(UserCtrl.ReceiveBuf, 6);
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
				DBG("too much data %u", Result);
				OS_Sleep(SYS_TICK/32);
				continue;
			}
			HexTrace(UserCtrl.ReceiveBuf, 6);
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
			DBG("too much data %u", Result);
			goto BLE_ERROR;
		}
		HexTrace(UserCtrl.ReceiveBuf, Result);
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
	uint32_t TxLen;
	int32_t Result;
	uint32_t RxLen;
	COS_EVENT Event;


#if (__CUST_CODE__ == __CUST_KQ__)
	uint8_t RunOK;
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
				OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_SINGLE, 100 * SYS_TICK);
			}
			else if (Event.nParam1 == KQ_CMD_DOWNLOAD_GPRS)
			{

				UserCtrl.GPRSUpgradeFlag = 0;
				KQ->UpgradeType = 0;
				KQ->UpgradeResult = 1;
				TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
				Monitor_RecordResponse(Monitor->TempBuf, TxLen);
				Monitor_Wakeup();
				OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_SINGLE, 100 * SYS_TICK);
			}
			continue;
		}
		RxLen = Result;
		User_DevDeal(0, (uint32_t)UserCtrl.ReceiveBuf, RxLen, &Result);
		if (Result)
		{
			DBG("error %d", Result);
			RunOK = 0;
		}
	}
	if (KQ->WaitFlag)
	{
		KQ->IsWaitOk = RunOK;
		DBG("%u %u", KQ->WaitFlag, KQ->IsWaitOk);
		Monitor_Wakeup();
	}
#elif (__CUST_CODE__ == __CUST_LY__)
	LY_CustDataStruct *LY = (LY_CustDataStruct *)LYCtrl.CustData;
	uint8_t Retry;
	uint8_t Temp[8];
	while (UserCtrl.ReqList.Len)
	{
		ReadRBuffer(&UserCtrl.ReqList, &Event, 1);
		switch (Event.nParam1)
		{
		case LY_USER_TO_ECU:
			DBG("To ECU %u", LY->ToECUBuf.Pos);
			HexTrace(LY->ToECUBuf.Data, LY->ToECUBuf.Pos);
			Temp[0] = 0x5a;
			Temp[1] = LY->ToECUBuf.Data[1];
			Temp[2] = LY->ToECUBuf.Data[2];
			Temp[3] = 1;
			Temp[4] = 2;
			Temp[5] = XorCheck(Temp, 5, 0);
			Temp[6] = 0x0d;
			Retry = 0;
LY_UART_TX:
			COM_TxReq(LY->ToECUBuf.Data, LY->ToECUBuf.Pos);
			if (Event.nParam2)
			{
				Result = User_WaitUartReceive(1);
				if (Result < 0)
				{
					if ((++Retry) < 6)
					{
						goto LY_UART_TX;
					}
					DBG("receive data error %d", Result);
					TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 1, LY_RS232TC_VERSION, Temp, 7);
					Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
				}
				else
				{
					RxLen = Result;
					User_DevDeal(0, (uint32_t)UserCtrl.ReceiveBuf, RxLen, &Result);
					if (Result)
					{
						if ((++Retry) < 6)
						{
							goto LY_UART_TX;
						}
						DBG("receive data error %d", Result);
						TxLen = LY_ResponseData(LYCtrl.TempBuf, 1, 1, LY_RS232TC_VERSION, Temp, 7);
						Monitor_RecordResponse(LYCtrl.TempBuf, TxLen);
					}
				}
			}
			break;

		default:
			break;
		}
	}

#elif (__CUST_CODE__ == __CUST_LB__ || __CUST_CODE__ == __CUST_LB_V2__)
	LB_CustDataStruct *LB = (LB_CustDataStruct *)LBCtrl.CustData;
	uint8_t TempBuf[256];
	while (UserCtrl.ReqList.Len)
	{
		ReadRBuffer(&UserCtrl.ReqList, &Event, 1);
		switch (Event.nParam1)
		{
		case LB_485_DEV_INFO:
			TxLen = LB_SendGPSInfo(Event.nParam2, TempBuf);

			COM_TxReq(TempBuf, TxLen);
			Result = User_WaitUartReceive(1);
			if (Result < 0)
			{
				DBG("receive data error %d", Result);
			}
			else
			{
				RxLen = Result;
				DBG("uart rx %u", RxLen);
				HexTrace(UserCtrl.ReceiveBuf, RxLen);
				LB_ComAnalyze(UserCtrl.ReceiveBuf, RxLen, LB_485_DEV_INFO);
			}
			break;
		case LB_485_DIR_SEND:
			TxLen = LB_SendServerToECS(TempBuf);
			COM_TxReq(TempBuf, TxLen);
			Result = User_WaitUartReceive(1);
			if (Result < 0)
			{
				DBG("receive data error %d", Result);
				TempBuf[0] = LB_LB_CTRL;
				if (LB->ECSNeedResponse)
				{
					LB->ECSNeedResponse = 0;
					LB_ServerToECSTx(TempBuf, 1);
				}
				else
				{
					LB_ECSToServerTx(TempBuf, 1);
				}

			}
			else
			{
				RxLen = Result;
				DBG("uart rx %u", RxLen);
				HexTrace(UserCtrl.ReceiveBuf, RxLen);
				LB_ComAnalyze(UserCtrl.ReceiveBuf, RxLen, LB_485_DIR_SEND);
			}
			break;
		default:
			break;
		}
	}
#else
	UserCtrl.ReqList.Len = 0;
#endif
}

void User_Task(void *pData)
{
	COS_EVENT Event;
	int32_t Result;
#if (__CUST_CODE__ == __CUST_KQ__)
	uint32_t TxLen;
	uint8_t Retry;
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_PERIODIC, 100 * SYS_TICK);
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

#if (__CUST_CODE__ == __CUST_LB__ || __CUST_CODE__ == __CUST_LB_V2__)
	IO_ValueUnion uIO;
	OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_PERIODIC, 10 * SYS_TICK);

#endif
	OS_StartTimer(gSys.TaskID[USER_TASK_ID], WDG_TIMER_ID, COS_TIMER_MODE_PERIODIC, SYS_TICK / 16);
	while (1)
	{
		COS_WaitEvent(gSys.TaskID[USER_TASK_ID], &Event, COS_WAIT_FOREVER);
		switch (Event.nEventId)
		{
    	case EV_TIMER:
    		switch (Event.nParam1)
    		{
    		case USER_TIMER_ID:
#if (__CUST_CODE__ == __CUST_KQ__)
    			DBG("ble shutdown gprs");
    			if ( (KQ->UploadInfo[2] == 0) && (KQ->UploadInfo[3] == 5) )
    			{
    				OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_PERIODIC, 10 * SYS_TICK);
    			}
    			else if ( gSys.Var[SYS_TIME] >= gSys.Var[SHUTDOWN_TIME] )
				{
					User_DevDeal(KQ_CMD_SYS_DOWN, 0, 0, &Result);
					GPIO_Write(WDG_PIN, 0);
					OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_PERIODIC, 10 * SYS_TICK);
				}
#endif
#if (__CUST_CODE__ == __CUST_LB__ || __CUST_CODE__ == __CUST_LB_V2__)
    			if (PRINT_NORMAL == gSys.State[PRINT_STATE])
    			{
        			uIO.Val = gSys.Var[IO_VAL];
        			if (uIO.IOVal.VACC)
        			{
        				User_Req(LB_485_DEV_INFO, 0, 0);
        				User_ReqRun();
        			}
//        			else
//        			{
//        				COM_SleepReq(1);
//        			}
    			}

#endif
				break;
    		case WDG_TIMER_ID:
    			OS_StopTimer(gSys.TaskID[USER_TASK_ID], WDG_TIMER_ID);
    			GPIO_Write(WDG_PIN, gSys.State[WDG_STATE]);
    			if (gSys.State[WDG_STATE])
    			{
    				OS_StartTimer(gSys.TaskID[USER_TASK_ID], WDG_TIMER_ID, COS_TIMER_MODE_PERIODIC, SYS_TICK);
    			}
    			else
    			{
    				OS_StartTimer(gSys.TaskID[USER_TASK_ID], WDG_TIMER_ID, COS_TIMER_MODE_PERIODIC, SYS_TICK * 20);
    			}
    			gSys.State[WDG_STATE] = !gSys.State[WDG_STATE];

    			break;
#if (__CUST_CODE__ == __CUST_KQ__)
    		case TTS_TIMER_ID:
    			//DM_PlayTone(DM_TONE_DTMF_1, DM_TONE_m3dB, 200, DM_TONE_m15dB);

    			if (UserCtrl.VoiceCode < TTS_CODE_MAX)
    			{
    				HexTrace(UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Data, UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Len);
    				__TTS_Play(UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Data, UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Len, User_PCMCb, User_TTSCb);
    			}
    			else
    			{
    				DBG("%u",UserCtrl.VoiceCode);
    			}
    			break;
#endif
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
			UserCtrl.FTPDone = 1;
			UserCtrl.FTPResult = Event.nParam1;
			DBG("%d %d", UserCtrl.FTPDone, UserCtrl.FTPResult);
    		break;
		}

		if (UserCtrl.FTPDone)
		{
			UserCtrl.FTPDone = 0;
			if (UserCtrl.FTPResult != FTP_FINISH_OK)
			{
				UserCtrl.FTPResult = 0;
				continue;
			}
			UserCtrl.FTPResult = 0;
			DBG("%d", UserCtrl.GPRSUpgradeFlag);
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
				OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_SINGLE, 600 * SYS_TICK);
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
#if (__CUST_CODE__ == __CUST_KQ__)
				UserCtrl.DevUpgradeFlag = 0;
				KQ->UpgradeType = 1;
				KQ->UpgradeResult = 1;
				TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
				Monitor_RecordResponse(Monitor->TempBuf, TxLen);

				Monitor_Wakeup();
				OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_SINGLE, 100 * SYS_TICK);
#endif
			}
			else if (UserCtrl.GPRSUpgradeFlag)
			{
#if (__CUST_CODE__ == __CUST_KQ__)
				UserCtrl.GPRSUpgradeFlag = 0;
				KQ->UpgradeType = 0;
				KQ->UpgradeResult = 1;
				TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
				Monitor_RecordResponse(Monitor->TempBuf, TxLen);
				Monitor_Wakeup();
				OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_SINGLE, 100 * SYS_TICK);
#endif
			}
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
				DBG("file len error %u", FileCache.Head.BinFileLen);
				goto BLE_UPGRADE_END;
			}

			if (FileCache.Head.CRC32 != __CRC32((uint8_t *)&FileCache.SectionData[0].Data[0], FileCache.Head.BinFileLen, CRC32_START))
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
			OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_PERIODIC, 30 * SYS_TICK);
			gSys.Var[SHUTDOWN_TIME] = 100;
			if (UserCtrl.DevUpgradeOK)
			{
				UserCtrl.DevUpgradeOK = 0;
				KQ->UpgradeType = 1;
				KQ->UpgradeResult = 0;
				TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
				Monitor_RecordResponse(Monitor->TempBuf, TxLen);
				Monitor_Wakeup();
				OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_SINGLE, 100 * SYS_TICK);
			}
			else
			{
				KQ->UpgradeType = 1;
				KQ->UpgradeResult = 1;
				TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
				Monitor_RecordResponse(Monitor->TempBuf, TxLen);
				Monitor_Wakeup();
				OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_SINGLE, 100 * SYS_TICK);

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
	InitRBuffer(&UserCtrl.ReqList, (uint8_t *)&UserCtrl.Event[0], 16, sizeof(COS_EVENT));
#if (__CUST_CODE__ == __CUST_KQ__)
	KQ_TTSInit();
#endif
#ifdef __TTS_TEST__
	UserCtrl.VoiceCode = 1;
#endif
}

void User_GPRSUpgradeStart(void)
{
#if (__CUST_CODE__ == __CUST_KQ__)
	int32_t Result;
#endif
	UserCtrl.GPRSUpgradeFlag = 1;
#if (__CUST_CODE__ == __CUST_KQ__)
	User_DevDeal(KQ_CMD_DOWNLOAD_GPRS, 0, 0, &Result);
#endif
}

void User_DevUpgradeStart(void)
{
	UserCtrl.DevUpgradeFlag = 1;
#if (__CUST_CODE__ == __CUST_KQ__)
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	KQ->BLEUpgradeStart = 0;
	OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_SINGLE, 600 * SYS_TICK);
#endif
}

void User_AGPSStart(void)
{
	UserCtrl.AGPSFlag = 1;
}

void User_Req(uint32_t Param1, uint32_t Param2, uint32_t Param3)
{
	COS_EVENT Event;
	Event.nEventId = 0;
	Event.nParam1 = Param1;
	Event.nParam2 = Param2;
	Event.nParam3 = Param3;
	WriteRBufferForce(&UserCtrl.ReqList, &Event, 1);
	//OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_USER_REQ, 0, 0, 0);
}

