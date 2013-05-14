// ConnectionSockets.h
#pragma once
#include "BTClient.h"
#include "IPPort.h"

class TorrentIndexResult;
class ConnectionSockets
{
public:
	// Public Member Functions
	ConnectionSockets();
	~ConnectionSockets();

	inline DWORD ReturnNumberOfEvents() { return NumberOfEvents; }
	inline int ReturnNumberOfReservedEvents() { return NumberOfReservedEvents; }
	inline int ReturnNumberOfSocketEvents() { return NumberOfSocketEvents; }

	void AddNewClient(SOCKET hSocket);
	void TorrentIndexRes(TorrentIndexResult *pTorrentIndexResponse);
	void KillClient(const IPPort &rIP);
	void RepairConnections(void);

	// Public Data Members
	const enum constants
	{
		NumberOfEvents = WSA_MAXIMUM_WAIT_EVENTS,
		NumberOfReservedEvents = 4,
		NumberOfSocketEvents = NumberOfEvents - NumberOfReservedEvents
	};

	WSAEVENT m_events[NumberOfEvents];
	BTClient m_sockets[NumberOfSocketEvents];

	HWND m_dlg_hwnd;
};