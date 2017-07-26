#ifndef __CUST_OS_H__
#define __CUST_OS_H__
#include "platform_api.h"
enum
{
	UTI_TSM = 1,
	UTI_GPRS_ATTACH,
	UTI_GPRS_DETACH,
	UTI_CID_IP_ACT,
	UTI_CID_IP_DEACT,
	UTI_SMS_INIT,
	UTI_SMS_SEND,
	UTI_GET_IMSI,
	UTI_MAKE_CALL,
	UTI_FLY_MODE,
	CID_IP = 1,
};

typedef union
{
	u32 u32_addr;
	u8 u8_addr[4];
}IP_AddrUnion;

void OS_APIInit(void);
void OS_GPIOInit(HAL_GPIO_GPIO_ID_T gpio, CONST HAL_GPIO_CFG_T* cfg);
void OS_GPIODeInit(HAL_GPIO_GPIO_ID_T gpio);
//SPI
void OS_SPIInit(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum, CONST HAL_SPI_CFG_T* spiConfigPtr);
void OS_SPIOpen(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum);
void OS_SPIClose(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum);
//DMA
u8 OS_DMAStart(HAL_IFC_REQUEST_ID_T IfcID, u8* Buf, u32 Len, HAL_IFC_MODE_T IfcMode);
//I2C
void OS_I2COpen(void);
void OS_I2CClose(void);
void OS_UartOpen(HAL_UART_ID_T UartID, HAL_UART_CFG_T* uartCfg, HAL_UART_IRQ_STATUS_T mask, HAL_UART_IRQ_HANDLER_T handler);
void OS_UartClose(HAL_UART_ID_T UartID);
void OS_UartSetBR(HAL_UART_ID_T UartID, u32 BR);
u8 OS_UartDMASend(HAL_IFC_REQUEST_ID_T IfcID, u8 *Buf, u32 Len);
HAL_ERR_T OS_I2CXfer(HAL_I2C_BUS_ID_T BusId, u8 Addr, u8 *Reg, u8 RegNum, u8 *Buf, u8 Len, u8 WriteFlag, u32 To);
//VBAT
u16 OS_GetVbatADC(void);
//PWM
void OS_PWMSetDuty(u8 Duty);
void OS_PWMStop(void);
//SYS
u8 OS_GetResetReason(void);
u8 OS_SendEvent(HANDLE hTask, u32 EventID, u32 Param1, u32 Param2, u32 Param3);
void OS_FlyMode(u8 Switch);
void OS_GetIMEI(u8 *IMEI);
void OS_StartTimer(HANDLE hTask, u8 nTimerId, u8 nMode, u32 nElapse);
void OS_StopTimer(HANDLE hTask, u8 nTimerId);
void OS_Sleep(u32 To);
//SIM
void OS_GetICCID(u8 *ICCID);
void OS_GetIMSI(u8 *IMEI, s8 *Str, u32 Len);
void OS_GetIMSIReq(void);
u8 OS_GetSimStatus(void);
//GPRS
u8 OS_GetRegStatus(void);
void OS_GPRSAttachReq(u8 Req);
void OS_GetGPRSAttach(u8 *State);
void OS_GPRSActReq(u8 Req, u8 *APNName, u8 *APNUser, u8 *APNPassword);
void OS_GetGPRSActive(u8 *State);
void OS_SetCIPIPPdpCxt(u8 *APNName, u8 *APNUser, u8 *APNPassword);
void OS_GetCIPIPPdpCxt(IP_AddrUnion *LocalIP, IP_AddrUnion *DNS);
void OS_GetCellInfo(CFW_TSM_CURR_CELL_INFO *pCurrCellInfo, CFW_TSM_ALL_NEBCELL_INFO *pNeighborCellInfo);
void OS_StartTSM(void);
//Call
u8 OS_Call(u8 *Num, u8 NumLen, u8 Type);
void OS_CallAccpet(void);
void OS_CallRelease(void);
//SMS
void OS_SMSInitStart(u8 Param);
void OS_SMSInitFinish(u16 nUTI, CFW_SMS_PARAMETER *sInfo);
void OS_SMSGetStorageInfo(CFW_SMS_STORAGE_INFO *Info);
u32 OS_SMSTxByPDU(u8 *pData, u16 nDataSize);
//TCPIP
u32 OS_GetHost(s8 *Name, struct ip_addr *IP);
SOCKET OS_CreateSocket(u8 nDomain, u8 nType, u8 nProtocol);
u32 OS_SocketConnect(SOCKET SocketID, u32 LocalIP, u32 RemoteIP, u16 Port);
u32 OS_SocketDisconnect(SOCKET SocketID);
u32 OS_SocketReceive(SOCKET SocketID, u8 *Buf, u32 Len, CFW_TCPIP_SOCKET_ADDR *from, INT32 *fromlen);
u32 OS_SocketSend(SOCKET SocketID, u8 *Buf, u32 Len, CFW_TCPIP_SOCKET_ADDR *to, INT32 tolen);

u32 OS_UCS2ToGB2312(u16 *InBuf, u32 InLen, u8 *OutBuf, u32 *OutLen);
u32 OS_GB2312ToUCS2(u8 *InBuf, u32 InLen, u8 *OutBuf, u32 *OutLen);
u32 OS_UCS2ToGB2312Big(u16 *InBuf, u32 InLen, u8 *OutBuf, u32 *OutLen);
u32 OS_GB2312ToUCS2Big(u8 *InBuf, u32 InLen, u8 *OutBuf, u32 *OutLen);
#endif
