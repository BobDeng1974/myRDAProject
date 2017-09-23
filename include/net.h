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
	uint32_t TimerID;
	uint32_t To;
	MyAPIFunc ReceiveFun;
	Socket_DescriptStruct *Socket;
	in_addr IPAddr;
	uint16_t LocalPort;
	uint16_t TCPPort;
	uint16_t UDPPort;
	SOCKET SocketID;
	uint8_t Channel;
	uint8_t Result;
	uint8_t Heart;
	uint8_t IsReceive;
}Net_CtrlStruct;

struct socket_dsc
{
    struct netconn *conn;
    struct netbuf *lastdata;
    UINT16 lastoffset;
    UINT16 rcvevent;
    UINT16 sendevent;
    UINT16 flags;
    INT32 err;
};

#define sock_set_errno(sk, e) do { \
      sk->err = (e); \
} while (0)

void Net_WaitTime(Net_CtrlStruct *Net);
void Net_WaitGPRSAct(Net_CtrlStruct *Net);
void Net_GetIP(Net_CtrlStruct *Net, int8_t *Name);
void Net_Connect(Net_CtrlStruct *Net, uint32_t IP, int8_t *Url);
void Net_Disconnect(Net_CtrlStruct *Net);
void Net_Send(Net_CtrlStruct *Net, uint8_t *Data, uint32_t Len);
void Net_WaitEvent(Net_CtrlStruct *Net);
void Net_WaitReceive(Net_CtrlStruct *Net);
void Net_WaitSpecialEvent(Net_CtrlStruct *Net, uint32_t EventID);
int32_t Net_UDPRead(SOCKET SocketID, void *Buf, uint32_t RxLen);
#endif
