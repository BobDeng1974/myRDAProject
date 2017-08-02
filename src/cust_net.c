#include "user.h"
#define __NET_DEBUG

void Net_WaitTime(Net_CtrlStruct *Net)
{
	u8 Finish = 0;
	COS_EVENT Event;
	Net->Result = NET_RES_NONE;
	GPRS_RegChannel(Net->Channel, Net->TaskID);
	if (Net->To)
	{
		OS_StartTimer(Net->TaskID, Net->TimerID, COS_TIMER_MODE_SINGLE, Net->To * SYS_TICK);
	}
	else
	{
		Net->Result = NET_RES_TO;
		return ;
	}
	while (!Finish)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
		switch (Event.nEventId)
		{
		case EV_TIMER:
			if (Event.nParam1 == Net->TimerID)
			{
				Net->Result = NET_RES_TO;
				Finish = 1;
			}
			else
			{
				OS_StopTimer(Net->TaskID, Event.nParam1);
			}

			break;
		case EV_MMI_NET_REC:
			if (Net->ReceiveFun)
			{
				Net->ReceiveFun((void *)Event.nParam1);
			}
			break;
		}
	}
	OS_StopTimer(Net->TaskID, Net->TimerID);
	return ;
}

void Net_WaitGPRSAct(Net_CtrlStruct *Net)
{
	u8 Finish = 0;
	COS_EVENT Event;
	Net->Result = NET_RES_NONE;

	if (GPRS_RUN == gSys.State[GPRS_STATE])
	{
		Net->Result = NET_RES_GET_IP_OK;
		return ;
	}
	DBG("%u wait for active!", Net->Channel);
	if (Net->To)
	{
		OS_StartTimer(Net->TaskID, Net->TimerID, COS_TIMER_MODE_SINGLE, 90 * SYS_TICK);
	}
	while (!Finish)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
		switch (Event.nEventId)
		{
		case EV_TIMER:
			if (Event.nParam1 == Net->TimerID)
			{
				DBG("To!");
				Net->Result = NET_RES_TO;
				Finish = 1;
			}
			else
			{
				OS_StopTimer(Net->TaskID, Event.nParam1);
			}
			break;
		case EV_MMI_GPRS_READY:
			Net->Result = NET_RES_GET_IP_OK;
			Finish = 1;
			break;
		}
	}
	OS_StopTimer(Net->TaskID, Net->TimerID);
	return ;
}

void Net_GetIP(Net_CtrlStruct *Net, s8 *Name)
{
	u8 Finish = 0;
	COS_EVENT Event;
	Net->Result = NET_RES_NONE;
	Net->IPAddr.s_addr = 0;

	if (GPRS_RegDNS(Net->Channel, Name) < 0)
	{
		DBG("!");
		Net->Result = NET_RES_GET_IP_FAIL;
		return;
	}
	if (Net->To)
	{
		OS_StartTimer(Net->TaskID, Net->TimerID, COS_TIMER_MODE_SINGLE, (30) * SYS_TICK);
	}
	while (!Finish)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
		switch (Event.nEventId)
		{
		case EV_TIMER:
			if (Event.nParam1 == Net->TimerID)
			{
				DBG("To!");
				Net->Result = NET_RES_TO;
				Finish = 1;
			}
			else
			{
				OS_StopTimer(Net->TaskID, Event.nParam1);
			}
			break;
		case EV_MMI_GPRS_GET_HOST:
			if (Event.nParam1)
			{
				Net->IPAddr.s_addr = Event.nParam2;
				Net->Result = NET_RES_GET_IP_OK;
			}
			else
			{
				Net->Result = NET_RES_GET_IP_FAIL;
			}
			Finish = 1;
			break;
		}
	}
	OS_StopTimer(Net->TaskID, Net->TimerID);
	return ;
}

