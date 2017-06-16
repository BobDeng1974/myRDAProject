#include "user.h"
enum SLIP_ENUM
{
	SLIP_PACK_FLAG = 0xC0,
	SLIP_PACK_CODE = 0xDB,
	SLIP_PACK_F1 = 0xDC,
	SLIP_PACK_F2 = 0xDD,

	SLIP_CMD_SET_UID = 0,
	SLIP_CMD_GET_VAR,
	SLIP_CMD_GET_PARAM,
	SLIP_CMD_SET_PARAM,
	SLIP_CMD_RESET,
	SLIP_CMD_GET_TRACE,
	SLIP_CMD_START_DOWNLOAD,
	SLIP_CMD_DL_FILE,
	SLIP_CMD_GET_VERSION,
	SLIP_CMD_TEST_SMS,

	SLIP_TYPE_REQ = 0,
	SLIP_TYPE_RES = 1,
	SLIP_DEV_PC = 0,
	SLIP_DEV_GDTM = 1,
	SLIP_LEN_MAX = 1032,
};
extern COM_CtrlStruct __attribute__((section (".usr_ram"))) COMCtrl;
void SLIP_SetHead(SLIP_AnalyzeStruct *SLIP)
{
	SLIP_HeadStruct Head;
	Head.Type = SLIP_TYPE_RES;
	Head.Cmd = SLIP->Cmd;
	Head.Len = SLIP->OutLen;
	if (Head.Len)
	{
		Head.CRC16 = CRC16Cal(SLIP->OutBuf + sizeof(SLIP_HeadStruct), Head.Len, CRC16_START, CRC16_GEN);
	}
	else
	{
		Head.CRC16 = 0;
	}
	Head.From = SLIP_DEV_GDTM;
	Head.To = SLIP_DEV_PC;
	memcpy(SLIP->OutBuf, &Head, sizeof(SLIP_HeadStruct));
	SLIP->OutLen += sizeof(SLIP_HeadStruct);
}

void SLIP_ResponseTx(SLIP_AnalyzeStruct *SLIP, u32 Result)
{
	memcpy(SLIP->OutBuf + sizeof(SLIP_HeadStruct), &Result, 4);
	SLIP->OutLen = 4;
	SLIP_SetHead(SLIP);
}

s32 SLIP_SetUIDRx(void *pData)
{
	s32 Result;
	SLIP_AnalyzeStruct *SLIP = (SLIP_AnalyzeStruct *)pData;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	if ( memcmp(MainInfo->UID, SLIP->InBuf, sizeof(MainInfo->UID)) )
	{
		DBG("new uid %d %d %d", MainInfo->UID[0], MainInfo->UID[1], MainInfo->UID[2]);
		memcpy(MainInfo->UID, SLIP->InBuf, sizeof(MainInfo->UID));
		Result = Param_Save(PARAM_TYPE_MAIN);
		SLIP_ResponseTx(SLIP, Result);
	}
	else
	{
		SLIP_ResponseTx(SLIP, 1);
	}
	return 0;
}

s32 SLIP_GetVarRx(void *pData)
{
	u32 Pos = sizeof(SLIP_HeadStruct);
	SLIP_AnalyzeStruct *SLIP = (SLIP_AnalyzeStruct *)pData;

	memcpy(SLIP->OutBuf + Pos, &gSys.Var[0], sizeof(gSys.Var));
	Pos += sizeof(gSys.Var);

	memcpy(SLIP->OutBuf + Pos, &gSys.State[0], sizeof(gSys.State));
	Pos += sizeof(gSys.State);

	memcpy(SLIP->OutBuf + Pos, &gSys.Error[0], sizeof(gSys.Error));
	Pos += sizeof(gSys.Error);

	memcpy(SLIP->OutBuf + Pos, &gSys.IMEI[0], sizeof(gSys.IMEI));
	Pos += sizeof(gSys.IMEI);

	memcpy(SLIP->OutBuf + Pos, &gSys.IMSI[0], sizeof(gSys.IMSI));
	Pos += sizeof(gSys.IMSI);

	memcpy(SLIP->OutBuf + Pos, &gSys.ICCID[0], sizeof(gSys.ICCID));
	Pos += sizeof(gSys.ICCID);

	memcpy(SLIP->OutBuf + Pos, gSys.RMCInfo, sizeof(RMC_InfoStruct));
	Pos += sizeof(RMC_InfoStruct);

	memcpy(SLIP->OutBuf + Pos, &gSys.GSVInfoSave, sizeof(GSV_InfoStruct));
	Pos += sizeof(GSV_InfoStruct);
	SLIP->OutLen = Pos - sizeof(SLIP_HeadStruct);
	SLIP_SetHead(SLIP);
	gSys.Var[GSENSOR_KEEP_VAL] = 0;
	return 0;
}

