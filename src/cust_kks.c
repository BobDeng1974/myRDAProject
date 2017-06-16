#include "user.h"
#if (__CUST_CODE__ == __CUST_KKS__)
Monitor_CtrlStruct __attribute__((section (".usr_ram"))) KKSCtrl;
extern User_CtrlStruct __attribute__((section (".usr_ram"))) UserCtrl;

u8 KKS_CheckUartHead(u8 Data)
{
	if (Data == 0xAA)
	{
		return 1;
	}
	return 0;
}

void KKS_FlushDevInfo(void)
{
	u8 DevInfo = 0;
	IO_ValueUnion uIO;
	KKS_CustDataStruct *KKS = (KKS_CustDataStruct *)KKSCtrl.CustData;
	if (gSys.RMCInfo->LocatStatus)
	{
		DevInfo = (1 << 6);
	}
	uIO.Val = gSys.Var[IO_VAL];
	if (uIO.IOVal.VCC)
	{
		DevInfo |= (1 << 2);
	}
	if (uIO.IOVal.ACC)
	{
		DevInfo |= (1 << 1);
	}
	if (gSys.State[ALARM_STATE])
	{
		DevInfo |= (1 << 0);
	}
	KKS->DevInfo = DevInfo;

	if (gSys.Var[VBAT] <= 3400)
	{
		KKS->Power = 0;
	}
	else if (gSys.Var[VBAT] <= 3700)
	{
		KKS->Power = 2;
	}
	else if (gSys.Var[VBAT] <= 3900)
	{
		KKS->Power = 4;
	}
	else
	{
		KKS->Power = 5;
	}

	if (gSys.State[RSSI_STATE] >= 25)
	{
		KKS->Signal = 4;
	}
	else if (gSys.State[RSSI_STATE] >= 15)
	{
		KKS->Signal = 3;
	}
	else if (gSys.State[RSSI_STATE] >= 10)
	{
		KKS->Signal = 2;
	}
	else if (gSys.State[RSSI_STATE])
	{
		KKS->Signal = 1;
	}
	else
	{
		KKS->Signal = 0;
	}
}

void KKS_GetMCC(void)
{
	KKS_CustDataStruct *KKS = (KKS_CustDataStruct *)KKSCtrl.CustData;
	u32 dwTemp;
	dwTemp = BCDToInt(gSys.IMSI, 2);
	KKS->MCC = dwTemp;
	dwTemp = BCDToInt(gSys.IMSI + 2, 1);
	KKS->MNC = dwTemp;
}

u32 KKS_Pack(u8 *Src, u16 Len, u8 Cmd, u8 IsLong, u8 *Dst)
{
	KKS_CustDataStruct *KKS = (KKS_CustDataStruct *)KKSCtrl.CustData;
	u16 MsgLen = Len + 5;
	u16 CRC16;
	u32 Pos = 0;
	if (IsLong)
	{
		Dst[Pos++] = KKS_LONG_HEAD;
		Dst[Pos++] = KKS_LONG_HEAD;
		Dst[Pos++] = MsgLen >> 8;
		Dst[Pos++] = MsgLen & 0x00ff;
	}
	else
	{
		Dst[Pos++] = KKS_SHORT_HEAD;
		Dst[Pos++] = KKS_SHORT_HEAD;
		Dst[Pos++] = MsgLen & 0x00ff;
	}
	Dst[Pos++] = Cmd;
	if (Len)
	{
		memcpy(Dst + Pos, Src, Len);
		Pos += Len;
	}

	Dst[Pos++] = KKS->MsgSn >> 8;
	Dst[Pos++] = KKS->MsgSn & 0x00ff;
	Dst[Pos++] = KKS->MsgSn++;
	CRC16 = ~CRC16Cal(Dst + 2, Pos - 2, CRC16_START, CRC16_GEN);
	Dst[Pos++] = CRC16 >> 8;
	Dst[Pos++] = CRC16 & 0x00ff;

	Dst[Pos++] = KKS_TAIL1;
	Dst[Pos++] = KKS_TAIL2;
	return Pos;
}

u32 KKS_LoginTx(void)
{
	KKS_LoginBody MsgBody;
	memcpy(MsgBody.DevID, gSys.IMEI, 8);
	MsgBody.DevType[0] = KKS_DEV_TYPE_H;
	MsgBody.DevType[1] = KKS_DEV_TYPE_L;
	MsgBody.DevZone[0] = 0x32;
	MsgBody.DevZone[1] = 0x01;
	return KKS_Pack(&MsgBody, sizeof(MsgBody), KKS_LOGIN_TX, 0, KKSCtrl.SendBuf);
}

