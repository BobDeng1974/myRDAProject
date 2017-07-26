#include "os.h"
#include "CApi.h"
#define SIM_SN			(CFW_SIM_0)
#define API_PS_DETACH_GPRS     1
#define HAL_I2C_SEND_BYTE_DELAY 20

extern PUBLIC UINT8* pal_GetImei(UINT8 simIndex);
extern UINT32 CFW_AttDetach (UINT8 nState, UINT16 nUTI, UINT8 AttDetachType
        , CFW_SIM_ID nSimID
       );
extern PUBLIC UINT16 pmd_GetGpadcBatteryLevel(VOID);
extern PUBLIC CONST UINT8 *pal_GetFactoryImei(UINT8 simIndex);
extern UINT32 CFW_getDnsServerbyPdp(UINT8 nCid, UINT8 nSimID );
extern BOOL hal_PwmResourceMgmt(VOID);
UINT32 CFW_GprsGetPdpAddr(UINT8 nCid, UINT8 *nLength, UINT8 *pPdpAdd, CFW_SIM_ID nSimID);
typedef void (*GPIOInit)(HAL_GPIO_GPIO_ID_T gpio, CONST HAL_GPIO_CFG_T* cfg);
typedef void (*GPIODeInit)(HAL_GPIO_GPIO_ID_T gpio);
typedef void (*SPIInit)(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum, CONST HAL_SPI_CFG_T* spiConfigPtr);
typedef void (*SPIOpen)(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum);
typedef void (*SPIClose)(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum);
typedef u8 (*DMAStart)(HAL_IFC_REQUEST_ID_T IfcID, u8* Buf, u32 Len, HAL_IFC_MODE_T IfcMode);
typedef void (*I2COpen)(void);
typedef void (*I2CClose)(void);
typedef HAL_ERR_T (*I2CXfer)(HAL_I2C_BUS_ID_T BusId, u8 Addr, u8 *Reg, u8 RegNum, u8 *Buf, u8 Len, u8 WriteFlag, u32 To);
typedef void (*UartOpen)(HAL_UART_ID_T UartID, HAL_UART_CFG_T* uartCfg, HAL_UART_IRQ_STATUS_T mask, HAL_UART_IRQ_HANDLER_T handler);
typedef void (*UartClose)(HAL_UART_ID_T UartID);
typedef void (*UartSetBR)(HAL_UART_ID_T UartID, u32 BR);
typedef u16 (*GetVbatADC)(void);
typedef void (*PWMSetDuty)(u8 Duty);
typedef void (*PWMStop)(void);
typedef u8 (*GetResetReason)(void);
typedef u8 (*SendEvent)(HANDLE hTask, u32 EventID, u32 Param1, u32 Param2, u32 Param3);
typedef void (*StartTimer)(HANDLE hTask, u8 nTimerId, u8 nMode, u32 nElapse);
typedef void (*StopTimer)(HANDLE hTask, u8 nTimerId);
typedef void (*Sleep)(u32 To);

typedef u8 (*GetSimStatus)(void);
typedef void (*GetICCID)(u8 *ICCID);
typedef void (*GetIMSI)(u8 *IMSI, s8 *Str, u32 Len);
typedef void (*GetIMSIReq)(void);
typedef void (*GetIMEI)(u8 *IMEI);
typedef void (*FlyMode)(u8 Switch);
typedef u8 (*GetRegStatus)(void);
typedef void (*GPRSAttachReq)(u8 Req);
typedef void (*GetGPRSAttach)(u8 *State);
typedef void (*GPRSActReq)(u8 Req, u8 *APNName, u8 *APNUser, u8 *APNPassword);
typedef void (*GetGPRSActive)(u8 *State);
typedef void (*SetCIPIPPdpCxt)(u8 *APNName, u8 *APNUser, u8 *APNPassword);
typedef void (*GetCIPIPPdpCxt)(IP_AddrUnion *LocalIP, IP_AddrUnion *DNS);
typedef void (*GetCellInfo)(CFW_TSM_CURR_CELL_INFO *pCurrCellInfo, CFW_TSM_ALL_NEBCELL_INFO *pNeighborCellInfo);
typedef void (*StartTSM)(void);

typedef void (*CallAccpet)(void);
typedef void (*CallRelease)(void);

typedef void (*SMSInitStart)(u8 Param);
typedef void (*SMSInitFinish)(u16 nUTI, CFW_SMS_PARAMETER *sInfo);
typedef void (*SMSGetStorageInfo)(CFW_SMS_STORAGE_INFO *Info);
typedef u32 (*SMSTxByPDU)(u8 *pData, u16 nDataSize);

typedef u32 (*GetHost)(s8 *Name, struct ip_addr *IP);
typedef SOCKET(*CreateSocket)(u8 nDomain, u8 nType, u8 nProtocol);
typedef u32 (*SocketConnect)(SOCKET SocketID, u32 LocalIP, u32 RemoteIP, u16 Port);
typedef u32 (*SocketDisconnect)(SOCKET SocketID);
typedef u32 (*SocketReceive)(SOCKET SocketID, u8 *Buf, u32 Len, CFW_TCPIP_SOCKET_ADDR *from, INT32 *fromlen);
typedef u32 (*SocketSend)(SOCKET SocketID, u8 *Buf, u32 Len, CFW_TCPIP_SOCKET_ADDR *to, INT32 tolen);
typedef s32 (*TTS_Play)(void *Data, u32 Len, void *PCMCB, void *TTSCB);
typedef u32 (*UCS2ToGB2312)(const u8 * src, u8 * dst, u32 srclen);
typedef u32 (*GB2312ToUCS2)(const u8* src, u8* dst, u32 srclen);

typedef double (*MathFun1)(double);
typedef double (*MathFun2)(double, double);

