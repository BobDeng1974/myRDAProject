#include "user.h"
GPRS_CtrlStruct __attribute__((section (".usr_ram"))) GPRSCtrl;

u8 RssiToCSQ(u8 nRssi)
{
	u8 CSQ;
    if (nRssi > 113)
    {
    	CSQ = 0;
    }
    else if ((nRssi <= 113) && (nRssi >= 51))
    {
    	CSQ = (u8)(31 - ((nRssi - 51) / 2));
    }
    else if (nRssi < 51)
    {
    	CSQ = 31;
    }
    else
    {
    	CSQ = 99;
    }
    return CSQ;
}

void GPRS_EntryState(u8 NewState)
{
	u32 i;
	gSys.State[GPRS_STATE] = NewState;
	if (GPRS_RUN == gSys.State[GPRS_STATE])
	{
		for (i = 0; i < GPRS_CH_MAX; i++)
		{
			if (GPRSCtrl.Data[i].TaskID)
			{
				OS_SendEvent(GPRSCtrl.Data[i].TaskID, EV_MMI_GPRS_READY, 0 ,0 ,0);
			}
		}
	}
}

void GPRS_Active(void)
{
	Param_APNStruct *APN = &gSys.nParam[PARAM_TYPE_APN].Data.APN;
	OS_GetGPRSActive(&gSys.State[GPRS_ACT_STATE]);
	if (gSys.State[GPRS_ACT_STATE])
	{
		//DBG("GPRS IP PDP already act!");
		DBG("GPRS IP PDP ACT OK!");
		GPRSCtrl.To = 0;
		OS_GetCIPIPPdpCxt(&gSys.LocalIP, &gSys.DNS);
		DBG("LocalIP %u.%u.%u.%u, DNS %u.%u.%u.%u", gSys.LocalIP.u8_addr[0], gSys.LocalIP.u8_addr[1],
				gSys.LocalIP.u8_addr[2], gSys.LocalIP.u8_addr[3], gSys.DNS.u8_addr[0], gSys.DNS.u8_addr[1],
				gSys.DNS.u8_addr[2], gSys.DNS.u8_addr[3]);
		GPRS_EntryState(GPRS_RUN);
	}
	else
	{
		GPRSCtrl.To = 0;
		DBG("ACT %s %s %s", APN->APNName, APN->APNUser, APN->APNPassword);
		OS_GPRSActReq(CFW_GPRS_ACTIVED, APN->APNName, APN->APNUser, APN->APNPassword);
		GPRS_EntryState(GPRS_PDP_ACTING);
		DBG("Start gprs act!");
	}
}

void GPRS_Attach(void)
{
	u8 State = OS_GetRegStatus();

	if (!gSys.State[SIM_STATE])
	{
		GPRS_EntryState(GPRS_IDLE);
		return ;
	}

	if (!OS_GetSimStatus())
	{
		DBG("sim down!");
		gSys.State[SIM_STATE] = 0;
		gSys.State[REG_STATE] = 0;
		OS_FlyMode(1);
		if (!gSys.Error[SIM_ERROR])
		{
			Monitor_RecordData();
			SYS_Error(SIM_ERROR, 1);
		}
		GPRSCtrl.To = 0;
		GPRS_EntryState(GPRS_IDLE);
		return ;
	}


    OS_GetGPRSAttach(&gSys.State[GPRS_ATTACH_STATE]);
    if (gSys.State[GPRS_ATTACH_STATE] != CFW_GPRS_ATTACHED)
    {
    	if ( (CFW_NW_STATUS_REGISTERED_HOME == State) || (CFW_NW_STATUS_REGISTERED_ROAMING == State))
    	{
    		OS_GPRSAttachReq(CFW_GPRS_ATTACHED);
    		GPRS_EntryState(GPRS_ATTACHING);
    	}
    	else
    	{
    		DBG("%u", State);
    		GPRS_EntryState(GPRS_RESTART);
    	}
    }
    else
    {
    	GPRS_Active();
    }
}