//s32 SLIP_GetStateRx(void *pData)
//{
//	SLIP_AnalyzeStruct *SLIP = (SLIP_AnalyzeStruct *)pData;
//	memcpy(SLIP->OutBuf + sizeof(SLIP_HeadStruct), &gSys.State[0], sizeof(gSys.State));
//	SLIP->OutLen = sizeof(gSys.Var);
//	SLIP_SetHead(SLIP);
//	return 0;
//}
//
//s32 SLIP_GetErrorRx(void *pData)
//{
//	SLIP_AnalyzeStruct *SLIP = (SLIP_AnalyzeStruct *)pData;
//	memcpy(SLIP->OutBuf + sizeof(SLIP_HeadStruct), &gSys.Error[0], sizeof(gSys.Error));
//	SLIP->OutLen = sizeof(gSys.Var);
//	SLIP_SetHead(SLIP);
//	return 0;
//}

s32 SLIP_GetParamRx(void *pData)
{
	u32 Pos, i;
	u8 ParamType;
	SLIP_AnalyzeStruct *SLIP = (SLIP_AnalyzeStruct *)pData;
	ParamType = SLIP->InBuf[0];
	if ( (ParamType < PARAM_TYPE_MAX) || (ParamType == 0xff) )
	{

	}
	else
	{
		SLIP_ResponseTx(SLIP, 1);
		return 0;
	}
	memset(SLIP->OutBuf + sizeof(SLIP_HeadStruct), 0, 4);
	SLIP->OutBuf[sizeof(SLIP_HeadStruct) + 4] = ParamType;
	Pos = 5;

	if (ParamType == 0xff)
	{
		for (i = 0; i < PARAM_TYPE_MAX; i++)
		{
			memcpy(SLIP->OutBuf + sizeof(SLIP_HeadStruct) + Pos, gSys.nParam[i].Data.pad, sizeof(Param_Byte60Union));
			Pos += sizeof(Param_Byte60Union);
		}
		SLIP->OutLen = 5 + sizeof(Param_Byte60Union) * PARAM_TYPE_MAX;
	}
	else
	{
		memcpy(SLIP->OutBuf + sizeof(SLIP_HeadStruct) + Pos, gSys.nParam[ParamType].Data.pad, sizeof(Param_Byte60Union));
		SLIP->OutLen = 5 + sizeof(Param_Byte60Union);
	}

	SLIP_SetHead(SLIP);
	return 0;
}

s32 SLIP_SetParamRx(void *pData)
{
	SLIP_AnalyzeStruct *SLIP = (SLIP_AnalyzeStruct *)pData;
	u8 ParamType;
	ParamType = SLIP->InBuf[0];
	if (ParamType >= PARAM_TYPE_MAX)
	{
		SLIP_ResponseTx(SLIP, 1);
		return 0;
	}
	DBG("%d", SLIP->InLen - 1);
	if (SLIP->InLen - 1)
	{
		memcpy(gSys.nParam[ParamType].Data.pad, &SLIP->InBuf[1], SLIP->InLen - 1);
		if (Param_Save(ParamType))
		{
			SLIP_ResponseTx(SLIP, 1);
		}
		else
		{
			SLIP_ResponseTx(SLIP, 0);
		}
	}
	else
	{
		if (Param_Format(ParamType))
		{
			SLIP_ResponseTx(SLIP, 1);
		}
		else
		{
			SLIP_ResponseTx(SLIP, 0);
		}
	}
	return 0;
}

