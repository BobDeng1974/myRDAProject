#include "user.h"

/*trace_on_uart OR trace_on_at*/
#define GSM_7BIT		0
#define GSM_8BIT		4
#define GSM_UCS20		0X18 //����
#define GSM_UCS2		8
#define SMS_PDU_BUF_MAX	18
typedef struct
{
	uint32_t Len;
	uint8_t Buf[184];
}SMS_PDUStruct;

typedef struct
{
	RBuffer PDUBuf;
	SMS_PDUStruct PDUData[SMS_PDU_BUF_MAX];
	uint8_t SMSBusy;
}SMS_CtrlStruct;

SMS_CtrlStruct __attribute__((section (".usr_ram"))) SMSCtrl;
/*Default Array for 7bit encode*/
const uint8_t DefaultToAsciiArray[128] =
{
    64, 163, 36, 165, 232, 233, 249, 236, 242, 199,
    10, 216, 248, 13, 197, 229, 16, 95, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 198, 230,
    223, 201, 32, 33, 34, 35, 164, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63, 161, 65, 66, 67, 68, 69,
    70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
    90, 196, 214, 209, 220, 167, 191, 97, 98, 99,
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
    110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 228, 246, 241, 252, 224
};

// 7bit����
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ����봮ָ��
// ����: Ŀ����봮����
int32_t gsmEncode7bit(const int8_t* pSrc, uint8_t* pDst, int32_t nSrcLength)
{
	uint32_t nSrc;		// Դ�ַ����ļ���ֵ
	uint32_t nDst;		// Ŀ����봮�ļ���ֵ
	uint32_t nChar;		// ��ǰ���ڴ���������ַ��ֽڵ���ţ���Χ��0-7
	uint8_t nLeft;	// ��һ�ֽڲ��������
	uint8_t gsm_char;
	uint8_t i;
	// ����ֵ��ʼ��
	nSrc = 0;
	nDst = 0;
	nChar = 0;
	nLeft = 0;
	gsm_char = 0;
	// ��Դ��ÿ8���ֽڷ�Ϊһ�飬ѹ����7���ֽ�
	// ѭ���ô�����̣�ֱ��Դ����������
	// ������鲻��8�ֽڣ�Ҳ����ȷ����
	while (nSrc < nSrcLength)
	{
		gsm_char = 0;
		for (i = 0; i < 128; i++)
		{
			if (DefaultToAsciiArray[i] == *pSrc)
			{
				gsm_char = i;
				break;
			}
		}
		// ȡԴ�ַ����ļ���ֵ�����3λ
		nChar = nSrc & 7;

		// ����Դ����ÿ���ֽ�
		if(nChar == 0)
		{
			// ���ڵ�һ���ֽڣ�ֻ�Ǳ�����������������һ���ֽ�ʱʹ��
			nLeft = gsm_char;
		}
		else
		{
			// ���������ֽڣ������ұ߲��������������ӣ��õ�һ��Ŀ������ֽ�
			*pDst = (gsm_char << (8-nChar)) | nLeft;

			// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
			nLeft = gsm_char >> nChar;

			// �޸�Ŀ�괮��ָ��ͼ���ֵ
			pDst++;
			nDst++;
		}

		// �޸�Դ����ָ��ͼ���ֵ
		pSrc++;
		nSrc++;
	}

	// ����Ŀ�괮����
	if (!nChar)
	{
		*pDst = gsm_char & 0x7f;
		return nDst + 1;
	}
	else if (nChar == 6)
	{
		return nDst;
	}
	else
	{
		*pDst = nLeft;
		return nDst + 1;
	}

}