typedef struct
{
	GPIOInit GPIOInitFun;
	GPIODeInit GPIODeInitFun;
	SPIInit SPIInitFun;
	SPIOpen SPIOpenFun;
	SPIClose SPICloseFun;
	I2COpen I2COpenFun;
	I2CClose I2CCloseFun;
	UartOpen UartOpenFun;
	UartClose UartCloseFun;
	UartSetBR UartSetBRFun;
	DMAStart DMAStartFun;
	I2CXfer I2CXferFun;
	GetVbatADC GetVbatADCFun;
	PWMSetDuty PWMSetDutyFun;
	PWMStop PWMStopFun;
	GetResetReason GetResetReasonFun;
	SendEvent SendEventFun;
	StartTimer StartTimerFun;
	StopTimer StopTimerFun;
	Sleep SleepFun;
	GetIMEI GetIMEIFun;
	GetSimStatus GetSimStatusFun;
	GetICCID GetICCIDFun;
	GetIMSI GetIMSIFun;
	GetIMSIReq GetIMSIReqFun;
	FlyMode FlyModeFun;
	GetRegStatus GetRegStatusFun;
	GPRSAttachReq GPRSAttachReqFun;
	GetGPRSAttach GetGPRSAttachFun;
	GPRSActReq GPRSActReqFun;
	SetCIPIPPdpCxt SetCIPIPPdpCxtFun;
	GetCIPIPPdpCxt GetCIPIPPdpCxtFun;
	GetCellInfo GetCellInfoFun;
	StartTSM StartTSMFun;
	CallAccpet CallAccpetFun;
	CallRelease CallReleaseFun;
	SMSInitStart SMSInitStartFun;
	SMSInitFinish SMSInitFinishFun;
	SMSGetStorageInfo SMSGetStorageInfoFun;
	SMSTxByPDU SMSTxByPDUFun;
	GetHost GetHostFun;
	CreateSocket CreateSocketFun;
	SocketConnect SocketConnectFun;
	SocketDisconnect SocketDisconnectFun;
	SocketReceive SocketReceiveFun;
	SocketSend SocketSendFun;
	UCS2ToGB2312 UCS2ToGB2312Fun;
	GB2312ToUCS2 GB2312ToUCS2Fun;
	UCS2ToGB2312 UCS2ToGB2312BigFun;
	GB2312ToUCS2 GB2312ToUCS2BigFun;
	TTS_Play TTSPlayFun;
	MathFun1 sin;
	MathFun1 cos;
	MathFun1 tan;
	MathFun1 asin;
	MathFun1 acos;
	MathFun1 atan;
	MathFun2 pow;
	MathFun1 sqrt;
	MathFun1 exp;
	MathFun1 log;
	MathFun1 log10;
}OS_APIListStruct;

OS_APIListStruct gOSAPIList;

s32 OS_Test(void *Param)
{
	s8 Test[32] = "just for test, check map!";
	s8 Test2[32] = "123";
	s8 Test3[64] = "255.255.255.255";
	//u8 Test4[64] = {0x55, 0x4A, 0x55, 0x4A, 0x30, 0x10, 0x4F, 0x0D};
	u32 Len;
	u16 SLen = 1;
	//DBG("test start!");
	Len = inet_addr(Test3);
	Len = htonl(Len) + Len;
	SLen = htons(SLen);
	strcpy(Test2, Test);
	strcat(Test3, Test2);
	if (strcmp(Test, Test3))
	{
		Len *= 31415926/6;
	}
	else
	{
		Len *= 31415926/12;
	}
#ifdef SUPPORT_SOCKET_8
	CORE("test %u %u - 8", Len, hal_SysGetFreq());
#else
	CORE("test %u %u - 4", Len, hal_SysGetFreq());
#endif
	return 0;
}

void OS_APIInit(void)
{
	gOSAPIList.GPIOInitFun = OS_GPIOInit;
	gOSAPIList.GPIODeInitFun = OS_GPIODeInit;
	gOSAPIList.SPIInitFun = OS_SPIInit;
	gOSAPIList.SPIOpenFun = OS_SPIOpen;
	gOSAPIList.SPICloseFun = OS_SPIClose;
	gOSAPIList.I2COpenFun = OS_I2COpen;
	gOSAPIList.I2CCloseFun = OS_I2CClose;
	gOSAPIList.UartOpenFun = OS_UartOpen;
	gOSAPIList.UartCloseFun = OS_UartClose;
	gOSAPIList.UartSetBRFun = OS_UartSetBR;
	gOSAPIList.DMAStartFun = OS_DMAStart;
	gOSAPIList.I2CXferFun = OS_I2CXfer;
	gOSAPIList.GetVbatADCFun = OS_GetVbatADC;
	gOSAPIList.PWMSetDutyFun = OS_PWMSetDuty;
	gOSAPIList.PWMStopFun = OS_PWMStop;
	gOSAPIList.GetResetReasonFun = OS_GetResetReason;
	gOSAPIList.SendEventFun = OS_SendEvent;
	gOSAPIList.StartTimerFun = OS_StartTimer;
	gOSAPIList.StopTimerFun = OS_StopTimer;
	gOSAPIList.SleepFun = OS_Sleep;
	gOSAPIList.GetIMEIFun = OS_GetIMEI;
	gOSAPIList.GetSimStatusFun = OS_GetSimStatus;
	gOSAPIList.GetICCIDFun = OS_GetICCID;
	gOSAPIList.GetIMSIFun = OS_GetIMSI;
	gOSAPIList.GetIMSIReqFun = OS_GetIMSIReq;
	gOSAPIList.FlyModeFun = OS_FlyMode;
	gOSAPIList.GetRegStatusFun = OS_GetRegStatus;
	gOSAPIList.GetGPRSAttachFun = OS_GetGPRSAttach;
	gOSAPIList.GPRSAttachReqFun = OS_GPRSAttachReq;
	gOSAPIList.GPRSActReqFun = OS_GPRSActReq;
	gOSAPIList.SetCIPIPPdpCxtFun = OS_SetCIPIPPdpCxt;
	gOSAPIList.GetCIPIPPdpCxtFun = OS_GetCIPIPPdpCxt;
	gOSAPIList.GetCellInfoFun = OS_GetCellInfo;
	gOSAPIList.StartTSMFun = OS_StartTSM;
	gOSAPIList.CallAccpetFun = OS_CallAccpet;
	gOSAPIList.CallReleaseFun = OS_CallRelease;
	gOSAPIList.SMSInitStartFun = OS_SMSInitStart;
	gOSAPIList.SMSInitFinishFun = OS_SMSInitFinish;
	gOSAPIList.SMSGetStorageInfoFun = OS_SMSGetStorageInfo;
	gOSAPIList.SMSTxByPDUFun = OS_SMSTxByPDU;
	gOSAPIList.GetHostFun = OS_GetHost;
	gOSAPIList.CreateSocketFun = OS_CreateSocket;
	gOSAPIList.SocketConnectFun = OS_SocketConnect;
	gOSAPIList.SocketDisconnectFun = OS_SocketDisconnect;
	gOSAPIList.SocketReceiveFun = OS_SocketReceive;
	gOSAPIList.SocketSendFun = OS_SocketSend;
#ifdef __TTS_ENABLE__
	gOSAPIList.TTSPlayFun = __TTS_Play;
#endif
	gOSAPIList.UCS2ToGB2312Fun = __UCS2ToGB2312;
	gOSAPIList.GB2312ToUCS2Fun = __GB2312ToUCS2;
	gOSAPIList.sin = sin;
	gOSAPIList.cos = cos;
	gOSAPIList.tan = tan;
	gOSAPIList.asin = asin;
	gOSAPIList.acos = acos;
	gOSAPIList.atan = atan;
	gOSAPIList.pow = pow;
	gOSAPIList.sqrt = sqrt;
	gOSAPIList.exp = exp;
	gOSAPIList.log = log;
	gOSAPIList.log10 = log10;
	OS_Test(NULL);
}

