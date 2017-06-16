#include "CApi.h"

#if 1
/*****************************************************************************
* FUNCTION
*   command_parse_param()
* DESCRIPTION
*    Parse AT command string to parameters
* PARAMETERS
*   char* pStr
* RETURNS
*  pCmdParam
*****************************************************************************/
u32 CmdParseParam(s8* pStr, CmdParam *CP, s8 Cut)
{
	u32 paramStrLen = strlen((char *)pStr);
	u32 paramIndex = 0;
	u32 paramCharIndex = 0;
	u32 index = 0;

	while ((pStr[index] != '\r')
		&& (index < paramStrLen)
		&& (paramIndex < CP->param_max_num)) {
		if (pStr[index] == Cut) {
			/* Next param string */
			paramCharIndex = 0;
			paramIndex++;
		}
		else {
			if (pStr[index] != '"')
			{
				if (paramCharIndex >= CP->param_max_len)
					return (0);

				/*Get each of command param char, the param char except char ' " '*/
				CP->param_str[paramIndex * CP->param_max_len + paramCharIndex] = pStr[index];
				paramCharIndex++;
			}
		}
		index++;
	}

	CP->param_num = paramIndex + 1;

	return (1);
}

void UnicodeToAsciiN(u16 *Src, u8 *Dst, u32 Len)
{
	u32 i;
	for (i = 0; i < Len; i++)
	{
		Dst[i] = Src[i];
	}
}

void AsciiToUnicodeN(u8 *Src, u16 *Dst, u32 Len)
{
	u32 i;
	for (i = 0; i < Len; i++)
	{
		Dst[i] = Src[i];
		if (!Src[i])
			break;
	}
}

u8 IsDigitStr(const u8 *Src, u32 Len)
{
	u32 i = 0;
	while (i < Len){
		if (!IsDigit(Src[i])) {
			return 0;
		}
		i++;
	}
	return 1;
}

void ReverseBCD(u8 *Src, u8 *Dst, u32 Len)
{
	u32 i;
	for (i = 0; i < Len; i++)
	{
		Dst[i] = (Src[i] >> 4) | (Src[i] << 4);
	}
}

u32 AsciiToHex(u8 *Src, u32 Len, u8 *Dst)
{
	u32 i = 0;
	u32 j = 0;
	u8 hex_temp;
	while (i < Len) {
		hex_temp = 0;
		if (IsDigit(Src[i]))
		{
			hex_temp = Src[i++] - '0';
		}
		else if (Src[i] > 'Z')
		{
			hex_temp = Src[i++] - 'a' + 10;
		}
		else
		{
			hex_temp = Src[i++] - 'A' + 10;
		}
		hex_temp <<= 4;
		if (IsDigit(Src[i]))
		{
			hex_temp += Src[i++] - '0';
		}
		else if (Src[i] > 'Z')
		{
			hex_temp += Src[i++] - 'a' + 10;
		}
		else
		{
			hex_temp += Src[i++] - 'A' + 10;
		}
		Dst[j++] = hex_temp;
	}
	return j;
}

u32 AsciiToU32(u8 *Src, u32 Len)
{
	u32 i = 0;
	u32 Temp = 0;
	for (i = 0; i < Len; i++)
	{

		if (Src[i])
		{
			Temp *= 10;
			Temp += Src[i] - '0';
		}
		else
		{
			break;
		}
	}
	return Temp;
}

double AsciiToFloat(u8 *Src)
{

	double s=0.0;

	double d=10.0;
	int jishu=0;

	bool falg=0;

	while(*Src==' ')
	{
		Src++;
	}

	if(*Src=='-')//��¼��������
	{
		falg=1;
		Src++;
	}

	if(!(*Src>='0'&&*Src<='9'))//���һ��ʼ���������˳�������0.0
		return s;

	while(*Src>='0'&&*Src<='9'&&*Src!='.')//����С����ǰ��������
	{
		s=s*10.0+*Src-'0';
		Src++;
	}

	if(*Src=='.')//�Ժ�ΪС������
		Src++;

	while(*Src>='0'&&*Src<='9')//����С������
	{
		s=s+(*Src-'0')/d;
		d*=10.0;
		Src++;
	}

//	if(*Src=='e'||*Src=='E')//���ǿ�ѧ������
//	{
//		Src++;
//		if(*Src=='+')
//		{
//			Src++;
//			while(*Src>='0'&&*Src<='9')
//			{
//				jishu=jishu*10+*Src-'0';
//				Src++;
//			}
//			while(jishu>0)
//			{
//				s*=10;
//				jishu--;
//			}
//		}
//		if(*Src=='-')
//		{
//			Src++;
//			while(*Src>='0'&&*Src<='9')
//			{
//				jishu=jishu*10+*Src-'0';
//				Src++;
//			}
//			while(jishu>0)
//			{
//				s/=10;
//				jishu--;
//			}
//		}
//	}

	return s*(falg?-1.0:1.0);

}

