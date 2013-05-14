// ConnectionSockets.h
#pragma once
#include "Peer.h"
#include "IPPort.h"

class Torrent;
class TorrentSearchResult;
class ConnectionSockets
{
public:
	const enum constants
	{
		NumberOfEvents = WSA_MAXIMUM_WAIT_EVENTS,
		NumberOfReservedEvents = 4,
		NumberOfSocketEvents = NumberOfEvents - NumberOfReservedEvents
	};

public:
	// Public Member Functions
	ConnectionSockets(HWND hwnd, const unsigned char *pData, unsigned int nDataSize,
		int nClientType);
	~ConnectionSockets();

	inline unsigned long ReturnNumberOfEvents() { return NumberOfEvents; }
	inline int ReturnNumberOfReservedEvents() { return NumberOfReservedEvents; }
	inline int ReturnNumberOfSocketEvents() { return NumberOfSocketEvents; }

	void AddNewClient(SOCKET hSocket);

	void TorrentSearch(TorrentSearchResult *pResult);

	void KillClient(const IPPort &rIP);
	void RepairConnections(void);
	void TimerFired(void);

public:
	WSAEVENT m_events[NumberOfEvents];
	Peer m_sockets[NumberOfSocketEvents];

	HWND					m_dlg_hwnd;
	const unsigned char *	m_pData;
	unsigned int			m_nDataSize;
	int						m_nClientType;
};