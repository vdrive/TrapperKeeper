#include "ConnectionManager.h"
#include "ConnectionModule.h"
#include "BTDecoyClientDll.h"
#include "TorrentSearchResult.h"
#include "AcceptedSocket.h"
#include "Torrent.h"

//
//
//
ConnectionManager::ConnectionManager(void)
: m_pParent(NULL), m_pData(NULL), m_nDataSize(0), m_nClientType(1)
{
	m_vMods.clear();
	m_sClientsConnected.clear();
}

//
//
//
ConnectionManager::~ConnectionManager(void)
{
	for( int i=0; i < (int)m_vMods.size(); i++ )
		delete m_vMods[i];

	m_vMods.clear();

	m_pData = NULL;
	m_nDataSize = 0;
}

//
//
//
void ConnectionManager::AddModule(void)
{
	// Create a new module
	m_vMods.push_back( new ConnectionModule( this, (unsigned int)m_vMods.size(), m_pData, m_nDataSize, m_nClientType ) );
}

//
//
//
void ConnectionManager::AddNewClient(const AcceptedSocket *pSocket)
{
	//
	if( pSocket == NULL )
		return;

	if( m_sClientsConnected.count( pSocket->m_addr ) != 0 ) 
	{
		closesocket( pSocket->m_hSocket );
		delete pSocket;
		return;
	}

	// FOR every module
	for( int i = 0; i < (int)m_vMods.size(); i++ )
	{
		// IF the module has an idle socket
		if( m_vMods[i]->HasIdleSocket() )
		{
			// Add the connection to this module
			m_vMods[i]->AddNewClient( pSocket->m_hSocket );
			delete pSocket;
			return;
		}
	}

#ifdef _DEBUG
	if( m_vMods.size() < 8 )
#else
	if( m_vMods.size() < 64 )
#endif
	{
		// if none of the modules had an empty socket, create a new one
		AddModule();
		m_vMods[ m_vMods.size()-1 ]->AddNewClient( pSocket->m_hSocket );
		delete pSocket;
		return;
	}
	else
	{
		closesocket( pSocket->m_hSocket );
		delete pSocket;
		return;
	}
}

//
//
//
void ConnectionManager::ClientConnect(IPPort *pIP)
{
	if( pIP == NULL ) 
		return;

	// display log message that connection was made
	char *pLogMessage = new char[64];
	sprintf( pLogMessage, "Client %s connected", pIP->ToString(false).c_str() );
	LogMsg( pLogMessage );

	delete pIP;
}

//
// Handle the disconnection of a high ID client
//
void ConnectionManager::ClientDisconnect(IPPort *pIP)
{
	if( pIP == NULL )
		return;

	// remove from the connected client set
	m_sClientsConnected.erase( pIP->GetIP() );

	// remove from attempting connection set
	char *pLogMessage = new char[64];
	sprintf( pLogMessage, "Client %s disconnected", pIP->ToString(false).c_str() );
	LogMsg( pLogMessage );

	delete pIP;
}

//
//
//
void ConnectionManager::TorrentSearch(TorrentSearchRequest *pTorrentSearchRequest)
{
	m_pParent->TorrentSearch( pTorrentSearchRequest );
}

//
//
//
void ConnectionManager::TorrentSearch(TorrentSearchResult *pTorrentSearchResult)
{
	if( pTorrentSearchResult == NULL )
		return;

	if( pTorrentSearchResult->m_nConnectionModule < m_vMods.size() )
	{
		m_vMods[pTorrentSearchResult->m_nConnectionModule]->TorrentSearch( pTorrentSearchResult );
	}
	else
	{
		delete pTorrentSearchResult;
	}
}

//
// Returns the number of connected clients
//
unsigned int ConnectionManager::ClientCountsReq(void)
{
	unsigned int clients = 0;

	for( size_t i = 0, nLen = m_vMods.size(); i < nLen; i++ )
		clients += m_vMods[i]->ClientCountsReq();

	return max( clients, 0 );
}

//
// Copies the client ip:ports from the attempting connection and connected client sets to the given vectors
//
void ConnectionManager::GetConnectedClients(set<unsigned int>::iterator &rConnectedClientsIter, size_t &rConnectedClientsSize)
{
	rConnectedClientsIter = m_sClientsConnected.begin();
	rConnectedClientsSize = m_sClientsConnected.size();
}

//
// Sends a message to the connection log in the GUI
//
void ConnectionManager::LogMsg(const char *pLogMessage)
{
	if( pLogMessage != NULL )
	{
		m_pParent->LogMsg( pLogMessage );
		delete pLogMessage;
	}
}