void IntToBCD(u32 Src, u8 *Dst, u8 Len)
{
	u8 i, j, k;
	u8 Temp[64];
	memset(Dst, 0, Len);
	memset(Temp, 0, 64);
	j = 0;
	while (Src)
	{
		Temp[j] = Src % 10;
		Src = Src/10;
		j++;
	}
	k = 0;
	for (i = Len; i > 0; i--)
	{
		Dst[i - 1] = Temp[k] + (Temp[k + 1] << 4);
		k += 2;
		if (k >= j)
		{
			break;
		}
	}
}

void LongToBCD(u64 Src, u8 *Dst, u8 Len)
{
	u8 i, j, k;
	u8 Temp[64];
	memset(Dst, 0, Len);
	memset(Temp, 0, 64);
	j = 0;
	while (Src)
	{
		Temp[j] = Src % 10;
		Src = Src/10;
		j++;
	}
	k = 0;
	for (i = Len - 1; i >= 0; i--)
	{
		Dst[i] = Temp[k] + (Temp[k + 1] << 4);
		k += 2;
		if (k >= j)
		{
			break;
		}
	}
}

u32 BCDToInt(u8 *Src, u8 Len)
{
	u32 Result = 0;
	u8 Temp, i;
	for (i = 0; i < Len; i++)
	{
		Result *= 100;
		Temp = (Src[i] >> 4) * 10 + (Src[i] & 0x0f);
		Result += Temp;
	}
	return Result;
}

u32 HexToAscii(u8 *Src, u32 Len, u8 *Dst)
{
	u32 i = 0;
	u32 j = 0;
	u8 hex_temp;
	while (i < Len) {
		hex_temp = (Src[i] & 0xf0) >> 4;
		if (hex_temp >= 10) {
			Dst[j++] = hex_temp - 10 + 'A';
		}
		else {
			Dst[j++] = hex_temp + '0';
		}
		hex_temp = Src[i++] & 0x0f;
		if (hex_temp >= 10) {
			Dst[j++] = hex_temp - 10 + 'A';
		}
		else {
			Dst[j++] = hex_temp + '0';
		}

	}
	return j;
}

u32 StrToUint(const u8 *Src)
{
	u32 hex_temp = 0;
	u32 i;
	for (i = 0; i < 8; i++)
	{
		if (Src[i])
		{
			if (i)
			{
				hex_temp <<= 4;
			}
			if (IsDigit(Src[i]))
			{
				hex_temp += Src[i] - '0';
			}
			else if (Src[i] >= 'a' && Src[i] <= 'f')
			{
				hex_temp += Src[i] - 'a' + 10;
			}
			else if (Src[i] >= 'A' && Src[i] <= 'Z')
			{
				hex_temp += Src[i] - 'A' + 10;
			}
			else
			{
				return 0;
			}

		}
		else
		{
			return hex_temp;
		}
	}
	return hex_temp;
}

/************************************************************************/
/*ʱ����ʱ���ת����C����ʵ��                                                                    */
/************************************************************************/
u8 IsLeapYear(u32 Year)
{
	if ((((Year % 4) == 0) && ((Year % 100) != 0)) || ((Year % 400) == 0))
		return 1;
	else
		return 0;
}
#ifdef L64_SUPPORT
u64 UTC2Tamp(Date_UserDataStruct *Date, Time_UserDataStruct *Time)
{
	u32 DayTable[2][12] = { { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 }, { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 } };
	u64 DYear, DDay, DSec;

	DYear = Date->Year - 1970;
	if (DYear)	//1970���Ժ�,1972�ǵ�һ������
	{
		DDay = DYear * 365 + ((DYear + 2) / 4) - ((DYear + 2) / 100) + ((DYear + 2) / 400) + DayTable[IsLeapYear(Date->Year)][Date->Mon - 1] + (Date->Day - 1);
	}
	else
	{
		DDay = DayTable[IsLeapYear(Date->Year)][Date->Mon - 1] + (Date->Day - 1);
	}
	DSec = DDay * 86400 + Time ->Hour * 3600 + Time->Min * 60 + Time->Sec;
	return DSec;
}

