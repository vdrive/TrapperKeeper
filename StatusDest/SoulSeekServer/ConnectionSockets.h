// ConnectionSockets.h

#pragma once

#include "ParentSocketInterface.h"

class ConnectionSockets
{
public:
	ConnectionSockets(HWND hwnd);
	~ConnectionSockets();

	int AddParentConnection(SOCKET socket, char* theIP);

public:
	WSAEVENT m_events[WSA_MAXIMUM_WAIT_EVENTS];
	ParentSocketInterface m_sockets[WSA_MAXIMUM_WAIT_EVENTS-4];

public:
	HWND m_hwnd;
};