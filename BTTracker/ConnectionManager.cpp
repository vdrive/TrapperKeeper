#include "ConnectionManager.h"
#include "ConnectionModule.h"
#include "BTTrackerDll.h"
#include "TorrentIndexRequest.h"
#include "TorrentIndexResult.h"

//
//
//
ConnectionManager::ConnectionManager(void)
: m_pParent(NULL), m_nMaxUsers(0)
{
	m_vMods.clear();
	m_sClientsConnected.clear();
}

//
//
//
ConnectionManager::~ConnectionManager(void)
{
	KillModules();
}

//
//
//
void ConnectionManager::Init(BTTrackerDll *pParent, int nMaxUsers)
{
	m_pParent = pParent;
	m_nMaxUsers = nMaxUsers;
}

//
//
//
void ConnectionManager::AddModule(void)
{
	// Create a new module
	m_vMods.push_back( new ConnectionModule( this, (UINT)m_vMods.size() ) );
}

//
// Called when the dll is going to be removed...so that the threads (hopefully)
//  aren't still looking for the GUID cache when everything's freed
//
void ConnectionManager::KillModules(void)
{
	// Free memory
	for( int i=0; i < (int)m_vMods.size(); i++ )
	{
		delete m_vMods[i];
	}
	m_vMods.clear();
}

//
//
//
void ConnectionManager::AddNewClient(SOCKET hSocket)
{
	TRACE( "ConnectionManager::AddNewClient: New client\n" );

	// FOR every module
	for( int i = (int)(m_vMods.size() - 1); i >= 0; i-- )
	{
		// IF the module has an idle socket
		if( m_vMods[i]->HasIdleSocket() )
		{
			// Add the connection to this module
			m_vMods[i]->AddNewClient( hSocket );
			return;
		}
	}

	if( m_vMods.size() < 1000 )
	{
		// if none of the modules had an empty socket, create a new one
		AddModule();
		m_vMods[ m_vMods.size()-1 ]->AddNewClient( hSocket );
	}
	else
	{
		char *pMsg = new char[64];
		sprintf( pMsg, "m_vMods.size() < 1000 - killing connection" );
		LogMsg( pMsg );

		closesocket( hSocket );
	}
}

//
//
//
void ConnectionManager::ClientConnect(IPPort *pIP)
{
	if( pIP == NULL ) 
		return;

	TRACE( "Client %s connected", pIP->ToString(false).c_str() );

	// insert the client into the connectedClients set
//	m_sClientsConnected.insert( *pIP );
	m_pParent->m_nConnectedClients++;

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
//	m_sClientsConnected.erase( *pIP );
	m_pParent->m_nConnectedClients--;

	// remove from attempting connection set
	TRACE( "Client %s disconnected", pIP->ToString(false).c_str() );

	delete pIP;
}

//
//
//
void ConnectionManager::TorrentIndexReq(TorrentIndexRequest *pTorrentIndexRequest)
{
	m_pParent->TorrentIndexReq( pTorrentIndexRequest );
}

//
//
//
void ConnectionManager::TorrentIndexRes(TorrentIndexResult *pTorrentIndexResponse)
{
	if( pTorrentIndexResponse != NULL )
	{
		if( pTorrentIndexResponse->m_nMod <= m_vMods.size() )
		{
			m_vMods[ pTorrentIndexResponse->m_nMod ]->TorrentIndexRes( pTorrentIndexResponse );
		}
		else
		{
			delete pTorrentIndexResponse;
		}
	}
}

//
// Returns the number of connected (verified) clients
//
UINT ConnectionManager::ClientCountsReq(void)
{
	UINT clients = 0;

	for( size_t i = 0, nLen = m_vMods.size(); i < nLen; i++ )
		clients += m_vMods[i]->ClientCountsReq();

	return max( clients, 0 );
}

//
// Copies the client ip:ports from the attempting connection and connected client sets to the given vectors
//
void ConnectionManager::GetConnectedClients(set<IPPort>::iterator &rConnectedClientsIter, size_t &rConnectedClientsSize)
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
		m_pParent->CLogEntry( pLogMessage );
		delete pLogMessage;
	}
}

//
// Sends a message to the connection log in the GUI
//
void ConnectionManager::GetCurrentCounts(size_t &rnMods, size_t &rnClients)
{
	rnMods = m_vMods.size();
	rnClients = m_sClientsConnected.size();
}