u32 Tamp2UTC(u64 Sec, Date_UserDataStruct *Date, Time_UserDataStruct *Time, u32 LastDDay)
{
	u32 DayTable[2][12] = { { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 }, { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 } };
	u32 DYear, LDYear,  i, LeapFlag;
	u32 DDay;
	DDay = Sec / 86400;

	if (DDay != LastDDay)
	{
		Time->Week = (4 + DDay) % 7;
		LDYear = DDay / 365;
		if (DDay >= (LDYear * 365 + ((LDYear + 2) / 4) - ((LDYear + 2) / 100) + ((LDYear + 2) / 400)))
		{
			DYear = LDYear;
		}
		else
		{
			DYear = LDYear - 1;
		}
		Date->Year = DYear + 1970;
		LeapFlag = IsLeapYear(DYear + 1970);
		if (Date->Year > 1970)
		{
			DDay -= (DYear * 365 + ((DYear + 2) / 4) - ((DYear + 2) / 100) + ((DYear + 2) / 400));
		}

		//gPrintMsg.Trace("%d\r\n", DDay);
		Date->Mon = 12;
		for (i = 1; i < 12; i++)
		{
			if (DDay < DayTable[LeapFlag][i])
			{
				Date->Mon = i;
				break;
			}
		}
		Date->Day = DDay - DayTable[LeapFlag][Date->Mon - 1] + 1;
	}

	Sec = Sec % 86400;
	Time->Hour = Sec / 3600;
	Sec = Sec % 3600;
	Time->Min = Sec / 60;
	Time->Sec = Sec % 60;
	return DDay;
}

u8 XorCheck(u8 *Data, u32 Len, u8 CheckStart)
{
	u8 Check = CheckStart;
	u32 i;
	for (i = 0; i < Len; i++)
	{
		Check ^= Data[i];
	}
	return Check;
}

///**
//* @brief  ��ת����
//* @param  ref ��Ҫ��ת�ı���
//* @param	ch ��ת���ȣ�����λ
//* @retval N��ת�������
//*/
//static u64 Reflect(u64 ref, u8 ch)
//{
//	unsigned long long value = 0;
//	u32 i;
//	for (i = 1; i< (ch + 1); i++)
//	{
//		if (ref & 1)
//			value |= (u64)1 << (ch - i);
//		ref >>= 1;
//	}
//	return value;
//}
//
///**
//* @brief  ����CRC32�Ĳ�ѯ��
//* @param  Tab ����
//* @param	Gen CRC32��
//* @retval None
//*/
//void CRC32_CreateTable(u32 *Tab, u32 Gen)
//{
//	u32 crc;
//	u32 i, j, temp, t1, t2, flag;
//	if (Tab[1] != 0)
//		return;
//	for (i = 0; i < 256; i++)
//	{
//		temp = Reflect(i, 8);
//		Tab[i] = temp << 24;
//		for (j = 0; j < 8; j++)
//		{
//			flag = Tab[i] & 0x80000000;
//			t1 = Tab[i] << 1;
//			if (0 == flag)
//			{
//				t2 = 0;
//			}
//			else
//			{
//				t2 = Gen;
//			}
//			Tab[i] = t1 ^ t2;
//		}
//		crc = Tab[i];
//		Tab[i] = Reflect(crc, 32);
//	}
//}
//
//
///**
//* @brief  ����buffer��crcУ����
//* @param  CRC32_Table CRC32��
//* @param  Buf ����
//* @param	Size ����������
//* @param	CRC32 ��ʼCRC32ֵ
//* @retval ������CRC32
//*/
//u32 CRC32_Cal(u32 *CRC32_Table, u8 *Buf, u32 Size, u32 CRC32Last)
//{
//	u32 i;
//	for (i = 0; i < Size; i++)
//	{
//		CRC32Last = CRC32_Table[(CRC32Last ^ Buf[i]) & 0xff] ^ (CRC32Last >> 8);
//	}
//	return CRC32Last;
//}
#endif
/************************************************************************/
/*  CRC16                                                                */
/************************************************************************/
u16 CRC16Cal(u8 *Src, u16 Len, u16 CRC16Last, u16 CRCRoot)
{
	u8 i;
	u16 CRC16 = CRC16Last;

	while (Len--)
	{
		for (i = 0x80; i != 0; i >>= 1)
		{
			if ((CRC16 & 0x8000) != 0)
			{
				CRC16 <<= 1;
				CRC16 ^= CRCRoot;
			}
			else
			{
				CRC16 <<= 1;
			}
			if ((*Src&i) != 0)
			{
				CRC16 ^= CRCRoot;
			}
		}
		Src++;
	}

	return CRC16;
}