void GPRS_MonitorTask(void)
{

	GPRSCtrl.To++;
	switch (gSys.State[GPRS_STATE])
	{
	case GPRS_IDLE:
		if (GPRSCtrl.To >= GPRSCtrl.Param[PARAM_SIM_TO])
		{
			if (!OS_GetSimStatus())
			{
				DBG("no sim!");
				if (!gSys.Error[SIM_ERROR])
				{
					Monitor_RecordData();
					SYS_Error(SIM_ERROR, 1);
				}
				SYS_Reset();
			}
		}
		break;
	case GPRS_ATTACHING:
		if (GPRSCtrl.To > GPRSCtrl.Param[PARAM_GPRS_TO])
		{
			DBG("GPRS Attach To!");
			SYS_Error(GPRS_ERROR, 1);
			SYS_Reset();
		}
		break;
	case GPRS_PDP_ACTING:
		OS_GetGPRSAttach(&gSys.State[GPRS_ATTACH_STATE]);
		if (gSys.State[GPRS_ATTACH_STATE] != CFW_GPRS_ATTACHED)
		{
			OS_GPRSAttachReq(CFW_GPRS_ATTACHED);
			GPRS_EntryState(GPRS_ATTACHING);
			DBG("start gprs attach!");
		}
		if (GPRSCtrl.To > (GPRSCtrl.Param[PARAM_GPRS_TO]))
		{
			DBG("GPRS Act to!");
			SYS_Error(GPRS_ERROR, 1);
			SYS_Reset();
		}
		break;
	case GPRS_RUN:
		GPRSCtrl.To = 0;
		break;
	default:
		if (GPRSCtrl.To > 1)
		{
			GPRS_Attach();
		}
		if (GPRSCtrl.To > (GPRSCtrl.Param[PARAM_GPRS_TO]))
		{
			DBG("GPRS Restart to!");
			SYS_Error(GPRS_ERROR, 1);
			SYS_Reset();
		}
		break;
	}



}