void Net_Connect(Net_CtrlStruct *Net, u32 IP, s8 *Url)
{
	u16 Port = 0;
	u8 Finish = 0;
	COS_EVENT Event;
	IP_AddrUnion uIP;
	u32 Error;
	GPRS_RegChannel(Net->Channel, Net->TaskID);
	Net_WaitGPRSAct(Net);
	if (Net->Result != NET_RES_GET_IP_OK)
	{
		Net->Result = NET_RES_CONNECT_FAIL;
		return ;
	}

	if (IP)
	{
		Net->IPAddr.s_addr = IP;
	}
	else
	{
		Net_GetIP(Net, Url);
		if (Net->Result != NET_RES_GET_IP_OK)
		{
			DBG("%s Get ip fail!", Url);
			Net->Result = NET_RES_CONNECT_FAIL;
			return ;
		}
	}

	Net->Result = NET_RES_NONE;
	if (Net->TCPPort)
	{
		Net->SocketID = OS_CreateSocket(CFW_TCPIP_AF_INET, CFW_TCPIP_SOCK_STREAM, CFW_TCPIP_IPPROTO_TCP);
		Port = Net->TCPPort;
#ifdef __NET_DEBUG
		DBG("TCP !");
#endif
	}
	else if (Net->UDPPort)
	{
		Net->SocketID = OS_CreateSocket(CFW_TCPIP_AF_INET, CFW_TCPIP_SOCK_DGRAM, CFW_TCPIP_IPPROTO_UDP);
		Port = Net->UDPPort;
#ifdef __NET_DEBUG
		DBG("UDP !");
#endif
	}
	else
	{
		DBG("no port!");
		Net->SocketID = INVALID_SOCKET;
		Net->Result = NET_RES_NO_PORT;
		return ;
	}
	if (INVALID_SOCKET == Net->SocketID)
	{
		DBG("create socket fail reboot!");
		Net->Result = NET_RES_NO_SOCKETID;
		return ;
	}
	uIP.u32_addr = Net->IPAddr.s_addr;
	DBG("%u Connect to %u.%u.%u.%u %u",Net->Channel, uIP.u8_addr[0], uIP.u8_addr[1], uIP.u8_addr[2], uIP.u8_addr[3], Port);
	GPRS_RegSocket(Net->Channel, Net->SocketID);
	Error = OS_SocketConnect(Net->SocketID, gSys.LocalIP.u32_addr, Net->IPAddr.s_addr, Port);
	Net->Socket = (Socket_DescriptStruct *)get_socket(Net->SocketID);
	if (Error)
	{
		DBG("%u Connect fail!");
		Net->Result = NET_RES_CONNECT_FAIL;
		return ;
	}
	if (Net->UDPPort)
	{
		Net->Result = NET_RES_CONNECT_OK;
		return ;
	}

	if (Net->To)
	{
		OS_StartTimer(Net->TaskID, Net->TimerID, COS_TIMER_MODE_SINGLE, Net->To * SYS_TICK);
	}
	while (!Finish)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
		switch (Event.nEventId)
		{
		case EV_TIMER:
			if (Event.nParam1 == Net->TimerID)
			{
				DBG("%u Connect To!", Net->Channel);
				Net->Result = NET_RES_TO;
				Finish = 1;
			}
			else
			{
				OS_StopTimer(Net->TaskID, Event.nParam1);
			}
			break;
		case EV_MMI_NET_CONNECT_OK:
			DBG("%u Connect OK!", Net->Channel);
			Net->Result = NET_RES_CONNECT_OK;
			Finish = 1;
			break;
		case EV_MMI_NET_ERROR:
			DBG("%u Connect Fail!", Net->Channel);
			Net->Result = NET_RES_CONNECT_FAIL;
			Finish = 1;
			break;
		}
	}
	OS_StopTimer(Net->TaskID, Net->TimerID);
	return ;
}