s32 SLIP_RebootRx(void *pData)
{
	SLIP_AnalyzeStruct *SLIP = (SLIP_AnalyzeStruct *)pData;
	SYS_Reset();
	SLIP_ResponseTx(SLIP, 0);
	return 0;
}

s32 SLIP_GetTraceRx(void *pData)
{
	SLIP_AnalyzeStruct *SLIP = (SLIP_AnalyzeStruct *)pData;
	SLIP_ResponseTx(SLIP, 1);
	return 0;
}

s32 SLIP_StartDLRx(void *pData)
{

	u32 Len;
	SLIP_AnalyzeStruct *SLIP = (SLIP_AnalyzeStruct *)pData;
	DBG("!");
	if (SLIP->InLen != 4)
	{
		SLIP_ResponseTx(SLIP, 1);
		return 0;
	}
	memcpy(&Len, SLIP->InBuf, 4);
	DBG("file %dbyte!", Len);
	__FileSet(Len);
	SLIP_ResponseTx(SLIP, 0);
	return 0;
}

s32 SLIP_DLRx(void *pData)
{
	u32 PackNum;
	SLIP_AnalyzeStruct *SLIP = (SLIP_AnalyzeStruct *)pData;
	memcpy(&PackNum, SLIP->InBuf, 4);
	DBG("file pack %d", PackNum);
	__WriteFile(SLIP->InBuf + 4, SLIP->InLen - 4);
	SLIP_ResponseTx(SLIP, 0);
	return 0;
}

s32 SLIP_GetVersionRx(void *pData)
{
	SLIP_AnalyzeStruct *SLIP = (SLIP_AnalyzeStruct *)pData;

	strcpy(SLIP->OutBuf + sizeof(SLIP_HeadStruct), DEV_VER);
	SLIP->OutLen = strlen(DEV_VER);
	SLIP_SetHead(SLIP);
	return 0;
}

s32 SLIP_TestSMSRx(void *pData)
{
	SLIP_AnalyzeStruct *SLIP = (SLIP_AnalyzeStruct *)pData;
	SLIP_ResponseTx(SLIP, 1);
	return 0;
}

const CmdFunStruct SLIPCmdFun[] =
{
		{
				SLIP_CMD_SET_UID,
				SLIP_SetUIDRx,
		},
		{
				SLIP_CMD_GET_VAR,
				SLIP_GetVarRx,
		},
		{
				SLIP_CMD_GET_PARAM,
				SLIP_GetParamRx,
		},
		{
				SLIP_CMD_SET_PARAM,
				SLIP_SetParamRx,
		},
		{
				SLIP_CMD_RESET,
				SLIP_RebootRx,
		},
		{
				SLIP_CMD_GET_TRACE,
				SLIP_GetTraceRx,
		},
		{
				SLIP_CMD_START_DOWNLOAD,
				SLIP_StartDLRx,
		},
		{
				SLIP_CMD_DL_FILE,
				SLIP_DLRx,
		},
		{
				SLIP_CMD_GET_VERSION,
				SLIP_GetVersionRx,
		},
		{
				SLIP_CMD_TEST_SMS,
				SLIP_TestSMSRx,
		},
};