void OS_GPIOInit(HAL_GPIO_GPIO_ID_T gpio, CONST HAL_GPIO_CFG_T* cfg)
{
	hal_GpioOpen(gpio, cfg);
}
void OS_GPIODeInit(HAL_GPIO_GPIO_ID_T gpio)
{
	hal_GpioClose(gpio);
}
//SPI
void OS_SPIInit(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum, CONST HAL_SPI_CFG_T* spiConfigPtr)
{
	hal_SpiOpen(BusId, csNum, spiConfigPtr);
}

void OS_SPIOpen(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum)
{
	hal_SpiActivateCs(BusId, csNum);
}

void OS_SPIClose(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum)
{
	hal_SpiDeActivateCs(BusId, csNum);
}
//DMA
u8 OS_DMAStart(HAL_IFC_REQUEST_ID_T IfcID, u8* Buf, u32 Len, HAL_IFC_MODE_T IfcMode)
{
	return hal_IfcTransferStart(IfcID, Buf, Len, IfcMode);
}
//I2C
void OS_I2COpen(void)
{
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_configRegs->GPIO_Mode &= ~((1 << 24)|(1 << 25));
#endif

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	 hwp_iomux->pad_GPIO_6_cfg = IOMUX_PAD_GPIO_6_SEL(2);
	 hwp_iomux->pad_GPIO_7_cfg = IOMUX_PAD_GPIO_7_SEL(2);
#endif
}

void OS_I2CClose(void)
{
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_configRegs->GPIO_Mode |= (1 << 24)|(1 << 25);
#endif
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	 hwp_iomux->pad_GPIO_6_cfg = IOMUX_PAD_GPIO_6_SEL(0);
	 hwp_iomux->pad_GPIO_7_cfg = IOMUX_PAD_GPIO_7_SEL(0);
#endif
}

void OS_UartOpen(HAL_UART_ID_T UartID, HAL_UART_CFG_T* uartCfg, HAL_UART_IRQ_STATUS_T mask, HAL_UART_IRQ_HANDLER_T handler)
{
	hal_UartOpen(UartID, uartCfg);
	hal_UartIrqSetMask(UartID, mask);
	hal_UartIrqSetHandler(UartID, handler);
}

void OS_UartClose(HAL_UART_ID_T UartID)
{
	hal_UartClose(UartID);
}

