#ifndef __TEA_H__
#define __TEA_H__

#include <string.h>

enum
{
	TEA_TYPE_ORG,
	TEA_TYPE_X,
	TEA_TYPE_XX,
};
void TEA_Encode(u32 *Data, u32 *Key, u32 Rounds, u8 Type);
void TEA_Decode(u32 *Data, u32 *Key, u32 Rounds, u8 Type);
#endif
