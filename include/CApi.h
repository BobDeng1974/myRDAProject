#ifndef __CAPI_H__
#define __CAPI_H__
#include <string.h>
#include <stdlib.h>
//#include "string.h"
//#include "stdlib.h"
#include "cs_types.h"

#define CRC32_GEN		(0x04C11DB7)
#define CRC16_GEN		(0x1021)
#define CRC32_START		(0xffffffff)
#define CRC16_START		(0xffff)

#define L64_SUPPORT
typedef U64 u64;
#define IsAlpha(c)      (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
#define IsHex(c)      (((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')))
#define IsDigit(c)        ((c >= '0') && (c <= '9'))
#define IsAlphaDigit(c)    (IsAlpha(c) || IsDigit(c))
#define BCD2HEX(x)	(((x >> 4) * 10) + (x & 0x0f))
typedef struct
{
	u8 *Data;
	u32 Pos;
	u32 MaxLen;
}Buffer_Struct;

typedef  struct
{
	u32 param_max_num;
	u32 param_max_len;
	u32 param_num;
	s8 *param_str;
}CmdParam;

typedef struct {
	u8 Sec;
	u8 Min;
	u8 Hour;
	u8 Week;//表示日期0~6,sun~sat，表示预约时，bit0~bit6,sun~sat
}Time_UserDataStruct;

typedef struct {	
	u16 Year;
	u8 Mon;
	u8 Day;
}Date_UserDataStruct;

typedef union
{
	u32 dwTime;
	Time_UserDataStruct Time;
}Time_Union;

typedef union
{
	u32 dwDate;
	Date_UserDataStruct Date;
}Date_Union;

typedef s32(*MyAPIFunc)(void *p);

typedef struct {
	u32 Cmd;
	MyAPIFunc Func;
}CmdFunStruct;

typedef struct {
	u8 Cmd[24];
	MyAPIFunc Func;
}StrFunStruct;


typedef struct {
	u8 *Data;
	u32 Len;
	u32 Offset;
	u32 MaxLength;
	u32 DataSize;
}RBuffer;

void UnicodeToAsciiN(u16 *Src, u8 *Dst, u32 Len);
void AsciiToUnicodeN(u8 *Src, u16 *Dst, u32 Len);
double AsciiToFloat(u8 *Src);
u32 AsciiToU32(u8 *Src, u32 Len);
u32 AsciiToHex(u8 *Src, u32 len, u8 *Dst);
u32 HexToAscii(u8 *Src, u32 Len, u8 *Dst);
u32 StrToUint(const u8 *Src);
void IntToBCD(u32 Src, u8 *Dst, u8 Len);
void LongToBCD(u64 Src, u8 *Dst, u8 Len);
u32 BCDToInt(u8 *Src, u8 Len);
u8 IsDigitStr(const u8 *Src, u32 Len);
void ReverseBCD(u8 *Src, u8 *Dst, u32 Len);
#ifdef L64_SUPPORT
u64 UTC2Tamp(Date_UserDataStruct *Date, Time_UserDataStruct *Time);
u32 Tamp2UTC(u64 Sec, Date_UserDataStruct *Date, Time_UserDataStruct *Time, u32 LastDDay);
u8 XorCheck(u8 *Data, u32 Len, u8 CheckStart);
void CRC32_CreateTable(u32 *Tab, u32 Gen);
u32 CRC32_Cal(u32 * CRC32_Table, u8 *Buf, u32 Size, u32 CRC32Last);
#endif
u16 CRC16Cal(u8 *Src, u16 Len, u16 CRC16Last, u16 CRCRoot);
u32 ReadRBuffer(RBuffer *Buf, u8 *Data, u32 Len);
u32 QueryRBuffer(RBuffer *Buf, u8 *Data, u32 Len);
void InitRBuffer(RBuffer *Buf, u8 *Data, u32 MaxLen, u32 DataSize);
void DelRBuffer(RBuffer *Buf, u32 Len);
u32 WriteRBufferForce(RBuffer *Buf, u8 *Data, u32 Len);

u32 TransferPack(u8 Flag, u8 Code, u8 F1, u8 F2, u8 *InBuf, u32 Len, u8 *OutBuf);
u32 TransferUnpack(u8 Flag, u8 Code, u8 F1, u8 F2, u8 *InBuf, u32 Len, u8 *OutBuf);
/************************************************************************/
/*MD5相关                                                                      */
/************************************************************************/
typedef struct
{
	u32 count[2];
	u32 state[4];
	u8 buffer[64];
}MD5_CTX;


void MD5Init(MD5_CTX *context);
void MD5Update(MD5_CTX *context, u8 *input, u32 inputlen);
void MD5Final(MD5_CTX *context, u8 digest[16]);
void MD5Transform(u32 state[4], u8 block[64]);
void MD5Encode(u8 *output, u32 *input, u32 len);
void MD5Decode(u32 *output, u8 *input, u32 len);
u32 CmdParseParam(s8* pStr, CmdParam *CmdParam, s8 Cut);
#endif 

