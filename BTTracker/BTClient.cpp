#include "StdAfx.h"
#include "BTClient.h"
#include "ByteBuffer.h"
#include "ConnectionSockets.h"
#include "TorrentIndexRequest.h"
#include "TorrentIndexResult.h"
#include "BTPacketParser.h"
#include "BTPacketGenerator.h"

#define MAX_RECV_BUFFER		0x1000

//
//
//
BTClient::BTClient(void)
: m_pParent(NULL)
{
	Init();
}

//
//
//
void BTClient::Destroy(void)
{
	m_bDestroying = true;
	Close();
}

//
//
//
BTClient::~BTClient(void)
{
	Destroy();
}

//
//
//
int BTClient::Close(void)
{
	int ret = __super::Close();

	if( !m_bDestroying )
	{
		::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_SOCKET_DISCONNECT, (WPARAM)new IPPort(m_clientIP), NULL );	
	}

	Init();
	return ret;
}

//
//
//
void BTClient::OnClose(int nErrorcode)
{
	// IF there was an error
	if( nErrorcode != 0 && nErrorcode != WSAECONNABORTED )
	{
		char *pLogMessage = new char[256];
		sprintf( pLogMessage, "ERROR: Client %s:%u - OnClose error! (code:%d)",
			m_clientIP.ToString(false).c_str(), m_clientIP.GetPort(), nErrorcode );
		::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_LOG_MSG, (WPARAM)pLogMessage, NULL );
	}

	Close();
}

//
//
//
void BTClient::Attach(SOCKET hSocket)
{
	// IF the socket is already connected, kill the connection
	if( IsSocket() )
		OnClose( 0 );

	// clear all of the variables before the new connection
	Init();

	// bind the socket and set the events
	m_hSocket = hSocket;
	EventSelect(FD_ALL_EVENTS);

	// get the new ip/port information
	sockaddr_in sinRemote;
	memset(&sinRemote,0,sizeof(sinRemote));
	int nAddrSize=sizeof(sinRemote);
	getpeername( hSocket, (sockaddr *)&sinRemote, &nAddrSize );

	// copy the ip/port information to the variables for quick lookup
	m_clientIP.SetIP( sinRemote.sin_addr.S_un.S_addr );
	m_clientIP.SetPort( sinRemote.sin_port );

	::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_SOCKET_CONNECT, (WPARAM)new IPPort(m_clientIP), NULL );	
}

//
//
//
int BTClient::SendSocketData(ByteBuffer &rBuf)
{
	if( rBuf.Size() == 0 )
		return 0;
	else
		return __super::SendSocketData( rBuf.GetMutableUcharPtr(), (UINT)rBuf.Size() );
}

//
//
//
int BTClient::SendSocketData(ByteBuffer *pBuf, bool bDelete /* = true */)
{
	if( pBuf == NULL )
		return 0;

	int ret = 0;
	if( pBuf->Size() != 0 )
	{
		ret = __super::SendSocketData( pBuf->GetMutableUcharPtr(), (UINT)pBuf->Size() );
	}

	if( bDelete )
		delete pBuf;

	return ret;
}

//
//
//
int BTClient::SendSocketData(unsigned char *pBuf, unsigned int nLen)
{
	if( pBuf == NULL || nLen == 0 )
		return 0;
	else
		return __super::SendSocketData( pBuf, nLen );
}

//
//
//
void BTClient::Init(void)
{
	m_clientIP.SetIP( 0 );
	m_clientIP.SetPort( 0 );

	m_mdPeer.SetIP( 0 );
	m_mdPeer.SetPort( 0 );

	m_bGarbage = false;
	m_bDestroying = false;

	m_receivedBufferReader.SetBuffer( (const BYTE *)NULL, 0 );

	m_bSupportsCompact = false;
}

//
//
//
bool BTClient::OnReceive(int nErrorcode)
{
	TRACE( "OnReceive: errorcode: %d\n", nErrorcode );

	if( nErrorcode != 0 )
	{
		// "OH CRAP"
		char *pLogMessage = new char[256];
		sprintf( pLogMessage, "ERROR: OnReceive error! (code:%d)", nErrorcode );
		::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_LOG_MSG, (WPARAM)pLogMessage, NULL );

		// Terminate the connection
		Close();
		return false;
	}

	// If we were receiving the socket data, keep working on it
	if( m_receiving_socket_data || m_receiving_some_socket_data )
	{
		ContinueToReceiveSocketData();
		return false;
	}

	TRY
	{
		// Try to receive a header
		int nRSDResult = ReceiveSomeSocketData( MAX_RECV_BUFFER );

		// IF the header receive did not fail
		if( nRSDResult == 0 )
		{
			// Check if there was a missed receive
			char buf[1];
			memset(buf,0,sizeof(buf));
			WSABUF wsabuf;
			wsabuf.buf=buf;
			wsabuf.len=1;

			DWORD num_read = 0;
			DWORD flags = MSG_PEEK;
			int nPeekResult = WSARecv( m_hSocket, &wsabuf, 1, &num_read, &flags, NULL, NULL );

			if( nPeekResult != SOCKET_ERROR && num_read != 0)
			{
				return true;
			}
		}
	}
	CATCH( CMemoryException, pEx )
	{
		if( pEx != NULL )
			delete pEx;
		Close();
	}
	END_CATCH

		return false;
}