u32 KKS_HeartTx(void)
{
	KKS_CustDataStruct *KKS = (KKS_CustDataStruct *)KKSCtrl.CustData;
	KKS_HeartBody MsgBody;
	KKS_FlushDevInfo();
	MsgBody.DevInfo[0] = KKS->DevInfo;
	MsgBody.Power[0] = KKS->Power;
	MsgBody.Signal[0] = KKS->Signal;
	MsgBody.Language[0] = 0;
	MsgBody.Language[1] = KKS_ENGLISH;
	return KKS_Pack(&MsgBody, sizeof(MsgBody), KKS_HEART_TX, 0, KKSCtrl.SendBuf);
}

u32 KKS_LocatTx(Monitor_RecordStruct *Record)
{
	KKS_CustDataStruct *KKS = (KKS_CustDataStruct *)KKSCtrl.CustData;
	KKS_LocatBody MsgBody;
	u8 ucTemp;
	u16 wTemp;
	u32 dwTemp;
	Date_Union uDate;
	Time_Union uTime;
	u64 Tamp;
	u64 GPSTamp;
	memset(&MsgBody, 0, sizeof(MsgBody));
	MsgBody.DateTime[0] = Record->uDate.Date.Year - 2000;
	MsgBody.DateTime[1] = Record->uDate.Date.Mon;
	MsgBody.DateTime[2] = Record->uDate.Date.Day;
	MsgBody.DateTime[3] = Record->uTime.Time.Hour;
	MsgBody.DateTime[4] = Record->uTime.Time.Min;
	MsgBody.DateTime[5] = Record->uTime.Time.Sec;
	ucTemp = Record->CN[0] + Record->CN[1] + Record->CN[2] + Record->CN[3];
	if (ucTemp > 15)
	{
		ucTemp = 15;
	}
	MsgBody.GPSInfo[0] = 0xC0 + ucTemp;

	dwTemp = Record->RMC.LatDegree * 1000000 + Record->RMC.LatMin * 100 / 60;
	dwTemp = htonl(dwTemp);
	memcpy(MsgBody.Lat, &dwTemp, 4);

	dwTemp = Record->RMC.LgtDegree * 1000000 + Record->RMC.LgtMin * 100 / 60;
	dwTemp = htonl(dwTemp);
	memcpy(MsgBody.Lgt, &dwTemp, 4);

	dwTemp = Record->RMC.Speed * 1852 / 1000000;
	if (dwTemp > 255)
	{
		dwTemp = 255;
	}
	ucTemp = dwTemp;
	MsgBody.Speed[0] = ucTemp;

	wTemp = Record->RMC.Cog / 1000;
	if (wTemp >= 360)
	{
		wTemp = 0;
	}

	if (Record->RMC.LocatStatus)
	{
		wTemp |= (1 << 12);
	}
	if (Record->RMC.LatNS != 'S')
	{
		wTemp |= (1 << 10);
	}
	if (Record->RMC.LgtEW == 'W')
	{
		wTemp |= (1 << 11);
	}
	MsgBody.State[0] = wTemp >> 8;
	MsgBody.State[1] = wTemp & 0x00ff;

	MsgBody.MCC[0] = KKS->MCC >> 8;
	MsgBody.MCC[1] = KKS->MCC & 0x00ff;
	MsgBody.MNC[0] = KKS->MNC;
	MsgBody.LAI[0] = Record->CellInfoUnion.CellInfo.ID[2];
	MsgBody.LAI[1] = Record->CellInfoUnion.CellInfo.ID[3];
	MsgBody.CI[1] = Record->CellInfoUnion.CellInfo.ID[0];
	MsgBody.CI[2] = Record->CellInfoUnion.CellInfo.ID[1];
	MsgBody.ACC[0] = Record->IOValUnion.IOVal.ACC;

	uDate.dwDate = gSys.Var[DATE];
	uTime.dwTime = gSys.Var[TIME];
	Tamp = UTC2Tamp(&uDate.Date, &uTime.Time);
	uDate.dwDate = Record->uDate.dwDate;
	uTime.dwTime = Record->uTime.dwTime;
	GPSTamp = UTC2Tamp(&uDate.Date, &uTime.Time);

	if ( Tamp > (GPSTamp + 60) )
	{
		MsgBody.Type[0] = 1;
	}
	else
	{
		MsgBody.Type[0] = 0;
	}

	dwTemp = htonl(Record->MileageKM);
	memcpy(MsgBody.Mileage, &dwTemp, 4);
	return KKS_Pack(&MsgBody, sizeof(MsgBody), KKS_LOCAT_TX, 0, KKSCtrl.SendBuf);
}