void OS_UartSetBR(HAL_UART_ID_T UartID, u32 BR)
{
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	UINT32 uartClockDivisor = 0;
    switch(BR)
    {
        //  Using the slow clock at 52MHz   //   8955  // div is set 4
        case HAL_UART_BAUD_RATE_3250000: //0x1001
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(4);
            break;
        case HAL_UART_BAUD_RATE_2166700: //0x1801
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(6);
            break;
        case HAL_UART_BAUD_RATE_1625000: //0x2001
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(8);
            break;
        case HAL_UART_BAUD_RATE_1300000: //0x2801
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(10);
            break;
        case HAL_UART_BAUD_RATE_921600:  //0x3801
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(14);
            break;
        case HAL_UART_BAUD_RATE_460800: //0x2c1464
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(100)|SYS_CTRL_CFG_UART_DENOM(2821);
            break;
        case HAL_UART_BAUD_RATE_230400: //0x46805
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(5)|SYS_CTRL_CFG_UART_DENOM(282);
            break;
        case HAL_UART_BAUD_RATE_115200: //0x1c401
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(113);
            break;
        case HAL_UART_BAUD_RATE_57600: //0x23440a
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(10)|SYS_CTRL_CFG_UART_DENOM(2257);
            break;
        case HAL_UART_BAUD_RATE_38400: //0xa9402
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(2)|SYS_CTRL_CFG_UART_DENOM(677);
            break;
        case HAL_UART_BAUD_RATE_33600: //0x1e3405
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(5)|SYS_CTRL_CFG_UART_DENOM(1933);
            break;
        case HAL_UART_BAUD_RATE_28800: //0x70c01
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(451);
            break;
        case HAL_UART_BAUD_RATE_19200: //0xa9401
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(677);
            break;
        case HAL_UART_BAUD_RATE_14400: //0xe1c01
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(903);
            break;
        case HAL_UART_BAUD_RATE_9600: //0x152801
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(1354);
            break;
        case HAL_UART_BAUD_RATE_4800: //0x2a5001
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(2708);
            break;
        case HAL_UART_BAUD_RATE_2400: //0x54a001
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(5416);//  52/4/0.0024
            break;
        case HAL_UART_BAUD_RATE_1200: //0xa94401
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(10833);//  52/4/0.0012
            break;

        default:
            // Baud rate is calculated as 6.5M/2, 6.5M/3, 6.5M/4, ...
            // Limit the number of supported baud rates to avoid
            // rounding issue in the division.
            return;
    }
    hwp_sysCtrl->Cfg_Clk_Uart[UartID] = uartClockDivisor;
#endif

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
    UINT32 uartClockDivisorMode = 4;
    UINT32 uartClockDivisor = 0;
    UINT32 fs, fsSys, mode;
    UINT32 clockToUse;
    register UINT32 uartConfig;
    switch(BR)
    {

	//  Using mode divisor = 16
	case HAL_UART_BAUD_RATE_2400:
	case HAL_UART_BAUD_RATE_4800:
		uartConfig |= UART_DIVISOR_MODE;
		uartClockDivisorMode = 16;
		break;
		//  Using mode divisor = 4
	default:
		uartConfig &= ~UART_DIVISOR_MODE;
		uartClockDivisorMode = 4;
		break;
    }
    fsSys = 26000000;
    clockToUse = SYS_CTRL_UART_SEL_PLL_SLOW;
    fs = BR;
    mode = uartClockDivisorMode;
    uartClockDivisor = ( (fsSys + ((mode / 2) * fs)) / (mode * fs) ) - 2;

    //  Configure the clock register.
    hwp_sysCtrl->Cfg_Clk_Uart[UartID] =
        SYS_CTRL_UART_DIVIDER(uartClockDivisor) | clockToUse;
#endif
}

static void OS_I2CClockDown(void)
{
	UINT32 criticalSectionValue;
    criticalSectionValue = hal_SysEnterCriticalSection();
    hal_SysRequestFreq(HAL_SYS_FREQ_I2C, HAL_SYS_FREQ_32K, NULL);
    hal_SysExitCriticalSection(criticalSectionValue);
}

static void OS_I2CClockUpdate(HAL_SYS_FREQ_T sysFreq)
{
    UINT32 newClkScale;
    UINT32 ctrlReg;

    ctrlReg = hwp_i2cMaster->CTRL & ~(I2C_MASTER_CLOCK_PRESCALE_MASK);
    newClkScale = sysFreq/(5 * 400000)-1;
    ctrlReg |= I2C_MASTER_CLOCK_PRESCALE(newClkScale);
    hwp_i2cMaster->CTRL = ctrlReg;

    ctrlReg = hwp_i2cMaster2->CTRL & ~(I2C_MASTER_CLOCK_PRESCALE_MASK);
    ctrlReg |= I2C_MASTER_CLOCK_PRESCALE(newClkScale);
    hwp_i2cMaster2->CTRL = ctrlReg;

    ctrlReg = hwp_i2cMaster3->CTRL & ~(I2C_MASTER_CLOCK_PRESCALE_MASK);
    ctrlReg |= I2C_MASTER_CLOCK_PRESCALE(newClkScale);
    hwp_i2cMaster3->CTRL = ctrlReg;

}