//
//
//
void BTClient::SomeSocketDataReceived(char *data, unsigned int data_len, unsigned int new_len, unsigned int max_len)
{
	if( strstr( data, "\r\n\r\n" ) == NULL )
	{
		TRACE( "HTTP termination not found\n" );

		if( data_len < 0x800 )
			ReceiveSomeMoreSocketData( data, data_len, new_len, max_len );
		else
		{
			TRACE( "Too much data - terminating connection\n" );

			//char aFilename[256];
			//CString ip = m_clientIP.ToString().c_str();
			//sprintf( aFilename, "c:\\BTTracker\\%s - %s - DataHandler error.log", ip.Left( 15 ),
			//	CTime::GetCurrentTime().Format( "%m.%d %H_%M_%S" ) );
			//CFile fErrorFile;
			//if( fErrorFile.Open( aFilename, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyWrite ) )
			//{
			//	fErrorFile.Write( data, data_len );
			//	fErrorFile.Close();
			//}

			m_bGarbage = true;
			Close();
			return;
		}
	}
	else
	{
		TRACE( "HTTP termination found\n" );

		m_receivedBufferReader.SetBuffer( data, data_len );
		if( DataHandler() < 0 )
		{
			TRACE( "DataHandler Error - terminating connection\n" );

			//char aFilename[256];
			//CString ip = m_clientIP.ToString().c_str();
			//sprintf( aFilename, "c:\\BTTracker\\%s - %s - DataHandler error.log", ip.Left( 15 ),
			//	CTime::GetCurrentTime().Format( "%m.%d %H_%M_%S" ) );
			//CFile fErrorFile;
			//if( fErrorFile.Open( aFilename, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyWrite ) )
			//{
			//	fErrorFile.Write( data, data_len );
			//	fErrorFile.Close();
			//}

			m_bGarbage = true;
			Close();
			return;
		}
	}
}

//
//
//
int BTClient::DataHandler(void)
{	
	// IF the client has been marked as garbage, return error
	if( m_bGarbage )
		return -1;
	else if( m_receivedBufferReader.Size() == 0 )
		return 1;

	// Try to create a packet holder based on the received packet data
	BTPacketParser *pPacket = new BTPacketParser( m_receivedBufferReader.GetCharPtr(), m_receivedBufferReader.Size() );

	// Handle the packet depending on its type
	switch( pPacket->GetPacketType() )
	{
	case BTPacketParser::TYPE_GET_ANNOUNCE:				// GET_ANNOUNCE request
		{
			m_bSupportsCompact = pPacket->IsCompact();

			Peer peer;
			peer.SetIP( m_clientIP.GetIP() );
			peer.SetPort( pPacket->GetPort() );
			peer.UnSetMD();
			peer.UpdateLastSeen();

			// Post a message to the torrent manager
			::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_TORRENT_INDEX_REQUEST,
				(WPARAM)new TorrentIndexRequest( m_nIndex, m_clientIP, peer, pPacket ), NULL );
			return 0;
		}
		break;

	case BTPacketParser::TYPE_GET_SCRAPE:				// GET_SCRAPE request
		{
			m_bSupportsCompact = pPacket->IsCompact();

			// Post a message to the torrent manager
			::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_TORRENT_INDEX_REQUEST,
				(WPARAM)new TorrentIndexRequest( m_nIndex, m_clientIP, pPacket ), NULL );
			return 0;
		}
		break;

	case BTPacketParser::TYPE_MISSING_INFOHASH:
		{
			if( pPacket != NULL )
			{
				delete pPacket;
				pPacket = NULL;
			}

			::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_LOG_MSG, (WPARAM)dupstring( "No infohash received from client (killing connection)" ), NULL );

			ByteBuffer buf;
			BTPacketGenerator packGen;
			packGen.SetFailureReason( FAILURE_MISSING_INFOHASH );
			packGen.GetBencodedPacket( buf, m_bSupportsCompact );
			SendSocketData( buf );

			return -2;
		}

	case BTPacketParser::TYPE_NON_COMPACT:
		{
			if( pPacket != NULL )
			{
				delete pPacket;
				pPacket = NULL;
			}

			::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_LOG_MSG, (WPARAM)dupstring( "Client does not support compact peer list (killing connection)" ), NULL );

			ByteBuffer buf;
			BTPacketGenerator packGen;
			packGen.SetFailureReason( FAILURE_UNSUPPORTED_PROTOCOL );
			packGen.GetBencodedPacket( buf, m_bSupportsCompact );
			SendSocketData( buf );

			// kill the connection
			return -3;
		}

	case BTPacketParser::TYPE_INVALID_URL:
		{
			if( pPacket != NULL )
			{
				delete pPacket;
				pPacket = NULL;
			}

			::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_LOG_MSG, (WPARAM)dupstring( "Client received 404 error (killing connection)" ), NULL );

			// send 404 error
			SendSocketData( (UCHAR *)HTTP_404, (UINT)strlen(HTTP_404) );

			// kill the connection
			return -4;
		}

	default:
		{
			if( pPacket != NULL )
			{
				delete pPacket;
				pPacket = NULL;
			}

			::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_LOG_MSG, (WPARAM)dupstring( "Invalid packet received from client (killing connection)" ), NULL );

			ByteBuffer buf;
			BTPacketGenerator packGen;
			packGen.SetFailureReason( FAILURE_INVALID_PACKET );
			packGen.GetBencodedPacket( buf, m_bSupportsCompact );
			SendSocketData( buf );

			return -5;
		}
	}
}

//
//
//
void BTClient::TorrentIndexRes(TorrentIndexResult *pTorrentIndexResult)
{
	if( pTorrentIndexResult != NULL )
	{
		// Start constructing the reply packet
		ByteBuffer buf;
		BTPacketGenerator packet( pTorrentIndexResult );

		// Send the message to the client
		packet.GetBencodedPacket( buf, m_bSupportsCompact );
		SendSocketData( buf );

		delete pTorrentIndexResult;
	}

	Close();
}