u32 KKS_LBSTx(void)
{
	KKS_CustDataStruct *KKS = (KKS_CustDataStruct *)KKSCtrl.CustData;
	KKS_LBSBody MsgBody;
	u8 i;
	Date_Union uDate;
	Time_Union uTime;
	uDate.dwDate = gSys.Var[DATE];
	uTime.dwTime = gSys.Var[TIME];

	memset(&MsgBody, 0, sizeof(MsgBody));
	MsgBody.DateTime[0] = uDate.Date.Year - 2000;
	MsgBody.DateTime[1] = uDate.Date.Mon;
	MsgBody.DateTime[2] = uDate.Date.Day;
	MsgBody.DateTime[3] = uTime.Time.Hour;
	MsgBody.DateTime[4] = uTime.Time.Min;
	MsgBody.DateTime[5] = uTime.Time.Sec;

	MsgBody.MCC[0] = KKS->MCC >> 8;
	MsgBody.MCC[1] = KKS->MCC & 0x00ff;
	MsgBody.MNC[0] = KKS->MNC;

	MsgBody.LBSList[0].LAI[0] = gSys.CurrentCell.nTSM_LAI[3];
	MsgBody.LBSList[0].LAI[1] = gSys.CurrentCell.nTSM_LAI[4];
	MsgBody.LBSList[0].CI[0] = gSys.CurrentCell.nTSM_CellID[0];
	MsgBody.LBSList[0].CI[1] = gSys.CurrentCell.nTSM_CellID[1];
	MsgBody.LBSList[0].RSSI[0] = 255 - gSys.CurrentCell.nTSM_AvRxLevel;

	for (i = 0;i < 6; i++)
	{
		MsgBody.LBSList[i + 1].LAI[0] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_LAI[3];
		MsgBody.LBSList[i + 1].LAI[1] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_LAI[4];
		MsgBody.LBSList[i + 1].CI[0] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[0];
		MsgBody.LBSList[i + 1].CI[1] = gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[1];
		MsgBody.LBSList[i + 1].RSSI[0] = 255 - gSys.NearbyCell.nTSM_NebCell[i].nTSM_AvRxLevel;
	}
	MsgBody.Diff[0] = 0xff;
	MsgBody.Language[1] = KKS_ENGLISH;
	return KKS_Pack(&MsgBody, sizeof(MsgBody), KKS_LBS_TX, 0, KKSCtrl.TempBuf);
}