u8 SLIP_CheckHead(u8 Data)
{
	if (Data == SLIP_PACK_FLAG)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

u8 SLIP_Receive(COM_CtrlStruct *COM, u8 Data)
{
	if (Data == SLIP_PACK_FLAG)
	{
		if (COM->RxPos == 1)
		{
			return 0;
		}
		COM->RxBuf[COM->RxPos++] = Data;
		COM->AnalyzeLen = COM->RxPos % COM_BUF_LEN;
		memcpy(COM->AnalyzeBuf, COM->RxBuf, COM->AnalyzeLen);
		COM->AnalyzeBuf[COM->AnalyzeLen] = 0;
		return 1;
	}
	else
	{
		COM->RxBuf[COM->RxPos++] = Data;
		return 0;
	}
}

u32 SLIP_Analyze(u8 *InBuf, u32 Len, u8 *OutBuf)
{
	u8 *Buf = COS_MALLOC(2048);
	SLIP_HeadStruct Head;
	SLIP_AnalyzeStruct SLIP;
	u32 RxLen;
	u32 TxLen;
	u16 CRC16;


	RxLen = TransferUnpack(SLIP_PACK_FLAG, SLIP_PACK_CODE, SLIP_PACK_F1, SLIP_PACK_F2, InBuf + 1, Len - 2, Buf);
	if ( (RxLen < 8) || (RxLen > (SLIP_LEN_MAX + 8)) )
	{
		DBG("%d", RxLen);
		TxLen = 0;
		goto SLIP_ANALYZE_DONE;
	}
	memcpy(&Head, Buf, sizeof(Head));
	if ( (u32)Head.Len != (RxLen - sizeof(Head)) )
	{
		DBG("len error %d %d", Head.Len, RxLen - sizeof(Head));
		if (Head.Len == 0x7878)
		{
			COMCtrl.SPFlag = 1;
			__HexTrace(InBuf, Len);
		}

		SLIP.Cmd = Head.Cmd;
		SLIP.InLen = Head.Len;
		SLIP.InBuf = Buf + sizeof(Head);
		SLIP.OutBuf = Buf;
		SLIP.OutLen = 0;
		SLIP_ResponseTx(&SLIP, 1);
		TxLen = TransferPack(SLIP_PACK_FLAG, SLIP_PACK_CODE, SLIP_PACK_F1, SLIP_PACK_F2, SLIP.OutBuf, SLIP.OutLen, OutBuf);
		goto SLIP_ANALYZE_DONE;
	}
	if (Head.Len)
	{
		CRC16 = CRC16Cal(Buf + sizeof(Head), Head.Len, CRC16_START, CRC16_GEN);
		if (CRC16 != Head.CRC16)
		{
			DBG("crc16 error %04x %04x", Head.CRC16, CRC16);
			TxLen = 0;
			goto SLIP_ANALYZE_DONE;
		}
	}


	SLIP.Cmd = Head.Cmd;
	SLIP.InLen = Head.Len;
	SLIP.InBuf = Buf + sizeof(Head);
	SLIP.OutBuf = Buf;
	SLIP.OutLen = 0;
	if ( Head.Cmd < (sizeof(SLIPCmdFun)/sizeof(CmdFunStruct)) )
	{
		SLIPCmdFun[Head.Cmd].Func(&SLIP);
		if (SLIP.OutLen)
		{
			TxLen = TransferPack(SLIP_PACK_FLAG, SLIP_PACK_CODE, SLIP_PACK_F1, SLIP_PACK_F2, SLIP.OutBuf, SLIP.OutLen, OutBuf);
		}
		else
		{
			SLIP_ResponseTx(&SLIP, 1);
			TxLen = TransferPack(SLIP_PACK_FLAG, SLIP_PACK_CODE, SLIP_PACK_F1, SLIP_PACK_F2, SLIP.OutBuf, SLIP.OutLen, OutBuf);
		}
	}
	else
	{
		SLIP_ResponseTx(&SLIP, 1);
		TxLen = TransferPack(SLIP_PACK_FLAG, SLIP_PACK_CODE, SLIP_PACK_F1, SLIP_PACK_F2, SLIP.OutBuf, SLIP.OutLen, OutBuf);
	}
SLIP_ANALYZE_DONE:
	COS_FREE(Buf);
	return TxLen;

}


