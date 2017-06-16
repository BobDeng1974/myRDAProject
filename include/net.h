#ifndef __CUST_NET_H__
#define __CUST_NET_H__

enum
{
	NET_RES_NONE = 0,
	NET_RES_GET_IP_OK,
	NET_RES_GET_IP_FAIL,
	NET_RES_CONNECT_OK,
	NET_RES_CONNECT_FAIL,
	NET_RES_CLOSE_OK,
	NET_RES_WAIT_OK,
	NET_RES_SEND_OK,
	NET_RES_SEND_FAIL,
	NET_RES_UPLOAD,
	NET_RES_EVENT,
	NET_RES_NO_PORT,
	NET_RES_NO_SOCKETID,
	NET_RES_ERROR,
	NET_RES_TO,
};

typedef struct
{
	HANDLE TaskID;
	u32 TimerID;
	u32 To;
	MyAPIFunc ReceiveFun;
	Socket_DescriptStruct *Socket;
	in_addr IPAddr;
	u16 TCPPort;
	u16 UDPPort;
	SOCKET SocketID;
	u8 Channel;
	u8 Result;
	u8 Heart;
	u8 IsReceive;
}Net_CtrlStruct;
void Net_WaitTime(Net_CtrlStruct *Net);
void Net_WaitGPRSAct(Net_CtrlStruct *Net);
void Net_GetIP(Net_CtrlStruct *Net, s8 *Name);
void Net_Connect(Net_CtrlStruct *Net, u32 IP, s8 *Url);
void Net_Disconnect(Net_CtrlStruct *Net);
void Net_Send(Net_CtrlStruct *Net, u8 *Data, u32 Len);
void Net_WaitEvent(Net_CtrlStruct *Net);
void Net_WaitReceive(Net_CtrlStruct *Net);
void Net_WaitSpecialEvent(Net_CtrlStruct *Net, u32 EventID);
#endif
