#pragma once
#include "WSocket.h"
#include "ConnectionModuleMessageWnd.h"

#define CMOD_RESERVED_EVENTS		4
#define CMOD_NUM_OF_SOCKETS			(WSA_MAXIMUM_WAIT_EVENTS - CMOD_RESERVED_EVENTS)

//
//
//
class IPPort;
class CCriticalSection;
class ConnectionManager;
class TorrentIndexRequest;
class TorrentIndexResult;

//
//
//
typedef struct ConnectionModuleThreadDataStruct
{
	WSAEVENT m_aReservedEvents[CMOD_RESERVED_EVENTS];
	queue<SOCKET> *m_pqAcceptedSockets;
	queue<TorrentIndexResult *> *m_pqTorrentSearchResults;
} ConnectionModuleThreadData;

//
//
//
class ConnectionModule
{
public:
	friend LRESULT ConnectionModuleMessageWnd::ConnectionsRepaired(WPARAM wparam, LPARAM lparam);

public:
	ConnectionModule(ConnectionManager *pManager, UINT mod);
	~ConnectionModule();
	inline unsigned int GetModNumber() { return m_nMod; };

	void InitThreadData(WPARAM wparam, LPARAM lparam);
	void LogMsg(WPARAM wparam, LPARAM lparam);

	void AddNewClient(SOCKET hSocket);
	void ClientConnect(IPPort *pIP);
	void ClientDisconnect(IPPort *pIP);

	void TorrentIndexReq(TorrentIndexRequest *pTorrentIndexRequest);
	void TorrentIndexRes(TorrentIndexResult *pTorrentIndexResponse);

	void RepairConnections(void);
private:
	void ConnectionsRepaired(int nCurrentConnections);

public:
	inline UINT ClientCountsReq(void) { return m_nClientConnections; };
	inline bool HasIdleSocket(void) { return ( !m_bRepairing && m_nClientConnections < (CMOD_NUM_OF_SOCKETS - 5) ); };

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

	CStdioFile m_fLogfile;
	char m_aLogFileBuf[1024];
};
