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
	uint32_t u32_addr;
	uint8_t u8_addr[4];
}IP_AddrUnion;

void OS_APIInit(void);
void OS_GPIOInit(HAL_GPIO_GPIO_ID_T gpio, CONST HAL_GPIO_CFG_T* cfg);
void OS_GPIODeInit(HAL_GPIO_GPIO_ID_T gpio);
//SPI
void OS_SPIInit(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum, CONST HAL_SPI_CFG_T* spiConfigPtr);
void OS_SPIOpen(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum);
void OS_SPIClose(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum);
//DMA
uint8_t OS_DMAStart(HAL_IFC_REQUEST_ID_T IfcID, uint8_t* Buf, uint32_t Len, HAL_IFC_MODE_T IfcMode);
//I2C
void OS_I2COpen(void);
void OS_I2CClose(void);
void OS_UartOpen(HAL_UART_ID_T UartID, HAL_UART_CFG_T* uartCfg, HAL_UART_IRQ_STATUS_T mask, HAL_UART_IRQ_HANDLER_T handler);
void OS_UartClose(HAL_UART_ID_T UartID);
void OS_UartSetBR(HAL_UART_ID_T UartID, uint32_t BR);
uint8_t OS_UartDMASend(HAL_IFC_REQUEST_ID_T IfcID, uint8_t *Buf, uint32_t Len);
HAL_ERR_T OS_I2CXfer(HAL_I2C_BUS_ID_T BusId, uint8_t Addr, uint8_t *Reg, uint8_t RegNum, uint8_t *Buf, uint8_t Len, uint8_t WriteFlag, uint32_t To);
//VBAT
uint16_t OS_GetVbatADC(void);
//PWM
void OS_PWLStart(uint8_t Duty);
void OS_PWLStop(void);
void OS_PWTStart(uint16_t Freq, uint16_t Level, uint8_t Duty);
void OS_PWTStop(void);
//SYS
uint8_t OS_GetResetReason(void);
uint8_t OS_SendEvent(HANDLE hTask, uint32_t EventID, uint32_t Param1, uint32_t Param2, uint32_t Param3);
void OS_FlyMode(uint8_t Switch);
void OS_GetIMEI(uint8_t *IMEI);
void OS_StartTimer(HANDLE hTask, uint8_t nTimerId, uint8_t nMode, uint32_t nElapse);
void OS_StopTimer(HANDLE hTask, uint8_t nTimerId);
void OS_Sleep(uint32_t To);
//SIM
void OS_GetICCID(uint8_t *ICCID);
void OS_GetIMSI(uint8_t *IMEI, int8_t *Str, uint32_t Len);
void OS_GetIMSIReq(void);
uint8_t OS_GetSimStatus(void);
//GPRS
uint8_t OS_GetRegStatus(void);
void OS_GPRSAttachReq(uint8_t Req);
void OS_GetGPRSAttach(uint8_t *State);
void OS_GPRSActReq(uint8_t Req, uint8_t *APNName, uint8_t *APNUser, uint8_t *APNPassword);
void OS_GetGPRSActive(uint8_t *State);
void OS_SetCIPIPPdpCxt(uint8_t *APNName, uint8_t *APNUser, uint8_t *APNPassword);
void OS_GetCIPIPPdpCxt(IP_AddrUnion *LocalIP, IP_AddrUnion *DNS);
void OS_GetCellInfo(CFW_TSM_CURR_CELL_INFO *pCurrCellInfo, CFW_TSM_ALL_NEBCELL_INFO *pNeighborCellInfo);
void OS_StartTSM(void);
//Call
uint8_t OS_Call(uint8_t *Num, uint8_t NumLen, uint8_t Type);
void OS_CallAccpet(void);
void OS_CallRelease(void);
//SMS
void OS_SMSInitStart(uint8_t Param);
void OS_SMSInitFinish(uint16_t nUTI, CFW_SMS_PARAMETER *sInfo);
void OS_SMSGetStorageInfo(CFW_SMS_STORAGE_INFO *Info);
uint32_t OS_SMSTxByPDU(uint8_t *pData, uint16_t nDataSize);
//TCPIP
uint32_t OS_GetHost(int8_t *Name, struct ip_addr *IP);
SOCKET OS_CreateSocket(uint8_t nDomain, uint8_t nType, uint8_t nProtocol);
uint32_t OS_SocketConnect(SOCKET SocketID, uint32_t LocalIP, uint16_t LocalPort, uint32_t RemoteIP, uint16_t RemotePort);
uint32_t OS_SocketDisconnect(SOCKET SocketID);
uint32_t OS_SocketReceive(SOCKET SocketID, uint8_t *Buf, uint32_t Len, CFW_TCPIP_SOCKET_ADDR *from, INT32 *fromlen);
uint32_t OS_SocketSend(SOCKET SocketID, uint8_t *Buf, uint32_t Len, CFW_TCPIP_SOCKET_ADDR *to, INT32 tolen);

uint32_t OS_GB2312ToUCS2(const uint8_t* src, uint8_t* dst, uint32_t srclen, uint8_t IsBigEnding);
uint32_t OS_UCS2ToGB2312(const uint8_t * src, uint8_t * dst,uint32_t srclen, uint8_t IsBigEnding);

#endif