// 7bit����
// ����: pSrc - Դ���봮ָ��
//       nSrcLength - Դ���봮����
// ���: pDst - Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int32_t gsmDecode7bit(const uint8_t* pSrc, int8_t* pDst, int32_t nSrcLength)
{
	int32_t nSrc;		// Դ�ַ����ļ���ֵ
	int32_t nDst;		// Ŀ����봮�ļ���ֵ
	int32_t nByte;		// ��ǰ���ڴ���������ֽڵ���ţ���Χ��0-6
	uint8_t nLeft;	// ��һ�ֽڲ��������

	// ����ֵ��ʼ��
	nSrc = 0;
	nDst = 0;

	// �����ֽ���źͲ������ݳ�ʼ��
	nByte = 0;
	nLeft = 0;

	// ��Դ����ÿ7���ֽڷ�Ϊһ�飬��ѹ����8���ֽ�
	// ѭ���ô�����̣�ֱ��Դ���ݱ�������
	// ������鲻��7�ֽڣ�Ҳ����ȷ����
	while(nSrc<nSrcLength)
	{
		// ��Դ�ֽ��ұ߲��������������ӣ�ȥ�����λ���õ�һ��Ŀ������ֽ�
		*pDst = ((*pSrc << nByte) | nLeft) & 0x7f;
		// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
		nLeft = *pSrc >> (7-nByte);

		// �޸�Ŀ�괮��ָ��ͼ���ֵ
		pDst++;
		nDst++;

		// �޸��ֽڼ���ֵ
		nByte++;

		// ����һ������һ���ֽ�
		if(nByte == 7)
		{
			// ����õ�һ��Ŀ������ֽ�
			*pDst = nLeft;
			// �޸�Ŀ�괮��ָ��ͼ���ֵ
			pDst++;
			nDst++;

			// �����ֽ���źͲ������ݳ�ʼ��
			nByte = 0;
			nLeft = 0;
		}

		// �޸�Դ����ָ��ͼ���ֵ
		pSrc++;
		nSrc++;
	}

	// ����ַ����Ӹ�������
	*pDst = '\0';

	// ����Ŀ�괮����
	return nDst;
}

// 7bit�������תascii��
// ����: pSrc - Դ���봮ָ��
//       nSrcLength - Դ���봮����
// ���: pDst - Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int32_t gsm7bit2ascii(const int8_t* pSrc, uint8_t* pDst, int32_t nSrcLength)
{
	int32_t i = 0;
	int32_t ret = 0;
	uint8_t flag_27 = 0;
	while (i < nSrcLength) {
		if (*pSrc == 27) {
			flag_27 = 0;
			switch (*(pSrc+1)) {
			case 10:
				*pDst = 12;
				flag_27 = 1;
				break;
			case 20:
				*pDst = 94;
				flag_27 = 1;
				break;
			case 40:
				*pDst = 123;
				flag_27 = 1;
				break;
			case 41:
				*pDst = 125;
				flag_27 = 1;
				break;
			case 47:
				*pDst = 92;
				flag_27 = 1;
				break;
			case 60:
				*pDst = 91;
				flag_27 = 1;
				break;
			case 61:
				*pDst = 126;
				flag_27 = 1;
				break;
			case 62:
				*pDst = 93;
				flag_27 = 1;
				break;
			case 64:
				*pDst = 124;
				flag_27 = 1;
				break;
			case 101:
				*pDst = 164;
				flag_27 = 1;
				break;
			}
			if (flag_27) {
				pSrc+=2;
				i+=2;
			} else {
				*pDst = DefaultToAsciiArray[(uint8_t)(*pSrc)];
				pSrc++;
				i++;
			}
		} else {
			*pDst = DefaultToAsciiArray[(uint8_t)(*pSrc)];
			pSrc++;
			i++;
		}
		pDst++;
		ret++;
	}
	*pDst = 0;
	return ret;
}

// 8bit����
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ����봮ָ��
// ����: Ŀ����봮����
int32_t gsmEncode8bit(const int8_t* pSrc, uint8_t* pDst, int32_t nSrcLength)
{
	// �򵥸���
	memcpy(pDst, pSrc, nSrcLength);

	return nSrcLength;
}

// 8bit����
// ����: pSrc - Դ���봮ָ��
//       nSrcLength -  Դ���봮����
// ���: pDst -  Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int32_t gsmDecode8bit(const uint8_t* pSrc, int8_t* pDst, int32_t nSrcLength)
{
	// �򵥸���
	memcpy(pDst, pSrc, nSrcLength);

	// ����ַ����Ӹ�������
	*(pDst+nSrcLength) = '\0';

	return nSrcLength;
}

