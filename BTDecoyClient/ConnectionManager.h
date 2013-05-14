#pragma once
#include "StdAfx.h"
#include "IPPort.h"
#include <winsock2.h>

class Torrent;
class AcceptedSocket;
class BTDecoyClientDll;
class ConnectionModule;
class ConnectionModuleStatusData;
class TorrentSearchRequest;
class TorrentSearchResult;
class ConnectionManager
{
public:
	ConnectionManager(void);
	~ConnectionManager(void);

	inline void InitParent(BTDecoyClientDll *pParent) { m_pParent = pParent; }
	inline void InitData(const unsigned char *pData) { m_pData = pData; }
	inline void InitDataSize(int nDataSize) { m_nDataSize = nDataSize; }
	inline void InitClientType(int nClientType) { m_nClientType = nClientType; }

	inline unsigned int ReturnModCount(void) { return (unsigned int)m_vMods.size(); }

	void AddModule(void);
	void AddNewClient(const AcceptedSocket *pSocket);

	void ClientConnect(IPPort *pIP);
	void ClientDisconnect(IPPort *pIP);

	void TorrentSearch(TorrentSearchRequest *pTorrentSearchRequest);
	void TorrentSearch(TorrentSearchResult *pTorrentSearchResult);

	unsigned int ClientCountsReq(void);
	void GetConnectedClients(set<unsigned int>::iterator &rConnectedClientsIter, size_t &rConnectedClientsSize);
	void LogMsg(const char *pLogMessage);

private:
	// Private Data Members
	vector<ConnectionModule *>		m_vMods;
	BTDecoyClientDll *				m_pParent;

	set<unsigned int>				m_sClientsConnected;

	const unsigned char *			m_pData;
	int								m_nDataSize;

	int								m_nClientType;
};