void Net_Disconnect(Net_CtrlStruct *Net)
{
	u8 Finish = 0;
	COS_EVENT Event;
	Net->Socket = NULL;
	Net->Result = NET_RES_NONE;
	OS_SocketDisconnect(Net->SocketID);
//	if (Net->To > 30)
//	{
//		OS_StartTimer(Net->TaskID, Net->TimerID, COS_TIMER_MODE_SINGLE, Net->To * SYS_TICK);
//	}
//	else
//	{
		OS_StartTimer(Net->TaskID, Net->TimerID, COS_TIMER_MODE_SINGLE, 30 * SYS_TICK);
//	}
	while (!Finish)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
		switch (Event.nEventId)
		{
		case EV_TIMER:
			if (Event.nParam1 == Net->TimerID)
			{
				DBG("%u, Close To!", Net->Channel);
				Net->Result = NET_RES_TO;
				Finish = 1;
			}
			else
			{
				OS_StopTimer(Net->TaskID, Event.nParam1);
			}
			break;
		case EV_MMI_NET_CLOSED:
			DBG("%u, Close OK!", Net->Channel);
			Net->Result = NET_RES_CLOSE_OK;
			Finish = 1;
			break;
		}
	}
	Net->SocketID = INVALID_SOCKET;
	GPRS_ResetSocket(Net->Channel);
	OS_StopTimer(Net->TaskID, Net->TimerID);
	return ;
}

void Net_Send(Net_CtrlStruct *Net, u8 *Data, u32 Len)
{
	u8 Finish = 0;
	COS_EVENT Event;
	Net->Result = NET_RES_NONE;
	CFW_TCPIP_SOCKET_ADDR DestAddr;
	if (Net->TCPPort)
	{

		OS_SocketSend(Net->SocketID, Data, Len, NULL, NULL);
	}
	else
	{
		memset(&DestAddr, 0, sizeof(CFW_TCPIP_SOCKET_ADDR));
		DestAddr.sin_family = CFW_TCPIP_AF_INET;
		DestAddr.sin_port = htons(Net->UDPPort);
		DestAddr.sin_len = 4;
		DestAddr.sin_addr = Net->IPAddr;
		OS_SocketSend(Net->SocketID, Data, Len, &DestAddr, DestAddr.sin_len);
		Net->Result = NET_RES_SEND_OK;
		return ;
	}

	if (Net->To)
	{
		OS_StartTimer(Net->TaskID, Net->TimerID, COS_TIMER_MODE_SINGLE, Net->To * SYS_TICK);
	}
	Net->IsReceive = 0;
	while (!Finish)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
		switch (Event.nEventId)
		{
		case EV_TIMER:
			if (Event.nParam1 == Net->TimerID)
			{
				DBG("To!");
				Net->Result = NET_RES_TO;
				Finish = 1;
			}
			else
			{
				OS_StopTimer(Net->TaskID, Event.nParam1);
			}
			break;
		case EV_MMI_MONITOR_WAKEUP:
			break;
		case EV_MMI_NET_SEND_OK:
			Net->Result = NET_RES_SEND_OK;
			Finish = 1;
			break;
		case EV_MMI_NET_REMOTE_CLOSE:
		case EV_MMI_NET_ERROR:
			Net->Result = NET_RES_SEND_FAIL;
			Finish = 1;
			break;
		case EV_MMI_NET_REC:
			if (Net->ReceiveFun)
			{
				Net->ReceiveFun((void *)Event.nParam1);
			}
			break;
		case EV_MMI_MONITOR_HEART:
			Net->Heart = 1;
			break;
		}
	}
	OS_StopTimer(Net->TaskID, Net->TimerID);

	return ;
}