static HAL_ERR_T OS_I2CGetData(u8 BusId, u8 Addr, u8 *Reg, u8 RegNum, u8 *Buf, u8 Len, u8 WriteFlag, u32 To)
{
    UINT32 second_time,first_time;
    HWP_I2C_MASTER_T* i2cMaster;
    UINT32 criticalSectionValue;
    UINT32 currentByte = 0;
    To = (To * SYS_TICK) / 1000;
    u8 i;

    first_time = hal_TimGetUpTime();

    switch (BusId)
    {
    case HAL_I2C_BUS_ID_1:
    	i2cMaster = hwp_i2cMaster;
    	break;
    case HAL_I2C_BUS_ID_2:
    	i2cMaster = hwp_i2cMaster2;
    	break;
    case HAL_I2C_BUS_ID_3:
    	i2cMaster = hwp_i2cMaster3;
    	break;
    default:
    	return HAL_ERR_RESOURCE_NOT_ENABLED;
    }



    criticalSectionValue = hal_SysEnterCriticalSection();
    hal_SysRequestFreq(HAL_SYS_FREQ_I2C, HAL_SYS_FREQ_104M, OS_I2CClockUpdate);
    OS_I2CClockUpdate(hal_SysGetFreq());
    hal_SysExitCriticalSection(criticalSectionValue);

    // Set the new clock scal.
    // Clear status bit in case previous transfer (Raw) hasn't
    // cleared it.
    i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

    // Write slave address (Write mode, to write memory address)
    i2cMaster -> TXRX_BUFFER = (Addr << 1);
    i2cMaster -> CMD = I2C_MASTER_WR | I2C_MASTER_STA;

    // Polling on the TIP flag
    /*     while(i2cMaster -> STATUS & I2C_MASTER_TIP); */
    while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
    {
        second_time = hal_TimGetUpTime();
        if (second_time - first_time > To)
        {
            i2cMaster->CMD = I2C_MASTER_STO;
            OS_I2CClockDown();
            return HAL_ERR_RESOURCE_TIMEOUT;
        }
    };

    i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

    // Transfert done

    // Check RxACK
    if (i2cMaster -> STATUS & I2C_MASTER_RXACK)
    {
        // Abort the transfert
        i2cMaster -> CMD = I2C_MASTER_STO ;
        while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
        {
            second_time = hal_TimGetUpTime();
            if (second_time - first_time > To)
            {
            	OS_I2CClockDown();
                return HAL_ERR_RESOURCE_TIMEOUT;
            }
        };

        i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;
        OS_I2CClockDown();
        return HAL_ERR_COMMUNICATION_FAILED;
    }

    for(i = 0; i < RegNum; i++)
    {
	   // Write memory address
		i2cMaster -> TXRX_BUFFER = Reg[i];
		i2cMaster -> CMD = I2C_MASTER_WR;

		// Polling on the TIP flag
		/*     while(i2cMaster -> STATUS & I2C_MASTER_TIP); */
		while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
		{
			second_time = hal_TimGetUpTime();
			if (second_time - first_time > To)
			{
				i2cMaster->CMD = I2C_MASTER_STO;
				OS_I2CClockDown();
				return HAL_ERR_RESOURCE_TIMEOUT;
			}
		};

		i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

		// Check RxACK
		if (i2cMaster -> STATUS & I2C_MASTER_RXACK)
		{
			// Abort the transfert
			i2cMaster -> CMD = I2C_MASTER_STO ;
			while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
			{
				second_time = hal_TimGetUpTime();
				if (second_time - first_time > To)
				{
					OS_I2CClockDown();
					return HAL_ERR_RESOURCE_TIMEOUT;
				}
			};

			i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;
			OS_I2CClockDown();
			return HAL_ERR_COMMUNICATION_FAILED;
		}
    }

    if (WriteFlag)
    {
	   // Write all data but the last one.
		for (currentByte = 0 ; currentByte < Len - 1 ; currentByte++)
		{
			i2cMaster -> TXRX_BUFFER = Buf[currentByte];
			i2cMaster -> CMD = I2C_MASTER_WR;

			// Polling on the TIP flag
			/*         while(i2cMaster -> STATUS & I2C_MASTER_TIP); */
			while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
			{
				second_time = hal_TimGetUpTime();
				if (second_time - first_time > To)
				{
					i2cMaster->CMD = I2C_MASTER_STO;
					OS_I2CClockDown();
					return HAL_ERR_RESOURCE_TIMEOUT;
				}
			};

			i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

			// Check RxACK
			if (i2cMaster -> STATUS & I2C_MASTER_RXACK)
			{
				// Stop condition sent via the previous
				// command
				OS_I2CClockDown();
				return HAL_ERR_COMMUNICATION_FAILED;
			}
		}

		// Send last byte with stop condition
		i2cMaster -> TXRX_BUFFER = Buf[Len - 1];
		i2cMaster -> CMD = I2C_MASTER_WR | I2C_MASTER_STO ;

		// Polling on the TIP flag
		/*     while(i2cMaster -> STATUS & I2C_MASTER_TIP); */
		while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
		{
			second_time = hal_TimGetUpTime();
			if(second_time - first_time > To)
			{
				OS_I2CClockDown();
				return HAL_ERR_RESOURCE_TIMEOUT;
			}
		};

		i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;


		// Check RxACK
		if (i2cMaster -> STATUS & I2C_MASTER_RXACK)
		{
			// Stop condition sent via the previous
			// command
			OS_I2CClockDown();
			return HAL_ERR_COMMUNICATION_FAILED;
		}
    }
    else
    {
        // Write slave address + R/W = '1' (Read mode)
        i2cMaster -> TXRX_BUFFER = (Addr << 1 | 0x1);
        i2cMaster -> CMD = I2C_MASTER_WR | I2C_MASTER_STA;

        // Polling on the TIP flag
        /*     while(i2cMaster -> STATUS & I2C_MASTER_TIP); */
        while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
        {
            second_time = hal_TimGetUpTime();
            if (second_time - first_time > To)
            {
                i2cMaster->CMD = I2C_MASTER_STO;
                OS_I2CClockDown();
                return HAL_ERR_RESOURCE_TIMEOUT;
            }
        };

        i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

        // Transfert done

        // Check RxACK
        if (i2cMaster -> STATUS & I2C_MASTER_RXACK)
        {
            // Abort the transfert
            i2cMaster -> CMD = I2C_MASTER_STO ;
            while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
            {
                second_time = hal_TimGetUpTime();
                if (second_time - first_time > To)
                {
                	OS_I2CClockDown();
                    return HAL_ERR_RESOURCE_TIMEOUT;
                }
            };

            i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;
            OS_I2CClockDown();
            return HAL_ERR_COMMUNICATION_FAILED;
        }

        // Read all values but the last one
        for (currentByte=0; currentByte<Len-1 ; currentByte++)
        {
            // Read value
            i2cMaster -> CMD = I2C_MASTER_RD;

            // Polling on the TIP flag
            /*         while(i2cMaster -> STATUS & I2C_MASTER_TIP); */
            while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
            {
                second_time = hal_TimGetUpTime();
                if (second_time - first_time > To)
                {
                    i2cMaster->CMD = I2C_MASTER_STO;
                    OS_I2CClockDown();
                    return HAL_ERR_RESOURCE_TIMEOUT;
                }
            };

            i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

            // Store read value
            Buf[currentByte] = i2cMaster -> TXRX_BUFFER;
        }

        // Read last value - send no acknowledge - send stop condition/bit
        i2cMaster -> CMD = I2C_MASTER_RD | I2C_MASTER_ACK | I2C_MASTER_STO;

        // Polling on the TIP flag
        while(i2cMaster -> STATUS & I2C_MASTER_TIP)
        {
            second_time = hal_TimGetUpTime();
            if (second_time - first_time > To)
            {
            	OS_I2CClockDown();
                return HAL_ERR_RESOURCE_TIMEOUT;
            }
        };

        i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

        Buf[Len-1] = i2cMaster -> TXRX_BUFFER;
    }


    OS_I2CClockDown();
    return HAL_ERR_NO;
}