u32 KKS_AlarmTx(Monitor_RecordStruct *Record)
{
	KKS_CustDataStruct *KKS = (KKS_CustDataStruct *)KKSCtrl.CustData;
	KKS_AlarmBody MsgBody;
	u8 ucTemp;
	u16 wTemp;
	u32 dwTemp;
	Date_Union uDate;
	Time_Union uTime;
	u64 Tamp;
	u64 GPSTamp;
	memset(&MsgBody, 0, sizeof(MsgBody));
	MsgBody.DateTime[0] = Record->uDate.Date.Year - 2000;
	MsgBody.DateTime[1] = Record->uDate.Date.Mon;
	MsgBody.DateTime[2] = Record->uDate.Date.Day;
	MsgBody.DateTime[3] = Record->uTime.Time.Hour;
	MsgBody.DateTime[4] = Record->uTime.Time.Min;
	MsgBody.DateTime[5] = Record->uTime.Time.Sec;
	ucTemp = Record->CN[0] + Record->CN[1] + Record->CN[2] + Record->CN[3];
	if (ucTemp > 15)
	{
		ucTemp = 15;
	}
	MsgBody.GPSInfo[0] = 0xC0 + ucTemp;

	dwTemp = Record->RMC.LatDegree * 1000000 + Record->RMC.LatMin * 100 / 60;
	dwTemp = htonl(dwTemp);
	memcpy(MsgBody.Lat, &dwTemp, 4);

	dwTemp = Record->RMC.LgtDegree * 1000000 + Record->RMC.LgtMin * 100 / 60;
	dwTemp = htonl(dwTemp);
	memcpy(MsgBody.Lgt, &dwTemp, 4);

	dwTemp = Record->RMC.Speed * 1852 / 1000000;
	if (dwTemp > 255)
	{
		dwTemp = 255;
	}
	ucTemp = dwTemp;
	MsgBody.Speed[0] = ucTemp;

	wTemp = Record->RMC.Cog / 1000;
	if (wTemp >= 360)
	{
		wTemp = 0;
	}

	if (Record->RMC.LocatStatus)
	{
		wTemp |= (1 << 12);
	}
	if (Record->RMC.LatNS != 'S')
	{
		wTemp |= (1 << 10);
	}
	if (Record->RMC.LgtEW == 'W')
	{
		wTemp |= (1 << 11);
	}
	MsgBody.State[0] = wTemp >> 8;
	MsgBody.State[1] = wTemp & 0x00ff;

	MsgBody.MCC[0] = KKS->MCC >> 8;
	MsgBody.MCC[1] = KKS->MCC & 0x00ff;
	MsgBody.MNC[0] = KKS->MNC;
	MsgBody.LAI[0] = Record->CellInfoUnion.CellInfo.ID[2];
	MsgBody.LAI[1] = Record->CellInfoUnion.CellInfo.ID[3];
	MsgBody.CI[1] = Record->CellInfoUnion.CellInfo.ID[0];
	MsgBody.CI[2] = Record->CellInfoUnion.CellInfo.ID[1];
	MsgBody.ACC[0] = Record->IOValUnion.IOVal.ACC;
	MsgBody.LBSLen[0] = 9;
	KKS_FlushDevInfo();
	MsgBody.DevInfo[0] = KKS->DevInfo;
	MsgBody.Power[0] = KKS->Power;
	MsgBody.Signal[0] = KKS->Signal;
	if (Record->Alarm[ALARM_TYPE_CRASH])
	{
		MsgBody.Language[0] = KKS_ALARM_CRASH;
	}
	else if (Record->Alarm[ALARM_TYPE_MOVE])
	{
		MsgBody.Language[0] = KKS_ALARM_MOVE;
	}
	else if (Record->Alarm[ALARM_TYPE_CUTLINE])
	{
		MsgBody.Language[0] = KKS_ALARM_CUTLINE;
	}
	else if (Record->Alarm[ALARM_TYPE_OVERSPEED])
	{
		MsgBody.Language[0] = KKS_ALARM_OVERSPEED;
	}
	else if (Record->Alarm[ALARM_TYPE_LOWPOWER])
	{
		MsgBody.Language[0] = KKS_ALARM_LOWPOWER;
	}
	else if (Record->Alarm[ALARM_TYPE_ACC_ON])
	{
		MsgBody.Language[0] = KKS_ALARM_ACC_ON;
	}
	else if (Record->Alarm[ALARM_TYPE_ACC_OFF])
	{
		MsgBody.Language[0] = KKS_ALARM_ACC_OFF;
	}

	MsgBody.Language[1] = KKS_ENGLISH;
	return KKS_Pack(&MsgBody, sizeof(MsgBody), KKS_ALARM_TX, 0, KKSCtrl.SendBuf);
}

u32 KKS_DirSendTx(u8 *Src, u16 Len)
{
	u32 TxLen;
	TxLen = KKS_Pack(Src, Len, KKS_DIRSEND_TX, 1, KKSCtrl.TempBuf);
	Monitor_RecordResponse(KKSCtrl.TempBuf, TxLen);
	return 0;
}

u32 KKS_LoginRx(void *pData)
{
	KKS_CustDataStruct *KKS = (KKS_CustDataStruct *)KKSCtrl.CustData;
	KKS->IsAuthOK = 1;
	return 0;
}

u32 KKS_HeartRx(void *pData)
{
	KKS_CustDataStruct *KKS = (KKS_CustDataStruct *)KKSCtrl.CustData;
	KKS->NoAck = 0;
	return 0;
}

u32 KKS_CmdRx(void *pData)
{
	return 0;
}

u32 KKS_AlarmRx(void *pData)
{
	return 0;
}

u32 KKS_TimeRx(void *pData)
{
	return 0;
}

u32 KKS_DWChineseRx(void *pData)
{
	return 0;
}

u32 KKS_DWEnglishRx(void *pData)
{
	return 0;
}

const CmdFunStruct KKSCmdFun[] =
{
		{
				KKS_LOGIN_RX,
				KKS_LoginRx,
		},
		{
				KKS_HEART_RX,
				KKS_HeartRx,
		},
		{
				KKS_CMD_RX,
				KKS_CmdRx,
		},
		{
				KKS_ALARM_RX,
				KKS_AlarmRx,
		},
		{
				KKS_TIME_RX,
				KKS_TimeRx,
		},
		{
				KKS_DW_CH_RX,
				KKS_DWChineseRx,
		},
		{
				KKS_DW_EN_RX,
				KKS_DWEnglishRx,
		},
};