// PDU���룬���ڱ��ơ����Ͷ���Ϣ
// ����: pSrc - ԴPDU����ָ��
// ���: pDst - Ŀ��PDU��ָ��
// ����: Ŀ��PDU������
int32_t gsmEncodePdu(uint8_t *ToNumber, uint8_t ToNumberLen, uint8_t* UserData, uint32_t Len, uint8_t DCS)
{
	uint8_t ucTemp;
	uint32_t dwTemp;
	uint32_t DummyLen;
	uint8_t UDLen;
	uint8_t Pos;
	uint16_t *pwTemp;
	SMS_PDUStruct PDU;
	// SMSC��ַ��Ϣ��
	PDU.Len = gSys.SMSParam.nNumber[0] + 1;
	memcpy(&PDU.Buf[0], gSys.SMSParam.nNumber, PDU.Len);

	ucTemp = (ToNumberLen / 2) + (ToNumberLen % 2);
	PDU.Buf[PDU.Len++] = 0x31;// �Ƿ��Ͷ���(TP-MTI=01)��TP-VP����Ը�ʽ(TP-VPF=10) TP-SRR=1
	PDU.Buf[PDU.Len++] = 0;// TP-MR=0
	PDU.Buf[PDU.Len++] = ToNumberLen;
	ReverseBCD(ToNumber, ToNumber, ucTemp);
	if (ToNumber[0] == 0x01)
	{
		PDU.Buf[PDU.Len++] = 0x81;
	}
	else
	{
		PDU.Buf[PDU.Len++] = 0x91;
	}
	memcpy(&PDU.Buf[PDU.Len], ToNumber, ucTemp);
	PDU.Len += ucTemp;
	PDU.Buf[PDU.Len++] = 0;// Э���ʶ(TP-PID)
	PDU.Buf[PDU.Len++] = DCS;// �û���Ϣ���뷽ʽ(TP-DCS)
	PDU.Buf[PDU.Len++] = 0;// ��Ч��(TP-VP)Ϊ5����

	if ( (DCS == GSM_UCS2)||(DCS == GSM_UCS20) )
	{
		pwTemp = COS_MALLOC(Len * 2 + 2);
		dwTemp = OS_GB2312ToUCS2(UserData, (uint8_t *)pwTemp, Len, 0);
		if (dwTemp % 2)
		{
			COS_FREE(pwTemp);
			return -1;
		}
		Pos = PDU.Len;
		DummyLen = 0;
		while (DummyLen < dwTemp)
		{
			PDU.Len = Pos;
			if ((dwTemp - DummyLen) > 140)
			{
				UDLen = 140;
			}
			else
			{
				UDLen = dwTemp - DummyLen;
			}
			PDU.Buf[PDU.Len++] = UDLen;
			memcpy(PDU.Buf + PDU.Len, (uint8_t *)&pwTemp[DummyLen/2], UDLen);
			DummyLen += UDLen;
			PDU.Len += UDLen;
			WriteRBufferForce(&SMSCtrl.PDUBuf, (uint8_t *)&PDU, 1);
		}
		COS_FREE(pwTemp);
	}
	else if (DCS == GSM_7BIT)
	{
		if (Len > 160)
		{
			DBG("too much, unsupport");
			return 0;
		}
		PDU.Buf[PDU.Len++] = Len;
		UDLen = gsmEncode7bit(UserData, PDU.Buf + PDU.Len, Len);
		PDU.Len += UDLen;
		WriteRBufferForce(&SMSCtrl.PDUBuf, (uint8_t *)&PDU, 1);
	}
	else
	{
		Pos = PDU.Len;
		DummyLen = 0;
		while (DummyLen < Len)
		{
			PDU.Len = Pos;
			if ((Len - DummyLen) > 140)
			{
				UDLen = 140;
			}
			else
			{
				UDLen = Len - DummyLen;
			}
			PDU.Buf[PDU.Len++] = UDLen;
			memcpy(PDU.Buf + PDU.Len, UserData + DummyLen, UDLen);
			DummyLen += UDLen;
			PDU.Len += UDLen;
			WriteRBufferForce(&SMSCtrl.PDUBuf, (uint8_t *)&PDU, 1);

		}
	}
	return 0;
}