#endif


void InitRBuffer(RBuffer *Buf, u8 *Data, u32 MaxLen, u32 DataSize)
{
	Buf->Data = Data;
	Buf->Len = 0;
	Buf->MaxLength = MaxLen;
	Buf->Offset = 0;
	Buf->DataSize = DataSize;
}

u32 QueryRBuffer(RBuffer *Buf, u8 *Data, u32 Len)
{
	u32 i, p;

	if (Buf->Len < Len)
	{
		Len = Buf->Len;
	}
	if (Buf->DataSize > 1)
	{
		for (i = 0, p = Buf->Offset; i < Len; i++, p++)
		{
			if (p >= Buf->MaxLength)
			{
				p -= Buf->MaxLength;
			}
			memcpy(Data + (i * Buf->DataSize), Buf->Data + (p * Buf->DataSize), Buf->DataSize);
		}
	} 
	else
	{
		for (i = 0, p = Buf->Offset; i < Len; i++, p++)
		{
			if (p >= Buf->MaxLength)
			{
				p -= Buf->MaxLength;
			}
			Data[i] = Buf->Data[p];
		}
	}



	//for query, don't update r_buffer control struct.
	//buf->len -= len;
	//buf->offset = p;

	return Len;
}

u32 ReadRBuffer(RBuffer *Buf, u8 *Data, u32 Len)
{
	u32 l;

	l = QueryRBuffer(Buf, Data, Len);
	Buf->Len -= l;
	Buf->Offset += l;
	if (Buf->Offset > Buf->MaxLength)
	{
		Buf->Offset -= Buf->MaxLength;

	}
	return l;
}

void DelRBuffer(RBuffer *Buf, u32 Len)
{
	if (Buf->Len < Len)
	{
		Len = Buf->Len;
	}

	Buf->Len -= Len;
	Buf->Offset += Len;
	if (Buf->Offset > Buf->MaxLength)
	{
		Buf->Offset -= Buf->MaxLength;
	}
		
	if (Buf->Len == 0) {
		Buf->Offset = 0;
	}
}

u32 WriteRBufferForce(RBuffer *Buf, u8 *Data, u32 Len)
{
	u32 i, p, cut_off = 0;
	cut_off = Buf->MaxLength - Buf->Len;
	if (cut_off >= Len)
	{
		cut_off = 0;
	}		
	else
	{
		cut_off = Len - cut_off;
	}

	if (Buf->DataSize > 1)
	{
		for (i = 0, p = Buf->Offset + Buf->Len; i < Len; i++, p++)
		{
			if (p >= Buf->MaxLength)
			{
				p -= Buf->MaxLength;
			}
			memcpy(Buf->Data + (p * Buf->DataSize), Data + (i * Buf->DataSize), Buf->DataSize);
		}
	} 
	else
	{
		for (i = 0, p = Buf->Offset + Buf->Len; i < Len; i++, p++)
		{
			if (p >= Buf->MaxLength)
			{
				p -= Buf->MaxLength;
			}

			Buf->Data[p] = Data[i];
		}
	}


	Buf->Offset += cut_off;
	if (Buf->Offset >= Buf->MaxLength)
		Buf->Offset -= Buf->MaxLength;

	Buf->Len += Len;
	if (Buf->Len > Buf->MaxLength)
		Buf->Len = Buf->MaxLength;

	return Len;
		
}


/*
 * ת����
 * ��ʶFlag������ͷ��β����Flag
 * ����������Flag -> Code F1
 * ����������Code -> Code F2
 */

u32 TransferPack(u8 Flag, u8 Code, u8 F1, u8 F2, u8 *InBuf, u32 Len, u8 *OutBuf)
{
	u32 TxLen = 0;
	u32 i;
	OutBuf[0] = Flag;
	TxLen = 1;
	for (i = 0; i < Len; i++)
	{
		if (InBuf[i] == Flag)
		{
			OutBuf[TxLen++] = Code;
			OutBuf[TxLen++] = F1;
		}
		else if (InBuf[i] == Code)
		{
			OutBuf[TxLen++] = Code;
			OutBuf[TxLen++] = F2;
		}
		else
		{
			OutBuf[TxLen++] = InBuf[i];
		}
	}
	OutBuf[TxLen++] = Flag;
	return TxLen;
}