void Net_WaitEvent(Net_CtrlStruct *Net)
{
	u8 Finish = 0;
	COS_EVENT Event;
	Net->Result = NET_RES_NONE;

	if (Net->To)
	{
		OS_StartTimer(Net->TaskID, Net->TimerID, COS_TIMER_MODE_SINGLE, Net->To * SYS_TICK);
	}
	while (!Finish)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
		switch (Event.nEventId)
		{
		case EV_TIMER:
			if (Event.nParam1 == Net->TimerID)
			{
				//DBG("To!");
				Net->Result = NET_RES_TO;
				Finish = 1;
			}
			else
			{
				OS_StopTimer(Net->TaskID, Event.nParam1);
			}
			break;
		case EV_MMI_MONITOR_WAKEUP:
			DBG("Wakeup!");
			Net->Result = NET_RES_UPLOAD;
			Finish = 1;
			break;
		case EV_MMI_NET_REC:
			if (Net->ReceiveFun)
			{
				Net->ReceiveFun((void *)Event.nParam1);
			}
			Net->Result = NET_RES_UPLOAD;
			Finish = 1;
			break;
		case EV_MMI_MONITOR_HEART:
			Net->Result = NET_RES_UPLOAD;
			Net->Heart = 1;
			Finish = 1;
			break;
		case EV_MMI_MONITOR_UPLOAD:
			Net->Result = NET_RES_UPLOAD;
			Finish = 1;
			break;
		case EV_MMI_NET_REMOTE_CLOSE:
		case EV_MMI_NET_ERROR:
			Net->Result = NET_RES_ERROR;
			Finish = 1;
			break;
		}
	}
	OS_StopTimer(Net->TaskID, Net->TimerID);
	return ;
}

void Net_WaitSpecialEvent(Net_CtrlStruct *Net, u32 EventID)
{
	u8 Finish = 0;
	COS_EVENT Event;
	Net->Result = NET_RES_NONE;

	if (Net->To)
	{
		OS_StartTimer(Net->TaskID, Net->TimerID, COS_TIMER_MODE_SINGLE, Net->To * SYS_TICK);
	}
	while (!Finish)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
		switch (Event.nEventId)
		{
		case EV_TIMER:
			if (Event.nParam1 == Net->TimerID)
			{
				DBG("To!");
				Net->Result = NET_RES_TO;
				Finish = 1;
			}
			else
			{
				OS_StopTimer(Net->TaskID, Event.nParam1);
			}
			break;
		case EV_MMI_MONITOR_WAKEUP:
			DBG("Wakeup!");
			Net->Result = NET_RES_UPLOAD;
			Finish = 1;
			break;
		case EV_MMI_NET_REC:
			if (Net->ReceiveFun)
			{
				Net->ReceiveFun((void *)Event.nParam1);
			}
			Net->Result = NET_RES_UPLOAD;
			Finish = 1;
			break;
		case EV_MMI_MONITOR_HEART:
			Net->Result = NET_RES_UPLOAD;
			Net->Heart = 1;
			Finish = 1;
			break;
		case EV_MMI_MONITOR_UPLOAD:
			Net->Result = NET_RES_UPLOAD;
			Finish = 1;
			break;
		case EV_MMI_NET_REMOTE_CLOSE:
		case EV_MMI_NET_ERROR:
			Net->Result = NET_RES_ERROR;
			Finish = 1;
			break;
		default:
			if (EventID == Event.nEventId)
			{
				Net->Result = NET_RES_EVENT;
				Finish = 1;
			}
			break;
		}
	}
	OS_StopTimer(Net->TaskID, Net->TimerID);
	return ;
}

void Net_WaitReceive(Net_CtrlStruct *Net)
{
	u8 Finish = 0;
	COS_EVENT Event;
	Net->Result = NET_RES_NONE;

	if (Net->To)
	{
		OS_StartTimer(Net->TaskID, Net->TimerID, COS_TIMER_MODE_SINGLE, Net->To * SYS_TICK);
	}
	while (!Finish)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
		switch (Event.nEventId)
		{
		case EV_TIMER:
			if (Event.nParam1 == Net->TimerID)
			{
				DBG("To!");
				Net->Result = NET_RES_TO;
				Finish = 1;
			}
			else
			{
				OS_StopTimer(Net->TaskID, Event.nParam1);
			}
			break;
		case EV_MMI_NET_REC:
			if (Net->ReceiveFun)
			{
				Net->ReceiveFun((void *)Event.nParam1);
			}
			Net->Result = NET_RES_UPLOAD;
			Finish = 1;
			break;
		case EV_MMI_NET_REMOTE_CLOSE:
		case EV_MMI_NET_ERROR:
			Net->Result = NET_RES_ERROR;
			Finish = 1;
			break;
		}
	}
	OS_StopTimer(Net->TaskID, Net->TimerID);
	return ;
}