void GPRS_EventAnalyze(CFW_EVENT *Event)
{
	CFW_NW_NETWORK_INFO *pNetwork;
	CFW_SPEECH_CALL_IND *pSpeechCallInfo;
	Date_UserDataStruct Date;
	Time_UserDataStruct Time;
	//COS_EVENT nEvent;
	HANDLE TaskID;
	Cell_InfoUnion uCellInfo;
	u8 i;
	u32 RxLen;
	u8 *Temp;
    switch (Event->nEventId)
    {
    case EV_CFW_TCPIP_SOCKET_CONNECT_RSP:
    	TaskID = GPRS_GetTaskFromSocketID(Event->nParam1);
    	if (TaskID)
    	{
    		OS_SendEvent(TaskID, EV_MMI_NET_CONNECT_OK, 0 ,0 ,0);
    	}
    	break;
    case EV_CFW_TCPIP_SOCKET_CLOSE_RSP:
    	TaskID = GPRS_GetTaskFromSocketID(Event->nParam1);
    	if (TaskID)
    	{
    		OS_SendEvent(TaskID, EV_MMI_NET_CLOSED, 0 ,0 ,0);
    	}
    	break;
    case EV_CFW_TCPIP_SOCKET_SEND_RSP:
    	TaskID = GPRS_GetTaskFromSocketID(Event->nParam1);
    	if (TaskID)
    	{
    		OS_SendEvent(TaskID, EV_MMI_NET_SEND_OK, Event->nParam2, 0 ,0);
    	}
    	break;
    case EV_CFW_TSM_INFO_IND:
    	if (CFW_TSM_CURRENT_CELL == Event->nParam2)
    	{
    		OS_GetCellInfo(&gSys.CurrentCell, &gSys.NearbyCell);
    		gSys.State[RSSI_STATE] = RssiToCSQ(gSys.CurrentCell.nTSM_AvRxLevel);
       		uCellInfo.CellInfo.ID[0] = gSys.CurrentCell.nTSM_CellID[0];
    		uCellInfo.CellInfo.ID[1] = gSys.CurrentCell.nTSM_CellID[1];
    		uCellInfo.CellInfo.ID[2] = gSys.CurrentCell.nTSM_LAI[3];
    		uCellInfo.CellInfo.ID[3] = gSys.CurrentCell.nTSM_LAI[4];
        	if (uCellInfo.CellID != gSys.Var[CELL_ID])
        	{
        		gSys.Var[CELL_ID] = uCellInfo.CellID;
        		DBG("tsm csq %u", (u32)RssiToCSQ(gSys.CurrentCell.nTSM_AvRxLevel));
        		__HexTrace(gSys.CurrentCell.nTSM_LAI + 3, 2);
        		__HexTrace(gSys.CurrentCell.nTSM_CellID, 2);
        	}

    	}
    	else if (CFW_TSM_NEIGHBOR_CELL == Event->nParam2)
    	{
    		OS_GetCellInfo(&gSys.CurrentCell, &gSys.NearbyCell);
//    		for (i = 0; i < gSys.NearbyCell.nTSM_NebCellNUM; i++)
//    		{
//    			DBG("%u", (u32)RssiToCSQ(gSys.NearbyCell.nTSM_NebCell[i].nTSM_AvRxLevel));
//    			__HexTrace(gSys.NearbyCell.nTSM_NebCell[i].nTSM_LAI + 3, 2);
//    			__HexTrace(gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID, 2);
//    		}
    	}
    	break;

    case EV_CFW_INIT_IND:
    	switch (Event->nType)
    	{
    	case CFW_INIT_STATUS_NO_SIM:
    		if (gSys.State[SIM_STATE])
    		{
    			DBG("sim down!");
    			gSys.State[SIM_STATE] = 0;
    			gSys.State[REG_STATE] = 0;
    			OS_FlyMode(1);
				if (!gSys.Error[SIM_ERROR])
				{
					Monitor_RecordData();
					SYS_Error(SIM_ERROR, 1);
				}
    			GPRSCtrl.To = 0;
    			GPRS_EntryState(GPRS_IDLE);
    		}
    		break;
    	case CFW_INIT_STATUS_SIM:
    		DBG("sim ok!");
    		GPRSCtrl.To = 0;
    		GPRS_EntryState(GPRS_IDLE);
    		OS_StartTSM();
    		gSys.State[SIM_STATE] = 1;
    		SYS_Error(SIM_ERROR, 0);
    		OS_GetIMSIReq();
    		OS_GetICCID(gSys.ICCID);
    		if (gSys.State[REG_STATE])
    		{
    			GPRS_Attach();
    		}
    		break;
    	case CFW_INIT_STATUS_SMS:
    		DBG("SMS init!");
    		OS_SMSInitStart(Event->nParam2 & 0xFF);
    		break;
    	default:
    		break;
    	}
    	break;

    case EV_CFW_SRV_STATUS_IND:
    	if (Event->nUTI == UTI_SMS_INIT && Event->nType == SMS_INIT_EV_OK_TYPE)
    	{
    		OS_SMSInitFinish(Event->nUTI, &gSys.SMSParam);
    		if (gSys.SMSParam.nNumber[0] <= 12)
    		{
    			DBG("SMS init OK %u!", gSys.SMSParam.dcs);
    			__HexTrace(gSys.SMSParam.nNumber, gSys.SMSParam.nNumber[0] + 1);
    			gSys.State[SMS_STATE] = 1;
    		}
    		else
    		{
    			DBG("SMSC ERROR %u!", gSys.SMSParam.nNumber[0]);
    		}
    	}
    	else
    	{
    		DBG("%u %x", Event->nUTI, Event->nType);
    	}
    	break;

    case EV_CFW_NW_SIGNAL_QUALITY_IND:
    	if ((Event->nParam1 & 0x000000ff) != 99)
    	{
        	if (gSys.State[RSSI_STATE] != (Event->nParam1 & 0x000000ff))
        	{
        		//DBG("signal %u %u", gSys.State[RSSI_STATE], Event->nParam1 & 0x000000ff);
        		gSys.State[RSSI_STATE] = Event->nParam1 & 0x000000ff;
        	}
    	}

//    	gSys.State[BER_STATE] = Event->nParam2 & 0x000000ff;
    	//DBG("signal %u %u", gSys.State[RSSI_STATE], gSys.State[BER_STATE]);
    	break;

    case EV_CFW_NW_REG_STATUS_IND:

        if ( (CFW_NW_STATUS_REGISTERED_HOME == Event->nParam1) || (CFW_NW_STATUS_REGISTERED_ROAMING == Event->nParam1))
        {
            DBG("net reg %u", Event->nParam1);
        	gSys.State[REG_STATE] = 1;
        	if (GPRS_IDLE == gSys.State[GPRS_STATE])
        	{
        		if (gSys.State[SIM_STATE])
        		{
        			GPRS_Attach();
        		}
        	}
        }
    	break;
    case EV_CFW_CC_SPEECH_CALL_IND:
    	if (Event->nType)
    	{
    		DBG("%u", Event->nType);
    	}
    	else
    	{
    		DBG("get a call:");
			pSpeechCallInfo = (CFW_SPEECH_CALL_IND *)Event->nParam1;
			__HexTrace(pSpeechCallInfo->TelNumber.nTelNumber, pSpeechCallInfo->TelNumber.nSize);
			if (1 == gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_CALL_AUTO_GET])
			{
				OS_CallAccpet();
			}
			else
			{
				OS_CallRelease();
			}
    	}
    	break;
    case EV_CFW_NEW_SMS_IND:
    	if (Event->nType)
    	{
    		DBG("sms ind type %u", Event->nType);
    	}
    	else
    	{
    		SMS_Receive((CFW_NEW_SMS_NODE *)Event->nParam1);
    	}
    	break;
    case EV_CFW_CC_RELEASE_CALL_IND:
    	DBG("remote hange up a call");
    	break;
    case EV_CFW_SMS_SEND_MESSAGE_RSP:
    	if (UTI_SMS_SEND == Event->nUTI)
    	{
    		DBG("sms send %02x %08x", Event->nType, Event->nParam1);
//			if (0 == Event->nType)
//			{
//				CFW_SatResponse(0x13, 0x00, 0x00, NULL, 0x00, 12, Event->nFlag);
//			}
//			else
//			{
//				CFW_SatResponse(0x13, 0x35, 0x00, NULL, 0x00, 12, Event->nFlag);
//			}
			SMS_SendFree();
			SMS_Submit();
    	}
    	break;
    case EV_CFW_TCPIP_REV_DATA_IND:
    	//DBG("NET REC!");
    	//DBG("%08x %u", Event->nParam1, Event->nParam2);
    	TaskID = GPRS_GetTaskFromSocketID(Event->nParam1);
    	if (TaskID)
    	{
    		OS_SendEvent(TaskID, EV_MMI_NET_REC, Event->nParam2, 0, 0);
    	}
    	else
    	{
    		DBG("!");
    		Temp = COS_MALLOC(1024);
    		do
    		{
    			RxLen = OS_SocketReceive((SOCKET)Event->nParam1, Temp, 1024, NULL, NULL);
    		}while (RxLen);
    		COS_FREE(Temp);
    	}
    	break;
    case EV_CFW_TCPIP_CLOSE_IND:
    	TaskID = GPRS_GetTaskFromSocketID(Event->nParam1);
    	if (TaskID)
    	{
    		for (i = 0; i < GPRS_CH_MAX; i++)
    		{
    			if (GPRSCtrl.Data[i].TaskID == TaskID)
    			{
    				DBG("Ch%d, remote close", i);
    				break;
    			}
    		}

    		OS_SendEvent(TaskID, EV_MMI_NET_REMOTE_CLOSE, 0 ,0 ,0);
    	}
    	break;
    case EV_CFW_TCPIP_ERR_IND:
    	DBG("net error %d", -Event->nParam2);
    	TaskID = GPRS_GetTaskFromSocketID(Event->nParam1);
    	if (TaskID)
    	{
    		OS_SendEvent(TaskID, EV_MMI_NET_ERROR, Event->nParam2, 0, 0);
    	}
    	break;
    case EV_CFW_TCPIP_ACCEPT_IND:
    case EV_CFW_ICMP_DATA_IND:
    case EV_CFW_ATT_STATUS_IND:
    case EV_CFW_GPRS_STATUS_IND:
    	break;
    case EV_CFW_CC_CALL_INFO_IND:
    	DBG("call info %u %u %u %u", Event->nParam1, Event->nUTI, Event->nType, Event->nFlag);
    	break;
    case EV_CFW_CC_PROGRESS_IND:
    	DBG("call runnning");
    	break;
    case EV_CFW_CC_STATUS_IND:
    	DBG("call state %x", Event->nParam1);
    	break;
    case EV_CFW_GPRS_CXT_DEACTIVE_IND:
    	if (Event->nParam1 == CID_IP)
    	{
    		DBG("CID %u pdp deact! react!", CID_IP);
    		GPRSCtrl.To = 0;
			for (i = 0; i < GPRS_CH_MAX; i++)
			{
				if (GPRSCtrl.Data[i].TaskID && (GPRSCtrl.Data[i].Socket != INVALID_SOCKET))
				{
					OS_SendEvent(GPRSCtrl.Data[i].TaskID, EV_MMI_NET_ERROR, 1000, 0, 0);
				}
			}
		    GPRS_Attach();
    	}
    	break;
    case EV_CFW_NW_NETWORKINFO_IND:
    	if(Event->nParam1 > RAM_BASE)
    	{
    		pNetwork = (CFW_NW_NETWORK_INFO *)Event->nParam1;
    		Date.Year = BCD2HEX(pNetwork->nUniversalTimeZone[0]);
    		Date.Year += 2000;
    		Date.Mon = BCD2HEX(pNetwork->nUniversalTimeZone[1]);
    		Date.Day = BCD2HEX(pNetwork->nUniversalTimeZone[2]);
    		Time.Hour = BCD2HEX(pNetwork->nUniversalTimeZone[3]);
    		Time.Min = BCD2HEX(pNetwork->nUniversalTimeZone[4]);
    		Time.Sec = BCD2HEX(pNetwork->nUniversalTimeZone[5]);
			DBG("%u %u %u %u:%u:%u", Date.Year, Date.Mon, Date.Day, Time.Hour, Time.Min, Time.Sec);
    		SYS_CheckTime(&Date, &Time);
    	}
    	break;

    case EV_CFW_DNS_RESOLV_SUC_IND:
    	GPRS_GetHostResult((s8 *)Event->nParam2, Event->nParam1);
    	Event->nParam1 = 0;
    	COS_FREE(Event->nParam2);
    	break;

    case EV_CFW_DNS_RESOLV_ERR_IND:
    	GPRS_GetHostResult((s8 *)Event->nParam2, 0);
    	COS_FREE(Event->nParam2);
    	break;

    case EV_CFW_SIM_GET_PROVIDER_ID_RSP:
    	if (Event->nParam1 > RAM_BASE)
    	{
    		OS_GetIMSI(gSys.IMSI, (s8 *)Event->nParam1, Event->nParam2);
    	}
    	if (!gSys.nParam[PARAM_TYPE_APN].Data.APN.APNName[0])
		{
    		switch (gSys.IMSI[2])
    		{
    		case GPRS_CHN_UNICOM_MNC1:
    		case GPRS_CHN_UNICOM_MNC2:
    		case GPRS_CHN_UNICOM_MNC3:
    			strcpy(gSys.nParam[PARAM_TYPE_APN].Data.APN.APNName, "cmnet");
    			break;
    		default:
    			strcpy(gSys.nParam[PARAM_TYPE_APN].Data.APN.APNName, "cmnet");
    			break;
    		}

			DBG("auto apn name %s", gSys.nParam[PARAM_TYPE_APN].Data.APN.APNName);
		}
    	__HexTrace(gSys.IMEI, IMEI_LEN);
    	__HexTrace(gSys.IMSI, IMSI_LEN);
    	__HexTrace(gSys.ICCID, ICCID_LEN);
    	break;

    case EV_CFW_GPRS_ATT_RSP:

    	if (UTI_GPRS_ATTACH == Event->nUTI)
    	{
        	if (Event->nParam1)
        	{
    			DBG("GPRS atttach error! reboot");
    			DBG("%x %u %x %x", Event->nParam1, Event->nUTI, Event->nType, Event->nFlag);
    			SYS_Reset();
        		break;
        	}
    		GPRS_Attach();
    	}
    	else if (UTI_GPRS_DETACH == Event->nUTI)
    	{
    		if (ERR_SUCCESS == Event->nParam1 && CFW_GPRS_DETACHED == Event->nType)
    		{
    			DBG("GPRS detach ok!");
    			GPRSCtrl.To = 0;
    			GPRS_EntryState(GPRS_RESTART);
    		}
    		else
    		{
    			DBG("GPRS detach error! retry");
    			DBG("%x %u %x %x", Event->nParam1, Event->nUTI, Event->nType, Event->nFlag);
    			SYS_Reset();
    		}
    	}
    	else
    	{
            DBG("%u %x %x %u %u %u\r\n", Event->nEventId, Event->nParam1, Event->nParam2,
            		Event->nUTI, Event->nType, Event->nFlag);
    	}
    	break;

    case EV_CFW_GPRS_ACT_RSP:
    	if (UTI_CID_IP_ACT == Event->nUTI)
    	{
    		GPRS_Attach();
    	}
    	else if (UTI_CID_IP_DEACT == Event->nUTI)
    	{
			OS_GetGPRSActive(&gSys.State[GPRS_ACT_STATE]);
			if (!gSys.State[GPRS_ACT_STATE])
			{
				DBG("GPRS PDP deact ok!");
				GPRS_Attach();
			}
			else
			{
				DBG("GPRS PDP deact failed, reboot!");
				DBG("%x %u %x %x", Event->nParam1, Event->nUTI, Event->nType, Event->nFlag);
    			SYS_Reset();
			}
    	}
    	else
    	{
    		DBG("GPRS act error!");
            DBG("%x %x %u %u %u\r\n", Event->nParam1, Event->nParam2,
            		Event->nUTI, Event->nType, Event->nFlag);
    	}
    	break;

    case EV_CFW_SET_COMM_RSP:
    	switch (Event->nParam1)
    	{
    	case CFW_DISABLE_COMM:
    		DBG("in fly mode!");
    		break;
    	case CFW_ENABLE_COMM:
    		DBG("out fly mode!");
    		break;
    	}
    	break;
    case EV_CFW_EXIT_IND:
    	DBG("in fly mode, ready to quit!");
		OS_FlyMode(0);
    	break;
    default:
        DBG("%u %x %x %u %u %u\r\n", Event->nEventId, Event->nParam1, Event->nParam2,
        		Event->nUTI, Event->nType, Event->nFlag);
    	break;
    }
}