s32 KKS_ReceiveAnalyze(void *pData)
{
	u32 RxLen = (u32)pData;
	u32 FinishLen = 0,i,j;
	u16 CRC16, CRC16Org;
	u32 Cmd;
	Buffer_Struct Buffer;
	KKS_CustDataStruct *KKS = (KKS_CustDataStruct *)KKSCtrl.CustData;
	DBG("Receive %d", RxLen);

	while (RxLen)
	{
		if (RxLen > MONITOR_RXBUF_LEN)
		{
			FinishLen = MONITOR_RXBUF_LEN;
		}
		else
		{
			FinishLen = RxLen;
		}

		RxLen -= OS_SocketReceive(KKSCtrl.Net.SocketID, KKSCtrl.RecBuf, FinishLen, NULL, NULL);
		//加入协议分析
		__HexTrace(KKSCtrl.RecBuf, FinishLen);
		for (i = 0; i < FinishLen; i++)
		{
			switch (KKSCtrl.RxState)
			{
			case KKS_PRO_FIND_HEAD1:
				if (KKS_SHORT_HEAD == KKSCtrl.RecBuf[i])
				{
					KKSCtrl.AnalyzeBuf[0] = KKS_SHORT_HEAD;
					KKSCtrl.RxState = KKS_PRO_FIND_HEAD2;
				}

				if (KKS_LONG_HEAD == KKSCtrl.RecBuf[i])
				{
					KKSCtrl.AnalyzeBuf[0] = KKS_LONG_HEAD;
					KKSCtrl.RxState = KKS_PRO_FIND_HEAD3;
				}
				break;
			case KKS_PRO_FIND_HEAD2:
				if (KKS_SHORT_HEAD == KKSCtrl.RecBuf[i])
				{
					KKSCtrl.AnalyzeBuf[1] = KKS_SHORT_HEAD;
					KKSCtrl.RxState = KKS_PRO_FIND_LEN;
					KKSCtrl.RxLen = 2;
				}
				else
				{
					KKSCtrl.RxState = KKS_PRO_FIND_HEAD1;
				}
				break;
			case KKS_PRO_FIND_HEAD3:
				if (KKS_LONG_HEAD == KKSCtrl.RecBuf[i])
				{
					KKSCtrl.AnalyzeBuf[1] = KKS_LONG_HEAD;
					KKSCtrl.RxState = KKS_PRO_FIND_LEN;
					KKSCtrl.RxLen = 2;
				}
				else
				{
					KKSCtrl.RxState = KKS_PRO_FIND_HEAD1;
				}
				break;
			case KKS_PRO_FIND_LEN:
				KKSCtrl.AnalyzeBuf[KKSCtrl.RxLen++] = KKSCtrl.RecBuf[i];
				if (KKSCtrl.RxLen == (KKSCtrl.AnalyzeBuf[0] - KKS_SHORT_HEAD + 3))
				{
					if (KKS_SHORT_HEAD == KKSCtrl.AnalyzeBuf[0])
					{
						KKSCtrl.RxNeedLen = KKSCtrl.AnalyzeBuf[KKSCtrl.RxLen - 1] + KKSCtrl.RxLen;
					}
					else
					{
						KKSCtrl.RxNeedLen = KKSCtrl.AnalyzeBuf[KKSCtrl.RxLen - 2];
						KKSCtrl.RxNeedLen = KKSCtrl.RxNeedLen * 256 + KKSCtrl.AnalyzeBuf[KKSCtrl.RxLen - 1] + KKSCtrl.RxLen;
					}

					KKSCtrl.RxState = KKS_PRO_FIND_TAIL1;
				}
				break;
			case KKS_PRO_FIND_TAIL1:
				KKSCtrl.AnalyzeBuf[KKSCtrl.RxLen++] = KKSCtrl.RecBuf[i];
				if (KKSCtrl.RxLen >= KKSCtrl.RxNeedLen)
				{
					if (KKS_TAIL1 == KKSCtrl.RecBuf[i])
					{
						KKSCtrl.RxState = KKS_PRO_FIND_TAIL2;
					}
					else
					{
						DBG("Tail error %02x", KKSCtrl.RecBuf[i]);
					}
				}
				break;
			case KKS_PRO_FIND_TAIL2:
				KKSCtrl.AnalyzeBuf[KKSCtrl.RxLen++] = KKSCtrl.RecBuf[i];
				if (KKSCtrl.RxLen >= KKSCtrl.RxNeedLen)
				{
					if (KKS_TAIL2 == KKSCtrl.RecBuf[i])
					{
						CRC16 = ~CRC16Cal(KKSCtrl.AnalyzeBuf + 2, KKSCtrl.RxLen - 6, CRC16_START, CRC16_GEN);
						CRC16Org = KKSCtrl.AnalyzeBuf[KKSCtrl.RxLen - 4];
						CRC16Org = CRC16Org * 256 + KKSCtrl.AnalyzeBuf[KKSCtrl.RxLen - 3];
						if (CRC16 != CRC16Org)
						{
							DBG("%04x, %04x", CRC16, CRC16Org);
						}
						else
						{
							if (KKS_SHORT_HEAD == KKSCtrl.AnalyzeBuf[0])
							{
								Cmd = KKSCtrl.AnalyzeBuf[3];
								Buffer.Data = KKSCtrl.AnalyzeBuf + 4;
								Buffer.Pos = KKSCtrl.RxNeedLen - 5;
							}
							else
							{
								Cmd = KKSCtrl.AnalyzeBuf[4];
								Buffer.Data = KKSCtrl.AnalyzeBuf + 5;
								Buffer.Pos = KKSCtrl.RxNeedLen - 5;
							}
							for (j = 0;j < sizeof(KKSCmdFun)/sizeof(CmdFunStruct); j++)
							{
								if (Cmd == KKSCmdFun[j].Cmd & 0x000000ff)
								{
									DBG("Cmd %08x", Cmd);
									KKSCmdFun[j].Func((void *)&Buffer);
									break;
								}
							}

						}
						KKSCtrl.RxState = KKS_PRO_FIND_HEAD1;
					}
					else
					{
						DBG("Tail error %02x", KKSCtrl.RecBuf[i]);
					}
				}
				break;
			}
		}
		if (RxLen)
			DBG("rest %d", RxLen);
	}
	return 0;
}