HAL_ERR_T OS_I2CXfer(HAL_I2C_BUS_ID_T BusId, u8 Addr, u8 *Reg, u8 RegNum, u8 *Buf, u8 Len, u8 WriteFlag, u32 To)
{
	HAL_ERR_T Error;
	u8 Retry = 0;
	u8 Result = 0;
	while(Retry < 2)
	{
		Error = hal_I2cOpen(BusId);
		if (Error)
		{
			Result = Error;
			Retry++;
			continue;
		}

		Error = OS_I2CGetData(BusId, Addr, Reg, RegNum, Buf, Len, WriteFlag, To);

		if (Error)
		{
			Result = Error;
			Retry++;
			continue;
		}
		else
		{
			Result = 0;
			break;
		}
	}
	hal_I2cClose(BusId);
	return Result;
}

u16 OS_GetVbatADC(void)
{
	return pmd_GetGpadcBatteryLevel();
}

void OS_PWMSetDuty(u8 Duty)
{
#if (__BOARD__ == __AIR201__) || (__BOARD__ == __AIR202__)
	hwp_pwm->PWL1_Config = PWM_PWL1_SET_OE|(PWM_PWL1_EN_H | PWM_PWL1_THRESHOLD(Duty));
	hal_PwmResourceMgmt();
	hwp_iomux->pad_GPIO_2_cfg = IOMUX_PAD_GPIO_2_SEL_FUN_PWL_1_SEL;
#endif
}

void OS_PWMStop(void)
{
#if (__BOARD__ == __AIR201__) || (__BOARD__ == __AIR202__)
	hwp_pwm->PWL1_Config = 0;
	hal_PwmResourceMgmt();
	hwp_iomux->pad_GPIO_2_cfg = IOMUX_PAD_GPIO_2_SEL_FUN_GPIO_2_SEL;
#endif
}

u8 OS_GetResetReason(void)
{
	return hal_SysGetResetCause();
}

void OS_StartTimer(HANDLE hTask, u8 nTimerId, u8 nMode, u32 nElapse)
{
	COS_KillTimer(hTask, nTimerId);
	COS_SetTimer(hTask, nTimerId, nMode, nElapse);
}
void OS_StopTimer(HANDLE hTask, u8 nTimerId)
{
	COS_KillTimer(hTask, nTimerId);
}

void OS_Sleep(u32 To)
{
	sxr_Sleep(To);
}

u8 OS_SendEvent(HANDLE hTask, u32 EventID, u32 Param1, u32 Param2, u32 Param3)
{
	COS_EVENT Event;
	Event.nEventId = EventID;
	Event.nParam1 = Param1;
	Event.nParam2 = Param2;
	Event.nParam3 = Param3;
	return COS_SendEvent(hTask, &Event, COS_WAIT_FOREVER, COS_EVENT_PRI_NORMAL);
}


void OS_GetIMEI(u8 *IMEI)
{
//	u8 Buf[128];
	u8 i;
//	u32 Addr = 0x003FE000;
	u8 *Temp = (u8 *)pal_GetImei(SIM_SN);
	if (Temp)
	{
		for (i = 0; i < IMEI_LEN; i++)
		{
			IMEI[i] = (Temp[i] >> 4) | (Temp[i] << 4);
		}
		IMEI[0] &= 0x0f;
	}
	else
	{
		memset(IMEI, 0, IMEI_LEN);
	}
//	__ReadFlash(Addr, Buf, 128);
//	__HexTrace(Buf, 16);
//	Addr = 0x003FC000;
//	__ReadFlash(Addr, Buf, 128);
//	__HexTrace(Buf, 40);
}

void OS_GetIMSI(u8 *IMSI, s8 *Str, u32 Len)
{
	u8 *Temp;
	u32 i,j;
	Len = Len & 0x000000ff;
	Temp = COS_MALLOC(Len + 1);
	memset(Temp, 0, Len + 1);
	j = 1;
	for (i = 0; i < Len; i++)
	{
		if (IsDigit(Str[i]))
		{
			Temp[j++] = Str[i] - '0';
		}
		else
		{
			CORE("OS_GetIMSI %c", Str[i]);
			COS_FREE(Temp);
			memset(IMSI, 0, IMSI_LEN);
			break;
		}
	}
	for (i = 0; i < IMSI_LEN; i++)
	{
		IMSI[i] = (Temp[i * 2] << 4) | (Temp[i * 2 + 1] & 0x0f);
	}
	COS_FREE(Temp);
}

void OS_GetIMSIReq(void)
{
	CFW_SimGetProviderId(UTI_GET_IMSI, SIM_SN);
}

void OS_FlyMode(u8 Switch)
{
	u32 nFM = 0;
	u32 Error;
	if (Switch)
	{
		Error = CFW_SetComm(CFW_DISABLE_COMM, 0, UTI_FLY_MODE, SIM_SN);
		if (Error != ERR_SUCCESS)
		{
			CORE("%s %u:%u",__FUNCTION__, __LINE__, Error);
		}
	}
	else
	{
		Error = CFW_GetComm((CFW_COMM_MODE *)&nFM, SIM_SN);
		if (nFM == CFW_DISABLE_COMM)
		{
			Error = CFW_SetComm(CFW_ENABLE_COMM, 0, UTI_FLY_MODE, SIM_SN);
			if (Error != ERR_SUCCESS)
			{
				CORE("%s %u:%u",__FUNCTION__, __LINE__, Error);
			}
		}
		else
		{
			CORE("%s %u:%u",__FUNCTION__, __LINE__, Error);
		}
	}
}

void OS_GetICCID(u8 *ICCID)
{
	u8 i;
	u8 *Temp = CFW_GetICCID(SIM_SN);
	if (Temp)
	{
		for (i = 0; i < ICCID_LEN; i++)
		{
			ICCID[i] = (Temp[i] >> 4) | (Temp[i] << 4);
		}
	}
	else
	{
		memset(ICCID, 0, ICCID_LEN);
	}
}