// PDU���룬���ڽ��ա��Ķ�����Ϣ
// ����: pSrc - ԴPDU��ָ��
// ���: pDst - Ŀ��PDU����ָ��
// ����: �û���Ϣ������
uint8_t gsmDecodePdu(uint8_t *pSrc, uint32_t SrcLen, uint8_t *pDst, uint8_t *FromNumber, uint8_t *NumberLen)
{
	uint8_t nDstLength = 0;			// Ŀ��PDU������
	uint32_t Pos = 0;
	uint8_t Temp[170];
	uint8_t NumberType;
	uint32_t TempLen;
	uint8_t TPPID, TPDCS;
	Pos += pSrc[0] + 1;//����SMSC
	//DBG("base param %02x", pSrc[Pos]);
	if (pSrc[Pos] & (1 << 6))
	{
		DBG("long sms, quit!");
		return 0;
	}

	Pos++;
	*NumberLen = pSrc[Pos++];
	TempLen = (*NumberLen / 2) + (*NumberLen % 2);
	NumberType = pSrc[Pos++];
	//HexTrace(&pSrc[Pos], NumberLen);
	memcpy(FromNumber, &pSrc[Pos], TempLen);
	ReverseBCD(FromNumber, FromNumber, TempLen);
	Pos += TempLen;

	TPPID = pSrc[Pos++];
	TPDCS = pSrc[Pos++];
	Pos += 7; //����ʱ���
	// TPDU��Э���ʶ�����뷽ʽ���û���Ϣ��

	nDstLength = pSrc[Pos++];
	TempLen = SrcLen - Pos;
	//DBG("%u %u", TempLen, nDstLength);
	if(TPDCS == GSM_7BIT)
	{
		// 7-bit����
		TempLen = gsmDecode7bit(pSrc + Pos, Temp, TempLen);	// ת����TP-DU
		nDstLength = gsm7bit2ascii(Temp, pDst, nDstLength);
		pDst[nDstLength] = 0;
		//DBG("%s", pDst);
	}
	else if((TPDCS == GSM_UCS2)||(TPDCS == GSM_UCS20))
	{
		nDstLength = OS_UCS2ToGB2312(pSrc + Pos, pDst, TempLen, 1);
		//HexTrace(pDst, nDstLength);
	}
	else
	{
		nDstLength = gsmDecode8bit(pSrc + Pos, pDst, nDstLength);	// ת����TP-DU
		pDst[nDstLength] = 0;
		//DBG("%s", pDst);
	}

	// ����Ŀ���ַ�������
	return nDstLength;
}

void SMS_Config(void)
{
	InitRBuffer(&SMSCtrl.PDUBuf, (uint8_t *)SMSCtrl.PDUData, SMS_PDU_BUF_MAX, sizeof(SMS_PDUStruct));
}

void SMS_Receive(CFW_NEW_SMS_NODE *pNewMsgNode)
{
	CFW_SMS_PDU_INFO *pPduNodeInfo = NULL;
	uint8_t *pPduData        = NULL;
	uint8_t *Response;
	uint32_t ResponseLen;
	uint8_t FromData[172];
	uint8_t nDstLen;
	uint8_t NumberLen;
	uint8_t FromNumber[8];
	switch (pNewMsgNode->nType)
	{
		// ///////////////////////////////////
		// PDU Message Content
		// ///////////////////////////////////
	case 0x20:
		pPduNodeInfo = (CFW_SMS_PDU_INFO *)(pNewMsgNode->pNode);
		pPduData = pPduNodeInfo->pData;
		nDstLen = gsmDecodePdu(pPduData, pPduNodeInfo->nDataSize, FromData, FromNumber, &NumberLen);
		DBG("from %u", NumberLen);
		HexTrace(FromNumber, 7);
		DBG("%s", FromData);
		Response = COS_MALLOC(512);
		LV_SMSAnalyze(FromData, strlen(FromData), Response, &ResponseLen);
		if (ResponseLen)
		{
			//DBG("%u %s", ResponseLen, Response);
			SMS_Send(FromNumber, NumberLen, Response, ResponseLen, GSM_7BIT);
		}
		COS_FREE(Response);
		break;
		// ///////////////////////////////////
		// PDU status report
		// ///////////////////////////////////
	case 0x40:
		pPduNodeInfo = (CFW_SMS_PDU_INFO *)(pNewMsgNode->pNode);
		pPduData = pPduNodeInfo->pData;
		DBG("get report");
		HexTrace(pPduData, pPduNodeInfo->nDataSize);
		break;
	default:
		DBG("unsupport %02x!", pNewMsgNode->nType);
		break;

	}
}

void SMS_Send(uint8_t *ToNumber, uint8_t ToNumberLen, uint8_t* UserData, uint32_t Len, uint8_t DCS)
{
	gsmEncodePdu(ToNumber, ToNumberLen, UserData, Len, DCS);
	SMS_Submit();
}

void SMS_Submit(void)
{
	SMS_PDUStruct PDU;
	if (SMSCtrl.SMSBusy || !SMSCtrl.PDUBuf.Len)
	{
		return;
	}
	SMSCtrl.SMSBusy = 1;
	ReadRBuffer(&SMSCtrl.PDUBuf, &PDU, 1);
	HexTrace(PDU.Buf, PDU.Len);
	OS_SMSTxByPDU(PDU.Buf, PDU.Len);
}

void SMS_SendFree(void)
{
	SMSCtrl.SMSBusy = 0;
}
