#pragma once
#include "StdAfx.h"
#include "WSocket.h"
#include "IPPort.h"

class BTTrackerDll;
class ConnectionModule;
class ConnectionModuleStatusData;
class TorrentIndexRequest;
class TorrentIndexResult;
class ConnectionManager
{
public:
	ConnectionManager(void);
	~ConnectionManager(void);

	void Init(BTTrackerDll *pParent, int nMaxUsers);
	inline UINT ReturnModCount(void) { return (UINT)m_vMods.size(); }

private:
	void AddModule(void);

public:
	void KillModules(void);

	void AddNewClient(SOCKET hSocket);

	void ClientConnect(IPPort *pIP);
	void ClientDisconnect(IPPort *pIP);

	void TorrentIndexReq(TorrentIndexRequest *pTorrentIndexRequest);
	void TorrentIndexRes(TorrentIndexResult *pTorrentIndexResponse);

	UINT ClientCountsReq(void);
	void GetConnectedClients(set<IPPort>::iterator &rConnectedClientsIter, size_t &rConnectedClientsSize);
	void LogMsg(const char *pLogMessage);

	void GetCurrentCounts(size_t &rMods, size_t &rClients);

private:
	// Private Data Members
	vector<ConnectionModule *>		m_vMods;
	BTTrackerDll *					m_pParent;

	int								m_nMaxUsers;

	set<IPPort>						m_sClientsConnected;
};
