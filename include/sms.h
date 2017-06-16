#ifndef __CUST_SMS_H__
#define __CUST_SMS_H__

void SMS_Config(void);
void SMS_Receive(CFW_NEW_SMS_NODE *pNewMsgNode);
void SMS_Send(u8 *ToNumber, u8 ToNumberLen, u8* UserData, u32 Len, u8 DCS);
void SMS_Submit(void);
void SMS_SendFree(void);
#endif