/*
 * ת����
 * ��ʶFlag������ͷ��β����Flag
 * ����������Code F1 -> Flag
 * ����������Code F2 -> Code
 * ����������Flag ������0
 */

u32 TransferUnpack(u8 Flag, u8 Code, u8 F1, u8 F2, u8 *InBuf, u32 Len, u8 *OutBuf)
{
	u32 RxLen = 0;
	u32 i = 0;
	while (i < Len)
	{
		if (InBuf[i] == Code)
		{
			if (InBuf[i+1] == F1)
			{
				OutBuf[RxLen++] = Flag;
			}
			else if (InBuf[i+1] == F2)
			{
				OutBuf[RxLen++] = Code;
			}
			else
			{
				return 0;
			}
			i += 2;
		}
		else if (InBuf[i] == Flag)
		{
			return 0;
		}
		else
		{
			OutBuf[RxLen++] = InBuf[i++];
		}
	}
	return RxLen;
}

#if 0
/************************************************************************/
/* MD5���                                                                     */
/************************************************************************/
static u8 PADDING[] = { 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

#define F(x,y,z) ((x & y) | (~x & z))  
#define G(x,y,z) ((x & z) | (y & ~z))  
#define H(x,y,z) (x^y^z)  
#define I(x,y,z) (y ^ (x | ~z))  
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))  
#define FF(a,b,c,d,x,s,ac) { a += F(b, c, d) + x + ac; a = ROTATE_LEFT(a, s); a += b; }
#define GG(a,b,c,d,x,s,ac) { a += G(b, c, d) + x + ac; a = ROTATE_LEFT(a, s); a += b; }
#define HH(a,b,c,d,x,s,ac) { a += H(b, c, d) + x + ac; a = ROTATE_LEFT(a, s); a += b; }
#define II(a,b,c,d,x,s,ac) { a += I(b, c, d) + x + ac; a = ROTATE_LEFT(a, s); a += b; }

