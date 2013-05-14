#pragma once
#include "ListeningSocket.h"
#include "IP.h"
#include <queue>
using namespace std;

#define WM_LISTENER_INIT_THREAD_DATA		WM_USER+1
#define WM_LISTENER_NEW_SOCKET				WM_USER+2
#define WM_LISTENER_STATUS					WM_USER+3
#define WM_LISTENER_LOG_MSG					WM_USER+4

#define CLIENTRECEIVER_RESERVED_EVENTS		2

class ClientReceiverThreadData
{
public:

	ClientReceiverThreadData(void)
		: m_eStatus(ListeningSocket::ListenerStatus::CLOSED)
	{
		memset( m_aReservedEvents, 0, sizeof(WSAEVENT) * CLIENTRECEIVER_RESERVED_EVENTS );
	}

	~ClientReceiverThreadData(void)
	{
	}

public:
	WSAEVENT								m_aReservedEvents[CLIENTRECEIVER_RESERVED_EVENTS];
	ListeningSocket::ListenerStatus			m_eStatus;
};
