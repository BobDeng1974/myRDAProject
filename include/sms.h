#ifndef __CUST_SMS_H__
#define __CUST_SMS_H__

void SMS_Config(void);
void SMS_Receive(CFW_NEW_SMS_NODE *pNewMsgNode);
void SMS_Send(uint8_t *ToNumber, uint8_t ToNumberLen, uint8_t* UserData, uint32_t Len, uint8_t DCS);
void SMS_Submit(void);
void SMS_SendFree(void);
#endif