u8 KKS_Connect(Monitor_CtrlStruct *Monitor, Net_CtrlStruct *Net, s8 *Url)
{
	u8 ProcessFinish = 0;
	IP_AddrUnion uIP;
	Led_FlushType(LED_NET_PIN, LED_FLUSH_SLOW);

	Net->To = Monitor->Param[PARAM_MONITOR_NET_TO];
	if (Net->SocketID != INVALID_SOCKET)
	{
		DBG("Need close socket before connect!");
		Net_Disconnect(Net);
	}

	if (Url)
	{
		Net_Connect(Net, 0, Url);
	}
	else
	{
		Net_Connect(Net, Net->IPAddr.s_addr, Url);
	}

	if (Net->Result != NET_RES_CONNECT_OK)
	{
		Led_FlushType(LED_NET_PIN, LED_FLUSH_SLOW);
		if (Net->SocketID != INVALID_SOCKET)
		{
			Net_Disconnect(Net);
		}
		ProcessFinish = 0;
	}
	else
	{
		Led_FlushType(LED_NET_PIN, LED_ON);
		uIP.u32_addr = Net->IPAddr.s_addr;
		DBG("IP %d.%d.%d.%d OK", (u32)uIP.u8_addr[0], (u32)uIP.u8_addr[1],
				(u32)uIP.u8_addr[2], (u32)uIP.u8_addr[3]);
		ProcessFinish = 1;
	}
	return ProcessFinish;
}

u8 KKS_Send(Monitor_CtrlStruct *Monitor, Net_CtrlStruct *Net, u32 Len)
{
	KKS_CustDataStruct *KKS = (KKS_CustDataStruct *)KKSCtrl.CustData;
	Led_FlushType(LED_NET_PIN, LED_FLUSH_FAST);
	Net->To = Monitor->Param[PARAM_MONITOR_NET_TO];
	DBG("%d", Len);
	__HexTrace(Monitor->SendBuf, Len);
	Net_Send(Net, Monitor->SendBuf, Len);
	if (Net->Result != NET_RES_SEND_OK)
	{
		Led_FlushType(LED_NET_PIN, LED_FLUSH_SLOW);
		return 0;
	}
	else
	{
		Led_FlushType(LED_NET_PIN, LED_ON);
		return 1;
	}
}