u8 OS_GetSimStatus(void)
{
	return CFW_GetSimStatus(SIM_SN);
}

void OS_GetGPRSAttach(u8 *State)
{
	CFW_GetGprsAttState(State, SIM_SN);
}

void OS_GetCellInfo(CFW_TSM_CURR_CELL_INFO *pCurrCellInfo, CFW_TSM_ALL_NEBCELL_INFO *pNeighborCellInfo)
{
	u32 Error = CFW_GetCellInfo(pCurrCellInfo, pNeighborCellInfo, SIM_SN);
	if (Error)
	{
		CORE("OS_GetCellInfo %x", Error);
	}
	else
	{
//		DBG("%x%x%x%x %u", gSys.CurrentCell.nTSM_LAI[3], gSys.CurrentCell.nTSM_LAI[4],
//				gSys.CurrentCell.nTSM_CellID[0], gSys.CurrentCell.nTSM_CellID[1],
//				gSys.CurrentCell.nTSM_AvRxLevel);
	}
}

u8 OS_GetRegStatus(void)
{
	CFW_NW_STATUS_INFO nStatusInfo;
    UINT32 nRet;
    nRet = CFW_NwGetStatus(&nStatusInfo, CFW_SIM_0);
    if (ERR_SUCCESS != nRet)
    {
    	return 0;
    }
    else
    {
    	return nStatusInfo.nStatus;
    }
}

void OS_GPRSActReq(u8 Req, u8 *APNName, u8 *APNUser, u8 *APNPassword)
{
	u32 Error;

	if (Req == CFW_GPRS_ACTIVED)
	{
		OS_SetCIPIPPdpCxt(APNName, APNUser, APNPassword);
		Error = CFW_GprsAct(Req, CID_IP, UTI_CID_IP_ACT, SIM_SN);
	}
	else
	{
		Error = CFW_GprsAct(Req, CID_IP, UTI_CID_IP_DEACT, SIM_SN);
	}
	if (Error)
	{
		CORE("OS_GPRSActReq %x", Error);
	}
}

void OS_GetGPRSActive(u8 *State)
{
	u32 Error = CFW_GetGprsActState(CID_IP, State, SIM_SN);
	if (Error)
	{
		CORE("OS_GetGPRSActive %x", Error);
	}
}

void OS_GetCIPIPPdpCxt(IP_AddrUnion *LocalIP, IP_AddrUnion *DNS)
{
	u32 Error;
	u8 IPSize;
	Error = CFW_GprsGetPdpAddr(CID_IP, &IPSize, LocalIP->u8_addr, SIM_SN);
	DNS->u32_addr = (u32)CFW_getDnsServerbyPdp(CID_IP, SIM_SN);
	if (Error)
	{
		CORE("OS_GetCIPIPPdpCxt %x", Error);
	}
}

void OS_SetCIPIPPdpCxt(u8 *APNName, u8 *APNUser, u8 *APNPassword)
{
	u32 Error;
	CFW_GPRS_PDPCONT_INFO PdpCont;
	//CFW_GPRS_QOS stTmpQos     = { 0, 0, 0, 0, 0 };
	//CFW_GPRS_QOS stTmpNullQos = { 3, 4, 3, 4, 16 };
	memset(&PdpCont, 0, sizeof(PdpCont));
	PdpCont.nPdpType = CFW_GPRS_PDP_TYPE_IP;
	PdpCont.nApnSize = strlen(APNName);
	PdpCont.nApnUserSize = strlen(APNUser);
	PdpCont.nApnPwdSize = strlen(APNPassword);
//	DBG("%u %u %u", PdpCont.nApnSize, PdpCont.nApnUserSize, PdpCont.nApnPwdSize);
	PdpCont.pApn = APNName;
	PdpCont.pApnUser = APNUser;
	PdpCont.pApnPwd = APNPassword;
//	CFW_GprsGetReqQos(CID_IP, &stTmpQos, SIM_SN);
//	CORE("OS_SetCIPIPPdpCxt %u %u %u %u %u", stTmpQos.nDelay, stTmpQos.nMean, stTmpQos.nPeak, stTmpQos.nPrecedence,
//			stTmpQos.nReliability);
//	if (!stTmpQos.nDelay)
//	{
//		CFW_GprsSetReqQos(CID_IP, &stTmpNullQos, SIM_SN);
//		CORE("OS_SetCIPIPPdpCxt %u %u %u %u %u", stTmpNullQos.nDelay, stTmpNullQos.nMean, stTmpNullQos.nPeak, stTmpNullQos.nPrecedence,
//			stTmpNullQos.nReliability);
//	}
	Error = CFW_GprsSetPdpCxt(CID_IP, &PdpCont, SIM_SN);
	if (Error)
	{
		CORE("OS_SetCIPIPPdpCxt %x", Error);
	}
}

void OS_StartTSM(void)
{
	u32 Error;
	CFW_TSM_FUNCTION_SELECT tSelecFUN;
	tSelecFUN.nNeighborCell = 1;
	tSelecFUN.nServingCell = 1;
	tSelecFUN.pad[0] = 0;
	tSelecFUN.pad[1] = 0;
	Error = CFW_EmodOutfieldTestStart(&tSelecFUN, UTI_TSM, SIM_SN);
	if (Error)
	{
		CORE("OS_StartTSM %x", Error);
	}
}

void OS_GPRSAttachReq(u8 Req)
{
	u32 Error;
	if (Req == CFW_GPRS_ATTACHED)
	{
		Error = CFW_GprsAtt(CFW_GPRS_ATTACHED, UTI_GPRS_ATTACH, SIM_SN);
	}
	else
	{
		Error = CFW_AttDetach(CFW_GPRS_DETACHED, UTI_GPRS_DETACH, API_PS_DETACH_GPRS, SIM_SN);
	}
	if (Error)
	{
		CORE("OS_GPRSAttachReq %x", Error);
	}
}

