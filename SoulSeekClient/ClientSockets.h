// ConnectionSockets.h

#pragma once

#include "ClientSocketInterface.h"
#include "PacketIO.h"

class ClientSockets
{
public:
	ClientSockets(HWND hwnd);
	~ClientSockets();

	void CheckForTimeout();
	int SendPacket(PacketWriter* packet);
	void ClearAll();

public:
	WSAEVENT m_events[WSA_MAXIMUM_WAIT_EVENTS];
	ClientSocketInterface m_sockets[WSA_MAXIMUM_WAIT_EVENTS-4];

public:
	HWND m_hwnd;
};