void GPRS_Config(void)
{
	u8 i;
	GPRSCtrl.Param = gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param;
	GPRSCtrl.To = 0;
	for (i = 0;i < GPRS_CH_MAX; i++)
	{
		GPRSCtrl.Data[i].Socket = INVALID_SOCKET;
	}
}

void GPRS_GetHostResult(s8 *HostName, u32 IP)
{
	u8 i;
	IP_AddrUnion uIP;
	for (i = 0; i < GPRS_CH_MAX; i++)
	{
		if (GPRSCtrl.Data[i].TaskID && !strcmp(GPRSCtrl.Data[i].Url, HostName))
		{
			uIP.u32_addr = IP;
			DBG("%s -> %u.%u.%u.%u", HostName, uIP.u8_addr[0], uIP.u8_addr[1], uIP.u8_addr[2], uIP.u8_addr[3]);
			if (IP)
			{

				OS_SendEvent(GPRSCtrl.Data[i].TaskID, EV_MMI_GPRS_GET_HOST, 1, IP, 0);
			}
			else
			{
				OS_SendEvent(GPRSCtrl.Data[i].TaskID, EV_MMI_GPRS_GET_HOST, 0, 0, 0);
			}
			memset(GPRSCtrl.Data[i].Url, 0, sizeof(GPRSCtrl.Data[i].Url));
		}
	}
}