void KKS_Task(void *pData)
{
	Monitor_CtrlStruct *Monitor = &KKSCtrl;
	Net_CtrlStruct *Net = &KKSCtrl.Net;
	KKS_CustDataStruct *KKS = (KKS_CustDataStruct *)KKSCtrl.CustData;
	Param_UserStruct *User = &gSys.nParam[PARAM_TYPE_USER].Data.UserInfo;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	u32 SleepTime;
	u32 KeepTime;
	u8 ErrorOut = 0;

	COS_EVENT Event;
	u8 AuthCnt = 0;
	u32 TxLen = 0;
	u8 DataType = 0;
//下面变量为每个协议独有的
	DBG("Task start! %d %d %d %d %d %d %d %d %d" ,
			Monitor->Param[PARAM_GS_WAKEUP_MONITOR], Monitor->Param[PARAM_GS_JUDGE_RUN],
			Monitor->Param[PARAM_UPLOAD_RUN_PERIOD], Monitor->Param[PARAM_UPLOAD_STOP_PERIOD],
			Monitor->Param[PARAM_UPLOAD_HEART_PERIOD], Monitor->Param[PARAM_MONITOR_NET_TO],
			Monitor->Param[PARAM_MONITOR_KEEP_TO], Monitor->Param[PARAM_MONITOR_SLEEP_TO],
			Monitor->Param[PARAM_MONITOR_RECONNECT_MAX]);

	Monitor->MonitorID.dwID = MainInfo->UID[0];
    DBG("monitor id %d", Monitor->MonitorID.dwID);
    AuthCnt = 0;
    Monitor->IsWork = 1;
    KeepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_KEEP_TO];
    gSys.State[MONITOR_STATE] = KKS_STATE_AUTH;
    while (!ErrorOut)
    {

    	if (Monitor->IsWork && Monitor->Param[PARAM_MONITOR_KEEP_TO])
    	{
    		if (gSys.Var[SYS_TIME] > KeepTime)
    		{
    			DBG("sleep!");
    			gSys.Monitor->WakeupFlag = 0;
    			if (Net->SocketID != INVALID_SOCKET)
    			{
    				DBG("Need close socket before sleep!");
    				Net_Disconnect(Net);
    			}
    			gSys.State[MONITOR_STATE] = KKS_STATE_SLEEP;
    			Monitor->IsWork = 0;
    			SleepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_SLEEP_TO];
    		}
    	}

    	switch (gSys.State[MONITOR_STATE])
    	{

    	case KKS_STATE_AUTH:
    		Monitor->IsWork = 1;
    		Net->TCPPort = MainInfo->TCPPort;
    		Net->UDPPort = MainInfo->UDPPort;
    		Net->To = AuthCnt * 15;
    		Net_WaitTime(Net);
    		DBG("start auth!");
    		if (KKS_Connect(Monitor, Net, MainInfo->MainURL))
    		{
    			Net->To = Monitor->Param[PARAM_MONITOR_NET_TO];
    			//发送认证
    			TxLen = KKS_LoginTx();
    			if (KKS_Send(Monitor, Net, TxLen))
    			{
    				Net_WaitEvent(Net);
    				if (Net->Result == NET_RES_UPLOAD)
					{
						if (KKS->IsAuthOK)
						{
							DBG("Auth success!");
							AuthCnt = 0;
							Monitor->ReConnCnt = 0;
							gSys.State[MONITOR_STATE] = KKS_STATE_AUTH;
							TxLen = KKS_Pack(NULL, 0, KKS_TIME_TX, 0, KKSCtrl.TempBuf);
							Monitor_RecordResponse(KKSCtrl.TempBuf, TxLen);
							break;
						}
					}
    			}
    			else
    			{
    				DBG("start send fail!");
    			}
    		}
			AuthCnt++;
			if (AuthCnt >= Monitor->Param[PARAM_MONITOR_RECONNECT_MAX])
			{
				DBG("Auth too much!");
				ErrorOut = 1;
				break;
			}
    		break;

    	case KKS_STATE_DATA:
    		if (Monitor_GetCacheLen(CACHE_TYPE_ALL))
    		{
    			if (Monitor_GetCacheLen(CACHE_TYPE_RES))
    			{
    				DataType = CACHE_TYPE_RES;
    				TxLen = Monitor_ExtractResponse(Monitor->SendBuf);
    			}
    			else if (Monitor_GetCacheLen(CACHE_TYPE_ALARM))
    			{
    				DataType = CACHE_TYPE_ALARM;
    				Monitor_ExtractAlarm(&Monitor->Record);
    				TxLen = KKS_AlarmTx(&Monitor->Record);

    			}
    			else if (Monitor_GetCacheLen(CACHE_TYPE_DATA))
    			{
    				DataType = CACHE_TYPE_DATA;
    				Monitor_ExtractData(&Monitor->Record);

    				if (1 == Monitor_GetCacheLen(CACHE_TYPE_DATA))
    				{
    					if (!gSys.RMCInfo->LocatStatus)
    					{
    						TxLen = KKS_LBSTx();
    						Monitor_RecordResponse(KKSCtrl.TempBuf, TxLen);
    					}
    				}
    				TxLen = KKS_LocatTx(&Monitor->Record);
    			}

				KKS_Send(Monitor, Net, TxLen);

				if (Net->Result != NET_RES_SEND_OK)
				{
					gSys.State[MONITOR_STATE] = KKS_STATE_AUTH;
					break;
				}
				else
				{
					Monitor_DelCache(DataType, 0);
				}
    		}
    		else
    		{

    			if (Monitor->Param[PARAM_MONITOR_KEEP_TO])
    			{
    				Net->To = Monitor->Param[PARAM_MONITOR_KEEP_TO];
    			}
    			else if (Monitor->Param[PARAM_UPLOAD_STOP_PERIOD] > Monitor->Param[PARAM_UPLOAD_RUN_PERIOD])
    			{
    				Net->To = Monitor->Param[PARAM_UPLOAD_STOP_PERIOD] * 2;
    			}
    			else
    			{
    				Net->To = Monitor->Param[PARAM_UPLOAD_RUN_PERIOD] * 2;
    			}
    			Net_WaitEvent(Net);
    			if (Net->Result != NET_RES_UPLOAD)
    			{
    				DBG("error!");
    				gSys.State[MONITOR_STATE] = KKS_STATE_AUTH;
    			}

    			else if (Net->Heart)
    			{
    				//合成心跳包
    				TxLen = KKS_HeartTx();
    				Net->Heart = 0;
    				KKS->NoAck++;
    				if (KKS->NoAck >= 4)
    				{
    					DBG("NO ACK %d, ReConnect", KKS->NoAck);
    					gSys.State[MONITOR_STATE] = KKS_STATE_AUTH;
    					continue;
    				}

    				Monitor_RecordResponse(Monitor->SendBuf, TxLen);
    			}

    		}

    		if (gSys.Monitor->WakeupFlag)
    		{
    			KeepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_KEEP_TO];
    		}
    		gSys.Monitor->WakeupFlag = 0;


			if (Monitor->DevCtrlStatus && !Monitor_GetCacheLen(CACHE_TYPE_ALL))
			{
				SYS_Reset();
			}
    		break;

    	case KKS_STATE_SLEEP:
    		Net_WaitEvent(Net);
    		if (Monitor->WakeupFlag)
    		{
    			DBG("alarm or vacc wakeup!");
    			gSys.State[MONITOR_STATE] = KKS_STATE_AUTH;
    		}
    		if (Monitor->Param[PARAM_MONITOR_SLEEP_TO])
    		{
        		if (gSys.Var[SYS_TIME] > SleepTime)
        		{
        			DBG("time wakeup!");
        			gSys.State[MONITOR_STATE] = KKS_STATE_AUTH;
        		}
    		}
    		break;

    	default:
    		gSys.State[MONITOR_STATE] = KKS_STATE_AUTH;
    		break;
    	}
    }