void MD5Init(MD5_CTX *context)
{
	context->count[0] = 0;
	context->count[1] = 0;
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
}
void MD5Update(MD5_CTX *context, u8 *input, u32 inputlen)
{
	u32 i = 0, index = 0, partlen = 0;
	index = (context->count[0] >> 3) & 0x3F;
	partlen = 64 - index;
	context->count[0] += inputlen << 3;
	if (context->count[0] < (inputlen << 3))
		context->count[1]++;
	context->count[1] += inputlen >> 29;

	if (inputlen >= partlen)
	{
		memcpy(&context->buffer[index], input, partlen);
		MD5Transform(context->state, context->buffer);
		for (i = partlen; i + 64 <= inputlen; i += 64)
			MD5Transform(context->state, &input[i]);
		index = 0;
	}
	else
	{
		i = 0;
	}
	memcpy(&context->buffer[index], &input[i], inputlen - i);
}
void MD5Final(MD5_CTX *context, u8 digest[16])
{
	u32 index = 0, padlen = 0;
	u8 bits[8];
	index = (context->count[0] >> 3) & 0x3F;
	padlen = (index < 56) ? (56 - index) : (120 - index);
	MD5Encode(bits, context->count, 8);
	MD5Update(context, PADDING, padlen);
	MD5Update(context, bits, 8);
	MD5Encode(digest, context->state, 16);
}
void MD5Encode(u8 *output, u32 *input, u32 len)
{
	u32 i = 0, j = 0;
	while (j < len)
	{
		output[j] = input[i] & 0xFF;
		output[j + 1] = (input[i] >> 8) & 0xFF;
		output[j + 2] = (input[i] >> 16) & 0xFF;
		output[j + 3] = (input[i] >> 24) & 0xFF;
		i++;
		j += 4;
	}
}
void MD5Decode(u32 *output, u8 *input, u32 len)
{
	u32 i = 0, j = 0;
	while (j < len)
	{
		output[i] = (input[j]) |
			(input[j + 1] << 8) |
			(input[j + 2] << 16) |
			(input[j + 3] << 24);
		i++;
		j += 4;
	}
}
void MD5Transform(u32 state[4], u8 block[64])
{
	u32 a = state[0];
	u32 b = state[1];
	u32 c = state[2];
	u32 d = state[3];
	u32 x[64];
	MD5Decode(x, block, 64);
	FF(a, b, c, d, x[0], 7, 0xd76aa478);
	FF(d, a, b, c, x[1], 12, 0xe8c7b756);
	FF(c, d, a, b, x[2], 17, 0x242070db);
	FF(b, c, d, a, x[3], 22, 0xc1bdceee);
	FF(a, b, c, d, x[4], 7, 0xf57c0faf);
	FF(d, a, b, c, x[5], 12, 0x4787c62a);
	FF(c, d, a, b, x[6], 17, 0xa8304613);
	FF(b, c, d, a, x[7], 22, 0xfd469501);
	FF(a, b, c, d, x[8], 7, 0x698098d8);
	FF(d, a, b, c, x[9], 12, 0x8b44f7af);
	FF(c, d, a, b, x[10], 17, 0xffff5bb1);
	FF(b, c, d, a, x[11], 22, 0x895cd7be);
	FF(a, b, c, d, x[12], 7, 0x6b901122);
	FF(d, a, b, c, x[13], 12, 0xfd987193);
	FF(c, d, a, b, x[14], 17, 0xa679438e);
	FF(b, c, d, a, x[15], 22, 0x49b40821);


	GG(a, b, c, d, x[1], 5, 0xf61e2562);
	GG(d, a, b, c, x[6], 9, 0xc040b340);
	GG(c, d, a, b, x[11], 14, 0x265e5a51);
	GG(b, c, d, a, x[0], 20, 0xe9b6c7aa);
	GG(a, b, c, d, x[5], 5, 0xd62f105d);
	GG(d, a, b, c, x[10], 9, 0x2441453);
	GG(c, d, a, b, x[15], 14, 0xd8a1e681);
	GG(b, c, d, a, x[4], 20, 0xe7d3fbc8);
	GG(a, b, c, d, x[9], 5, 0x21e1cde6);
	GG(d, a, b, c, x[14], 9, 0xc33707d6);
	GG(c, d, a, b, x[3], 14, 0xf4d50d87);
	GG(b, c, d, a, x[8], 20, 0x455a14ed);
	GG(a, b, c, d, x[13], 5, 0xa9e3e905);
	GG(d, a, b, c, x[2], 9, 0xfcefa3f8);
	GG(c, d, a, b, x[7], 14, 0x676f02d9);
	GG(b, c, d, a, x[12], 20, 0x8d2a4c8a);


	HH(a, b, c, d, x[5], 4, 0xfffa3942);
	HH(d, a, b, c, x[8], 11, 0x8771f681);
	HH(c, d, a, b, x[11], 16, 0x6d9d6122);
	HH(b, c, d, a, x[14], 23, 0xfde5380c);
	HH(a, b, c, d, x[1], 4, 0xa4beea44);
	HH(d, a, b, c, x[4], 11, 0x4bdecfa9);
	HH(c, d, a, b, x[7], 16, 0xf6bb4b60);
	HH(b, c, d, a, x[10], 23, 0xbebfbc70);
	HH(a, b, c, d, x[13], 4, 0x289b7ec6);
	HH(d, a, b, c, x[0], 11, 0xeaa127fa);
	HH(c, d, a, b, x[3], 16, 0xd4ef3085);
	HH(b, c, d, a, x[6], 23, 0x4881d05);
	HH(a, b, c, d, x[9], 4, 0xd9d4d039);
	HH(d, a, b, c, x[12], 11, 0xe6db99e5);
	HH(c, d, a, b, x[15], 16, 0x1fa27cf8);
	HH(b, c, d, a, x[2], 23, 0xc4ac5665);


	II(a, b, c, d, x[0], 6, 0xf4292244);
	II(d, a, b, c, x[7], 10, 0x432aff97);
	II(c, d, a, b, x[14], 15, 0xab9423a7);
	II(b, c, d, a, x[5], 21, 0xfc93a039);
	II(a, b, c, d, x[12], 6, 0x655b59c3);
	II(d, a, b, c, x[3], 10, 0x8f0ccc92);
	II(c, d, a, b, x[10], 15, 0xffeff47d);
	II(b, c, d, a, x[1], 21, 0x85845dd1);
	II(a, b, c, d, x[8], 6, 0x6fa87e4f);
	II(d, a, b, c, x[15], 10, 0xfe2ce6e0);
	II(c, d, a, b, x[6], 15, 0xa3014314);
	II(b, c, d, a, x[13], 21, 0x4e0811a1);
	II(a, b, c, d, x[4], 6, 0xf7537e82);
	II(d, a, b, c, x[11], 10, 0xbd3af235);
	II(c, d, a, b, x[2], 15, 0x2ad7d2bb);
	II(b, c, d, a, x[9], 21, 0xeb86d391);
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}

#endif