s32 GPRS_RegDNS(u8 Channel, u8 *Url)
{
	IP_AddrUnion uIP;
	struct ip_addr IP;
	u32 Result;
	if (Channel < GPRS_CH_MAX)
	{
		if (!GPRSCtrl.Data[Channel].TaskID)
		{
			DBG("no reg Channel");
			return -1;
		}

		strcpy(GPRSCtrl.Data[Channel].Url, Url);
		Result = OS_GetHost(GPRSCtrl.Data[Channel].Url, &IP);

		switch (Result)
		{
		case RESOLV_QUERY_INVALID:
			DBG("%u", Result);
			OS_SendEvent(GPRSCtrl.Data[Channel].TaskID, EV_MMI_GPRS_GET_HOST, 0, 0 ,0);
			return -1;
		case RESOLV_COMPLETE:
			uIP.u32_addr = IP.addr;
			DBG("%s -> %u.%u.%u.%u", GPRSCtrl.Data[Channel].Url, uIP.u8_addr[0], uIP.u8_addr[1], uIP.u8_addr[2], uIP.u8_addr[3]);
			OS_SendEvent(GPRSCtrl.Data[Channel].TaskID, EV_MMI_GPRS_GET_HOST, 1, IP.addr, 0);
			return 0;
		case RESOLV_QUERY_QUEUED:
			return 0;
		default:
			DBG("%u", Result);
			return -1;
		}
	}
	DBG("!");
	return -1;
}

void GPRS_RegChannel(u8 Channel, HANDLE TaskID)
{
	if (Channel < GPRS_CH_MAX)
	{
		GPRSCtrl.Data[Channel].TaskID = TaskID;
	}
}

void GPRS_RegSocket(u8 Channel, SOCKET Socket)
{
	if (Channel < GPRS_CH_MAX)
	{
		GPRSCtrl.Data[Channel].Socket = Socket;
		DBG("%u %u", Channel, Socket);
	}
}

void GPRS_ResetSocket(u8 Channel)
{
	if (Channel < GPRS_CH_MAX)
	{
		GPRSCtrl.Data[Channel].Socket = INVALID_SOCKET;
	}
}

HANDLE GPRS_GetTaskFromSocketID(SOCKET SocketID)
{
	u8 i;
	for (i = 0; i < GPRS_CH_MAX; i++)
	{
		if (GPRSCtrl.Data[i].Socket == SocketID)
		{
			return GPRSCtrl.Data[i].TaskID;
		}
	}
	return 0;
}