KKS_ERROR:
	SYS_Reset();
	while (1)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
	}
}

void KKS_Config(void)
{
	gSys.TaskID[MONITOR_TASK_ID] = COS_CreateTask(KKS_Task, NULL,
				NULL, MMI_TASK_MAX_STACK_SIZE , MMI_TASK_PRIORITY + MONITOR_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI KKS Task");
	KKSCtrl.Param = gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param;
	KKSCtrl.Net.SocketID = INVALID_SOCKET;
	KKSCtrl.Net.TaskID = gSys.TaskID[MONITOR_TASK_ID];
	KKSCtrl.Net.Channel = GPRS_CH_MAIN_MONITOR;
	KKSCtrl.Net.TimerID = MONITOR_TIMER_ID;
	KKSCtrl.RxState = KKS_PRO_FIND_HEAD1;
	KKSCtrl.Net.ReceiveFun = KKS_ReceiveAnalyze;
	KKSCtrl.CustData = (KKS_CustDataStruct *)COS_MALLOC(sizeof(KKS_CustDataStruct));
	memset(KKSCtrl.CustData, 0, sizeof(KKS_CustDataStruct));
	gSys.Monitor = &KKSCtrl;
	if (!KKSCtrl.Param[PARAM_UPLOAD_RUN_PERIOD])
	{
		KKSCtrl.Param[PARAM_UPLOAD_RUN_PERIOD] = 30;
	}

}
#endif
