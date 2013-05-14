#pragma once
#include "WSocket.h"
#include "ConnectionModuleMessageWnd.h"

#define CMOD_RESERVED_EVENTS		4
#define CMOD_NUM_OF_SOCKETS			(WSA_MAXIMUM_WAIT_EVENTS - CMOD_RESERVED_EVENTS)

//
//
//
class Torrent;
class IPPort;
class CCriticalSection;
class ConnectionManager;
class TorrentSearchRequest;
class TorrentSearchResult;
class ConnectionModuleThreadData;
class ConnectionModule
{
public:
	friend LRESULT ConnectionModuleMessageWnd::ConnectionsRepaired(WPARAM wparam, LPARAM lparam);

public:
	ConnectionModule(ConnectionManager *pManager, unsigned int mod, const unsigned char *pData,
		int nDataSize, int nClientType);
	~ConnectionModule();

	inline unsigned int GetModNumber() { return m_nMod; };

	void InitThreadData(WPARAM wparam, LPARAM lparam);
	void LogMsg(WPARAM wparam, LPARAM lparam);

	void AddNewClient(SOCKET hSocket);

	void ClientConnect(IPPort *pIP);
	void ClientDisconnect(IPPort *pIP);

	void TorrentSearch(TorrentSearchRequest *pTorrentSearchRequest);
	void TorrentSearch(TorrentSearchResult *pTorrentSearchResult);

	void RepairConnections(void);
private:
	void ConnectionsRepaired(int nCurrentConnections);

public:
	void TimerFired(void);

	inline unsigned int ClientCountsReq(void) { return m_nClientConnections; };
	inline bool HasIdleSocket(void) { return ( !m_bRepairing && m_nClientConnections < CMOD_NUM_OF_SOCKETS ); };

private:
	unsigned int m_nMod;
	ConnectionManager *m_pManager;

	ConnectionModuleMessageWnd m_wnd;
	CCriticalSection *m_pCriticalSection;
	ConnectionModuleThreadData *m_pThreadData;
	CWinThread* m_pThread;

	// accepted socket handles or verification requests that occured before the thread had started up
	queue<SOCKET> m_qInitialAcceptedConnections;

	int m_nClientConnections;
	bool m_bRepairing;
};