void OS_CallAccpet(void)
{
	UINT32 nRet = CFW_CcGetCallStatus(SIM_SN);
	if ((CC_STATE_INCOMING != nRet) && (CC_STATE_WAITING != nRet))
	{
		CORE("OS_CallAccpet %02x", nRet);
		return;
	}
	CFW_CcAcceptSpeechCall(SIM_SN);
}

void OS_CallRelease(void)
{
	UINT32 uStat = CFW_CcGetCallStatus(SIM_SN);
	if (CC_STATE_NULL == uStat)
	{
		return;
	}
	CFW_CcReleaseCall(SIM_SN);
}

void OS_SMSInitStart(u8 Param)
{
    CFW_SMS_STORAGE_INFO nStorageInfo;
    //ML_SetCodePage(ML_CP936);
    nStorageInfo.totalSlot = Param;
    CFW_CfgSetSmsStorageInfo(&nStorageInfo, CFW_SMS_STORAGE_SM, SIM_SN);
    nStorageInfo.totalSlot = PHONE_SMS_ENTRY_COUNT / NUMBER_OF_SIM;
    CFW_CfgSetSmsStorageInfo(&nStorageInfo, CFW_SMS_STORAGE_ME, SIM_SN);
    CFW_CfgSetSmsStorageInfo(&nStorageInfo, CFW_SMS_STORAGE_MT, SIM_SN);
    CFW_SmsMoInit(UTI_SMS_INIT, SIM_SN);
}

void OS_SMSInitFinish(u16 nUTI, CFW_SMS_PARAMETER *sInfo)
{
//	u8 Result;
	UINT32 nOperationRet = ERR_SUCCESS;
	UINT8 nOption        = 0;
	UINT8 nNewSmsStorage = 0;
	CFW_SmsInitComplete(nUTI, SIM_SN);
	nOperationRet = CFW_CfgSetSmsOverflowInd(1, SIM_SN);

	CFW_CfgGetSmsParam(sInfo, 0, SIM_SN);
	sInfo->ssr = (17 & 0x20) >> 5;
	sInfo->dcs = 4;
	nOperationRet = CFW_CfgSetSmsParam(sInfo, 0, SIM_SN);
	nOperationRet = CFW_CfgGetNewSmsOption(&nOption, &nNewSmsStorage, SIM_SN);
	nOption = (nOption & 0x1f) | CFW_SMS_ROUT_DETAIL_INFO;
	nOperationRet = CFW_CfgSetNewSmsOption(nOption, CFW_SMS_STORAGE_ME, SIM_SN);

	CFW_CfgSetSmsFormat(0, SIM_SN);
}

void OS_SMSGetStorageInfo(CFW_SMS_STORAGE_INFO *Info)
{
	CFW_CfgGetSmsStorageInfo(Info, CFW_SMS_STORAGE_ME, SIM_SN);
}

u32 OS_SMSTxByPDU(u8 *pData, u16 nDataSize)
{
	return CFW_SmsSendMessage(NULL, pData, nDataSize, UTI_SMS_SEND, SIM_SN);
}

u32 OS_GetHost(s8 *Name, struct ip_addr *IP)
{
	return CFW_Gethostbyname(Name, IP, CID_IP, SIM_SN);
}

SOCKET OS_CreateSocket(u8 nDomain, u8 nType, u8 nProtocol)
{
	return CFW_TcpipSocket(nDomain, nType, nProtocol);
}

u32 OS_SocketConnect(SOCKET SocketID, u32 LocalIP, u32 RemoteIP, u16 Port)
{
	CFW_TCPIP_SOCKET_ADDR nDestAddr;
	CFW_TCPIP_SOCKET_ADDR stLocalAddr;
	u32 Error;
	memset(&nDestAddr, 0, sizeof(CFW_TCPIP_SOCKET_ADDR));
	memset(&stLocalAddr, 0, sizeof(CFW_TCPIP_SOCKET_ADDR));
	stLocalAddr.sin_family = CFW_TCPIP_AF_INET;
	stLocalAddr.sin_addr.s_addr = LocalIP;
	Error = CFW_TcpipSocketBind(SocketID, &stLocalAddr, sizeof(CFW_TCPIP_SOCKET_ADDR));
	if (Error)
	{
		CORE("OS_SocketConnect %u", CFW_TcpipGetLastError());
		return CFW_TcpipGetLastError();
	}
	if (RemoteIP)
	{
		nDestAddr.sin_family = CFW_TCPIP_AF_INET;
		nDestAddr.sin_addr.s_addr = RemoteIP;
		nDestAddr.sin_port = htons(Port);
		Error = CFW_TcpipSocketConnect(SocketID, &nDestAddr, SIZEOF(CFW_TCPIP_SOCKET_ADDR));
		if (Error)
		{
			CORE("OS_SocketConnect %u", CFW_TcpipGetLastError());
			return CFW_TcpipGetLastError();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

u32 OS_SocketDisconnect(SOCKET SocketID)
{
	return CFW_TcpipSocketClose(SocketID);
}

u32 OS_SocketReceive(SOCKET SocketID, u8 *Buf, u32 Len, CFW_TCPIP_SOCKET_ADDR *from, INT32 *fromlen)
{
	return CFW_TcpipSocketRecvfrom(SocketID, Buf, Len, SIM_SN, from, fromlen);
}

u32 OS_SocketSend(SOCKET SocketID, u8 *Buf, u32 Len, CFW_TCPIP_SOCKET_ADDR *to, INT32 tolen)
{
	if (to)
	{
		return CFW_TcpipSocketSendto(SocketID, Buf, Len, 0, to, tolen);
	}
	else
	{
		return CFW_TcpipSocketSend(SocketID, Buf, Len, 0);
	}
}
