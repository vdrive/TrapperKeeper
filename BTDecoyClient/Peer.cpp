#include "StdAfx.h"
#include "Peer.h"
#include "ByteBuffer.h"
#include "ConnectionSockets.h"
#include "TorrentSearchRequest.h"
#include "TorrentSearchResult.h"

#define BT_CHOKE			0
#define BT_UNCHOKE			1
#define BT_INTERESTED		2
#define BT_NOT_INTERESTED	3
#define BT_HAVE				4
#define BT_BITFIELD			5
#define BT_REQUEST			6
#define BT_PIECE			7
#define BT_CANCEL			8

// helps to randomize the client id
#define BTCLIENT_AZUREUS2300_ID		"-AZ2300-"
#define BTCLIENT_AZUREUS2304_ID		"-AZ2304-"
#define BTCLIENT_TORRENTSTORM_ID	"-TS1300-"
#define BTCLIENT_OFFICIAL_414_ID	"M4-1-4--"
#define BTCLIENT_BITTORNADO_ID		"T037-----"
#define BTCLIENT_ABC_269_ID			"A269-----"
#define BTCLIENT_ABC_300_ID			"A300-----"

#define BTCLIENT_2ND_TIME_INTERVAL		43200		// 12 HOURS
#define BTCLIENT_3RD_TIME_INTERVAL		86400		// 24 HOURS
#define BTCLIENT_4TH_TIME_INTERVAL		172800		// 48 HOURS
#define BTCLIENT_5TH_TIME_INTERVAL		259200		// 72 HOURS
#define BTCLIENT_6TH_TIME_INTERVAL		345600		// 96 HOURS

//
//
//
Peer::Peer(void)
: m_pParent(NULL), m_nIndex(0), m_pRemoteBitField(NULL), m_nMaxFastPiecesSent(3), m_nMaxSlowPiecesSent(2)
, m_pTorrent(NULL)
{
	ZeroMemory( m_aKeepAliveMessage, sizeof( m_aKeepAliveMessage ) );

	m_aChokeMessage[0] = 0x0;
	m_aChokeMessage[1] = 0x0;
	m_aChokeMessage[2] = 0x0;
	m_aChokeMessage[3] = 0x1;
	m_aChokeMessage[4] = BT_CHOKE;

	m_aUnchokeMessage[0] = 0x0;
	m_aUnchokeMessage[1] = 0x0;
	m_aUnchokeMessage[2] = 0x0;
	m_aUnchokeMessage[3] = 0x1;
	m_aUnchokeMessage[4] = BT_UNCHOKE;

	m_aHaveMessage[0] = 0x0;
	m_aHaveMessage[1] = 0x0;
	m_aHaveMessage[2] = 0x0;
	m_aHaveMessage[3] = 0x5;
	m_aHaveMessage[4] = BT_HAVE;
	m_aHaveMessage[5] = 0x0;
	m_aHaveMessage[6] = 0x0;
	m_aHaveMessage[7] = 0x0;
	m_aHaveMessage[8] = 0x0;
	
	Init();
}

//
//
//
void Peer::Destroy(void)
{
	m_bDestroying = true;
	Close();
}

//
//
//
Peer::~Peer(void)
{
	Destroy();
}

//
//
//
int Peer::Close(void)
{
	int ret = __super::Close();

	// create offeredfiles removal request
	if( !m_bDestroying && m_clientIP.GetIP() != 0x00000000 )
	{
		::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_SOCKET_DISCONNECT, (WPARAM)new IPPort(m_clientIP), NULL );	
	}

	Init();
	return ret;
}

//
//
//
void Peer::OnClose(int nErrorcode)
{
	// IF there was an error
	if( nErrorcode != 0 && nErrorcode != WSAECONNABORTED )
	{
		char *pLogMessage = new char[64];
		sprintf( pLogMessage, "ERROR: Client %s:%u - OnClose error! (code:%d)",
			m_clientIP.ToString(false).c_str(), m_clientIP.GetPort(), nErrorcode );
		::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_LOG_MSG, (WPARAM)pLogMessage, NULL );
	}

	Close();
}

//
//
//
void Peer::Attach(SOCKET hSocket)
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
int Peer::SendSocketData(ByteBuffer &rBuf)
{
	if( rBuf.Size() == 0 )
		return 0;
	else
		return __super::SendSocketData( rBuf.GetMutableUcharPtr(), (UINT)rBuf.Size() );
}

//
//
//
int Peer::SendSocketData(ByteBuffer *pBuf, bool bDelete /* = true */)
{
	if( pBuf == NULL )
		return 0;

	int ret = __super::SendSocketData( pBuf->GetMutableUcharPtr(), (UINT)pBuf->Size() );

	if( bDelete )
		delete pBuf;

	return ret;
}

//
//
//
int Peer::SendSocketData(unsigned char *pBuf, unsigned int nLen)
{
	if( pBuf == NULL || nLen == 0 )
		return 0;
	else
		return __super::SendSocketData( pBuf, nLen );
}

//
//
//
void Peer::Init(void)
{
	m_clientIP.SetIP( 0 );
	m_clientIP.SetPort( 0 );

	m_bGarbage = false;
	m_bDestroying = false;

	m_bNeedHandshake = true;
	m_bGettingHandshake = true;
	m_bFinishingHandshake = true;
	m_bNeedInitialSearchResult = true;

	m_bGettingHeader = true;

	m_receivedBufferReader.SetBuffer( (const unsigned char *)NULL, 0 );

	m_sAvailablePieces.clear();

	ZeroMemory( m_aRemoteID, sizeof( m_aRemoteID ) );

	m_bNeverUnchokedPeer = true;
	m_bAmChokingPeer = true;
	m_bPeerChokingMe = true;
	m_bAmInterestedInPeer = false;
	m_bPeerInterestedInMe = false;

	m_bReceivedBitField = false;
	if( m_pRemoteBitField != NULL )
	{
		delete m_pRemoteBitField;
		m_pRemoteBitField = NULL;
	}
	m_bComplete = false;

	m_nTimerCount = 0;
	m_nLastReceiveCount = 0;

	m_vPieceRequests.clear();

	m_nSlowPercentageLimit = randInt( 25, 35 );
	SetTorrentSpeedFast();
	m_nMaxPiecesSent = m_nMaxFastPiecesSent;

	m_pTorrent = NULL;
}

//
//
//
void Peer::InitClientType(int nClientType)
{
	// Set the client ID at random
	for( int i = 0; i < BTCLIENT_ID_SIZE; i++ )
	{
		if( randInt( 0, 1 ) == 0 )
			m_aLocalID[i] = randByte( 0x41, 0x5A );
		else
			m_aLocalID[i] = randByte( 0x61, 0x7A );
	}

	switch( nClientType )
	{
	case 1:		// bittornado
        memcpy( m_aLocalID, BTCLIENT_AZUREUS2300_ID, strlen(BTCLIENT_AZUREUS2300_ID) );
		break;
	case 2:
        memcpy( m_aLocalID, BTCLIENT_AZUREUS2304_ID, strlen(BTCLIENT_AZUREUS2304_ID) );
		break;
	case 3:
        memcpy( m_aLocalID, BTCLIENT_TORRENTSTORM_ID, strlen(BTCLIENT_TORRENTSTORM_ID) );
		break;
	case 4:
        memcpy( m_aLocalID, BTCLIENT_BITTORNADO_ID, strlen(BTCLIENT_BITTORNADO_ID) );
		break;
	case 5:
        memcpy( m_aLocalID, BTCLIENT_ABC_269_ID, strlen(BTCLIENT_ABC_269_ID) );
		break;
	case 6:
        memcpy( m_aLocalID, BTCLIENT_ABC_300_ID, strlen(BTCLIENT_ABC_300_ID) );
		break;
	case 7:		// bitcomet
		m_aLocalID[0] = 'e';
		m_aLocalID[1] = 'x';
		m_aLocalID[2] = 'b';
		m_aLocalID[3] = 'c';
		m_aLocalID[4] = '\0';
		m_aLocalID[5] = 0x38;
		break;
	case 8:		// bitcomet
		m_aLocalID[0] = 'e';
		m_aLocalID[1] = 'x';
		m_aLocalID[2] = 'b';
		m_aLocalID[3] = 'c';
		m_aLocalID[4] = '\0';
		m_aLocalID[5] = 0x39;
		break;
	case 9:		// bitcomet
		m_aLocalID[0] = 'e';
		m_aLocalID[1] = 'x';
		m_aLocalID[2] = 'b';
		m_aLocalID[3] = 'c';
		m_aLocalID[4] = '\0';
		m_aLocalID[5] = 0x3A;
		break;
	case 10:
        memcpy( m_aLocalID, BTCLIENT_OFFICIAL_414_ID, strlen(BTCLIENT_OFFICIAL_414_ID) );
		break;
	}
}

//
//
//
bool Peer::OnReceive(int nErrorcode)
{
	if( nErrorcode != 0 )
	{
		// "OH CRAP"
		char *pLogMessage = new char[64];
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

	int nRSDResult;
	m_nLastReceiveCount = m_nTimerCount;

	TRY
	{
		if( m_bNeedHandshake )
		{
			// Try to get the length of the protocol string
			nRSDResult = ReceiveSocketData( 1 );
		}
		else
		{
			// Try to receive the length header
			nRSDResult = ReceiveSocketData( 4 );
		}
	}
	CATCH( CMemoryException, pEx )
	{
		TRACE( "ReceiveSocketData MemException\n" );

		if( pEx != NULL )
			delete pEx;

		Close();
		m_bGarbage = true;
		return false;
	}
	END_CATCH

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

	return false;
}

//
//
//
void Peer::SocketDataReceived(char *data, unsigned int data_len)
{
	// First connection with the client must begin by starting a handshake
	if( m_bNeedHandshake )
	{
		m_bNeedHandshake = false;

		// IF there 
		if( (unsigned int)(data[0] & 0x000000ff) + 20 + 8 > 256 )
		{
			m_bGarbage = true;
			Close();
			return;
		}
		
		// Try to get the length of the protocol string
		ReceiveSocketData( (unsigned int)(data[0] & 0x000000ff) + 20 + 8 );
	}
	// Receving the middle of the handshake
	else if( m_bGettingHandshake )
	{
		m_bGettingHandshake = false;

		m_receivedBufferReader.SetBuffer( data, data_len );

		// IF there is a infohash AND it is being offered by this client
		if( m_receivedBufferReader.ValidIndex( data_len - 20 ) )
		{
			TorrentSearch( m_receivedBufferReader.GetBytePtr( data_len - 20 ) );
			ReceiveSocketData( 20 );
		}
		else
		{
			TRACE( "else if( m_bGettingHandshake ) dropped connection due to error\n" );

			// Disconnect from client
			m_bGarbage = true;
			Close();
		}
	}
	// Receving the rest of the handshake
	else if( m_bFinishingHandshake )
	{
		m_bFinishingHandshake = false;

		m_receivedBufferReader.SetBuffer( data, data_len );

		// IF there is a infohash AND it is being offered by this client
		if( m_receivedBufferReader.ValidIndex( data_len - 20 ) )
		{
			memcpy( m_aRemoteID, m_receivedBufferReader.GetBytePtr( 0 ), BTCLIENT_ID_SIZE );
		}
		else
		{
			TRACE( "else if( m_bFinishingHandshake ) dropped connection due to error\n" );

			// Disconnect from client
			m_bGarbage = true;
			Close();
		}
	}
	else if( m_bGettingHeader )
	{
		m_bGettingHeader = false;

		// Try to receive the length header
		ReceiveSocketData( swapbytes( *((DWORD *)data) ) );
	}
	else
	{
		m_bGettingHeader = true;

		m_receivedBufferReader.SetBuffer( data, data_len );
		if( DataHandler() == -1 )
		{
			TRACE( "Peer::SocketDataReceived: DataHandler returned -1\n" );

			m_bGarbage = true;
			Close();
		}
	}
}

//
//
//
int Peer::DataHandler(void)
{	
	// IF the client has been marked as garbage, return error
	if( m_bGarbage )
		return -1;
	else if( m_receivedBufferReader.Size() == 0 )
		return 1;

	// IF the message is of zero length it is a keep alive message
	if( m_receivedBufferReader.Size() == 0 )
	{
		// TODO: keep alive timer(?)
	}
	// ELSE it is an actual message
	else
	{
		switch( m_receivedBufferReader.GetByte(0) )
		{
		case BT_CHOKE:
			{
//				TRACE( "Peer::DataHandler(void): Being choked\n" );
				m_bPeerChokingMe = true;
			}
			break;

		case BT_UNCHOKE:
			{
//				TRACE( "Peer::DataHandler(void): Being unchoked\n" );
				m_bPeerChokingMe = false;
			}
			break;

		case BT_INTERESTED:
			{
//				TRACE( "Peer::DataHandler(void): Peer is interested\n" );
				m_bPeerInterestedInMe = true;

				// Peer is interested, unchoke the peer and tell peer about it
				if( m_bNeverUnchokedPeer )
				{
					m_bNeverUnchokedPeer = false;

//					TRACE( "BT_INTERESTED: unchoking peer\n" );
					m_bAmChokingPeer = false;
					SendUnchoke();
				}
			}
			break;

		case BT_NOT_INTERESTED:
			{
//				TRACE( "Peer::DataHandler(void): Peer is uninterested\n" );
				m_bPeerInterestedInMe = false;
			}
			break;

		case BT_HAVE:
			{
//				TRACE( "Peer::DataHandler(void): Peer has piece %u\n", swapbytes( m_receivedBufferReader.GetDword( 1 ) ) );
				SetBit( swapbytes( m_receivedBufferReader.GetDword( 1 ) ) );

				if( m_eSpeed == FAST && PercentCompleted() > m_nSlowPercentageLimit )
				{
//					TRACE( "BT_HAVE: Peer has more than %d%% of file, switching to slow torrent\n", (int)m_nSlowPercentageLimit );

					SetTorrentSpeedSlow();
				}
			}
			break;

		case BT_BITFIELD:
			{
//				TRACE( "Peer::DataHandler(void): Peer sending bitfield\n" );

				// IF we have already received the bit field OR the size isn't quite right,
				//  something is wrong, kill the connection
				if( m_bReceivedBitField == true || m_pTorrent == NULL ||
					( m_bReceivedBitField == false && m_pTorrent->GetTotalLength() != 0 && m_receivedBufferReader.Size() - 1 != m_pTorrent->m_nBytesInBitfield ) ||
					( m_bReceivedBitField == false && m_pTorrent->GetTotalLength() == 0 && m_receivedBufferReader.Size() - 1 > 16384 ) )
				{
					return -1;
				}

				if( m_pRemoteBitField == NULL )
				{
					TRY
					{
						m_pRemoteBitField = new unsigned char[ m_receivedBufferReader.Size() - 1 ];
					}
					CATCH( CMemoryException, pEx )
					{
						::MessageBox( NULL, "Caught memory exception in Datahandler", "Datahandler Exception", MB_OK | MB_ICONEXCLAMATION );
						pEx->ReportError();
						Close();
					}
					END_CATCH
				}

				memcpy( m_pRemoteBitField, m_receivedBufferReader.GetBytePtr(1), m_receivedBufferReader.Size() - 1 );
				m_bReceivedBitField = true;

				if( m_eSpeed == FAST && PercentCompleted() > m_nSlowPercentageLimit )
				{
//					TRACE( "BT_HAVE: Peer has more than %d%% of file, switching to slow torrent\n", (int)m_nSlowPercentageLimit );
					SetTorrentSpeedSlow();
				}
			}
			break;

		case BT_REQUEST:
			{
				// IF the request is not the right size, return error
				if( m_receivedBufferReader.Size() != 13 )
					return -1;

				if( m_bAmChokingPeer )
				{
//					TRACE( "Peer::DataHandler(void): Peer request, but we are choking peer\n" );
					return 0;
				}

				PieceRequest pieceRequest( swapbytes( m_receivedBufferReader.GetInt( 1 ) ),
					swapbytes( m_receivedBufferReader.GetInt( 5 ) ), swapbytes( m_receivedBufferReader.GetInt( 9 ) ) );

				if( pieceRequest.m_nLength > 16384 )
				{
//					TRACE( "Peer::DataHandler(void): ERROR: Peer requested more than 16384 bytes of data\n" );
					return -1;
				}

				if( m_sAvailablePieces.count( pieceRequest.m_nIndex ) == 0 )
				{
					pieceRequest.m_nBegin = 0;
					pieceRequest.m_nIndex = 0;
					pieceRequest.m_nLength = 0;
					pieceRequest.m_bChoke = true;
				}

				UniqueVectorPushBack( pieceRequest, m_vPieceRequests );
			}
			break;

		case BT_PIECE:
			{
				// IF the request is not the right size
				if( m_receivedBufferReader.Size() <= 9 )
					return -1;

				// Why are we getting a piece?  We are a seed.
			}
			break;

		case BT_CANCEL:
			{
				// IF the cancel is not the right size, return error
				if( m_receivedBufferReader.Size() != 13 )
					return -1;

				PieceRequest pieceRequest( swapbytes( m_receivedBufferReader.GetInt( 1 ) ),
					swapbytes( m_receivedBufferReader.GetInt( 5 ) ), swapbytes( m_receivedBufferReader.GetInt( 9 ) ) );

				//TRACE( "Peer::DataHandler(void): Peer cancel: I/B/L: %d/%d/%d\n", pieceRequest.m_nIndex, pieceRequest.m_nBegin,
				//	pieceRequest.m_nLength );

				if( pieceRequest.m_nLength > 16384 )
				{
//					TRACE( "Peer::DataHandler(void): ERROR: Peer canceled(?) more than 16384 bytes of data\n" );
					return -1;
				}

				UniqueVectorRemove( pieceRequest, m_vPieceRequests );
			}
			break;

		default:
			{
				// Return error code since the id did not match on any of the established BT codes
				return -1;
			}
		}
	}

	return 0;
}

//
//
//
bool Peer::GetIndexedData(unsigned int nIndex, unsigned int nBegin, unsigned int nLength,
						  ByteBuffer &rBuf) const
{
	//TRACE( "Peer::GetIndexedData: nIndex: %u, nBegin: %u, nLength: %u, m_pData: %u\n",
	//	nIndex, nBegin, nLength, m_pParent->m_pData );

	// Check that we have all the data we need to process a request and that
	//  the request is valid
	if( m_pTorrent == NULL || m_pParent == NULL || m_pParent->m_pData == NULL || m_pParent->m_nDataSize == 0 ||
		m_pTorrent->GetPieceLength() == 0 || m_pTorrent->GetTotalLength() == 0 || nLength == 0 || nLength > 16384 ||
		( m_pTorrent->GetPieceLength() * nIndex ) + nBegin + nLength > m_pTorrent->GetTotalLength() )
		return false;

	// Calculate the starting byte number of the requested data
	unsigned int nDataStart = ( m_pTorrent->GetPieceLength() * nIndex ) + nBegin;

	// IF the starting byte of the requested data is before the end of the data
	if( nDataStart < m_pParent->m_nDataSize )
	{
		// IF the ending byte of the requested data is before the end of the data
		if( nDataStart + nLength <= m_pParent->m_nDataSize )
		{
			// No modifications need to be made to the data
			rBuf.PutBytePtr( (m_pParent->m_pData) + nDataStart, nLength );
			return true;
		}
		else
		{
			// Start of the data is taken from the end of the data file and the end is
			//  taken from the begining

			unsigned int nDataAvailible = m_pParent->m_nDataSize - nDataStart;

			rBuf.PutBytePtr( (m_pParent->m_pData) + nDataStart, nDataAvailible );
			if( nLength - nDataAvailible != 0 )
				rBuf.PutBytePtr( (m_pParent->m_pData) + 1, nLength - nDataAvailible );
			return true;
		}
	}
	else
	{
		unsigned int nDataSize = m_pParent->m_nDataSize;
		unsigned int nStartingOffset = 0;
		while( true )
		{
			if( nDataStart >= nDataSize )
			{
				nDataStart -= nDataSize;
				nDataSize--;
				nStartingOffset++;
			}
			else
			{
				break;
			}

			if( nDataSize == 0 )
			{
				nDataSize = m_pParent->m_nDataSize;
				nStartingOffset = 0;
			}
		}

		// IF the ending byte of the requested data is before the end of the data
		if( nDataStart + nStartingOffset + nLength <= m_pParent->m_nDataSize )
		{
			// No modifications need to be made to the data
			rBuf.PutBytePtr( (m_pParent->m_pData) + nDataStart + nStartingOffset, nLength );
			return true;
		}
		else
		{
			// Start of the data is taken from the end of the data file and the end is
			//  taken from the begining

			unsigned int nDataAvailible = m_pParent->m_nDataSize - ( nDataStart + nStartingOffset );

			rBuf.PutBytePtr( (m_pParent->m_pData) + nDataStart + nStartingOffset, nDataAvailible );
			if( nLength - nDataAvailible != 0 )
				rBuf.PutBytePtr( (m_pParent->m_pData) + nStartingOffset + 1, nLength - nDataAvailible );
			return true;
		}
	}
}

//
//
//
void Peer::TorrentSearch(const unsigned char *pHash) const
{
	if( pHash == NULL )
		return;

	::PostMessage( m_pParent->m_dlg_hwnd, WM_CMOD_TORRENT_SEARCH, 
		(WPARAM)new TorrentSearchRequest( pHash, m_nIndex, m_clientIP ), NULL );
}

//
//
//
void Peer::TorrentSearch(TorrentSearchResult *pResult)
{
	if( pResult == NULL )
		return;

	unsigned char *pLocalBitfield = NULL;

	if( pResult->m_ip == m_clientIP && m_bNeedInitialSearchResult )
	{
		m_pTorrent = pResult->m_pTorrent;
		m_bComplete = pResult->m_bComplete;
		ByteBuffer sendBuf( 68 );

		// Check for first send data
		if( m_pTorrent != NULL && m_pTorrent->IsTorrent() )
		{
			// Send the result back to the peer
			sendBuf.PutByte( 19 );
			sendBuf.PutCharPtr( "BitTorrent protocol" );
			sendBuf.PutDword( 0 );
			sendBuf.PutDword( 0 );
			sendBuf.PutBytePtr( m_pTorrent->GetInfoHashBytePtr(), 20 );
			sendBuf.PutCharPtr( m_aLocalID, 20 );
			SendSocketData( sendBuf );
		}
		else
		{
			TRACE( "Torrent not found, disconnecting client\n" );
			m_bGarbage = true;
			Close();
			return;
		}

		// Check for second send data
		if( m_pTorrent != NULL && m_pTorrent->IsTorrent() )
		{
			// Store info about the torrent we will be transmitting to this peer
			unsigned int nMinutes = (unsigned int)( CTime::GetCurrentTime() - m_pTorrent->GetActivationTime() ).GetTotalMinutes();
			for( set<TorrentPiece>::const_iterator iter = m_pTorrent->m_sSeededPieces.begin();
				iter != m_pTorrent->m_sSeededPieces.end(); iter++ )
			{
				if( nMinutes > iter->m_nTimeInterval )
				{
					m_sAvailablePieces.insert( iter->m_nPiece );
				}
			}

			// Alocate a buffer for the remote bitfield
			if( m_pRemoteBitField == NULL )
			{
				TRY
				{
					m_pRemoteBitField = new unsigned char[ m_pTorrent->m_nBytesInBitfield ];
					ZeroMemory( m_pRemoteBitField, m_pTorrent->m_nBytesInBitfield );
				}
				CATCH( CMemoryException, pEx )
				{
					::MessageBox( NULL, "Caught memory exception in TorrentSearch (loc: 1)", "TorrentSearch Exception", MB_OK | MB_ICONEXCLAMATION );
					pEx->ReportError();
					Close();
				}
				END_CATCH
			}

			// Resize the sending buffer to hold the local bitfield
			sendBuf.ResetBuffer();
			sendBuf.ResizeBuffer( 4 + 1 + m_pTorrent->m_nBytesInBitfield );
			sendBuf.PutDword( swapbytes( (DWORD)(1 + m_pTorrent->m_nBytesInBitfield) ) );

			sendBuf.PutByte( BT_BITFIELD );

			TRY
			{
				pLocalBitfield = new unsigned char[m_pTorrent->m_nBytesInBitfield];
			}
			CATCH( CMemoryException, pEx )
			{
				::MessageBox( NULL, "Caught memory exception in TorrentSearch (loc: 2)",
					"TorrentSearch Exception", MB_OK | MB_ICONEXCLAMATION );
				pEx->ReportError();
				Close();
			}
			END_CATCH

			if( m_bComplete )
			{
				memset( pLocalBitfield, 0xff, m_pTorrent->m_nBytesInBitfield );
				switch( m_pTorrent->GetBitsInBitfieldFinalByte() )
				{
				case 8:
					pLocalBitfield[m_pTorrent->m_nBytesInBitfield-1] = 0xFF;
					break;
				case 7:
					pLocalBitfield[m_pTorrent->m_nBytesInBitfield-1] = 0xFE;
					break;
				case 6:
					pLocalBitfield[m_pTorrent->m_nBytesInBitfield-1] = 0xFC;
					break;
				case 5:
					pLocalBitfield[m_pTorrent->m_nBytesInBitfield-1] = 0xF8;
					break;
				case 4:
					pLocalBitfield[m_pTorrent->m_nBytesInBitfield-1] = 0xF0;
					break;
				case 3:
					pLocalBitfield[m_pTorrent->m_nBytesInBitfield-1] = 0xE0;
					break;
				case 2:
					pLocalBitfield[m_pTorrent->m_nBytesInBitfield-1] = 0xC0;
					break;
				case 1:
					pLocalBitfield[m_pTorrent->m_nBytesInBitfield-1] = 0x80;
					break;
				}
			}
			else
			{
				memset( pLocalBitfield, 0x00, m_pTorrent->m_nBytesInBitfield );

				int nPercentDone = randInt( 30, 60 );
				nPercentDone = max( 30, min( 60, nPercentDone ) );

				unsigned int nCompletedPieces = (int)( m_pTorrent->m_nBitsInBitfield * nPercentDone / 100.0  );
				unsigned int nSettingPiece = 0;

				for( set<unsigned int>::iterator iter = m_sAvailablePieces.begin(); iter != m_sAvailablePieces.end(); iter++ )
				{
					switch( *iter % 8 )
					{
					case 0:
						pLocalBitfield[(*iter)/8] = pLocalBitfield[(*iter)/8] | 0x80;
						break;
					case 1:
						pLocalBitfield[(*iter)/8] = pLocalBitfield[(*iter)/8] | 0x40;
						break;
					case 2:
						pLocalBitfield[(*iter)/8] = pLocalBitfield[(*iter)/8] | 0x20;
						break;
					case 3:
						pLocalBitfield[(*iter)/8] = pLocalBitfield[(*iter)/8] | 0x10;
						break;
					case 4:
						pLocalBitfield[(*iter)/8] = pLocalBitfield[(*iter)/8] | 0x08;
						break;
					case 5:
						pLocalBitfield[(*iter)/8] = pLocalBitfield[(*iter)/8] | 0x04;
						break;
					case 6:
						pLocalBitfield[(*iter)/8] = pLocalBitfield[(*iter)/8] | 0x02;
						break;
					case 7:
						pLocalBitfield[(*iter)/8] = pLocalBitfield[(*iter)/8] | 0x01;
						break;
					}
				}

				nCompletedPieces -= (int)m_sAvailablePieces.size();

				for( unsigned int i = 0; i < nCompletedPieces; i++ )
				{
					nSettingPiece = randInt( 0, m_pTorrent->m_nBitsInBitfield-1 );
					switch( nSettingPiece % 8 )
					{
					case 0:
						pLocalBitfield[nSettingPiece/8] = pLocalBitfield[nSettingPiece/8] | 0x80;
						break;
					case 1:
						pLocalBitfield[nSettingPiece/8] = pLocalBitfield[nSettingPiece/8] | 0x40;
						break;
					case 2:
						pLocalBitfield[nSettingPiece/8] = pLocalBitfield[nSettingPiece/8] | 0x20;
						break;
					case 3:
						pLocalBitfield[nSettingPiece/8] = pLocalBitfield[nSettingPiece/8] | 0x10;
						break;
					case 4:
						pLocalBitfield[nSettingPiece/8] = pLocalBitfield[nSettingPiece/8] | 0x08;
						break;
					case 5:
						pLocalBitfield[nSettingPiece/8] = pLocalBitfield[nSettingPiece/8] | 0x04;
						break;
					case 6:
						pLocalBitfield[nSettingPiece/8] = pLocalBitfield[nSettingPiece/8] | 0x02;
						break;
					case 7:
						pLocalBitfield[nSettingPiece/8] = pLocalBitfield[nSettingPiece/8] | 0x01;
						break;
					}
				}
			}

			if( m_pTorrent->m_nBytesInBitfield == 0 )
			{
				TRACE( "m_pTorrent->m_nBytesInBitfield == 0: bytes: %d, bits: %d\n", m_pTorrent->m_nBytesInBitfield, m_pTorrent->m_nBitsInBitfield );
				TRACE( "pLocal %u, pResult %u, pLocalBitfield %u\n", pLocalBitfield, pResult, pLocalBitfield );
				ASSERT(false);
			}
			else
			{
				sendBuf.PutBytePtr( pLocalBitfield, m_pTorrent->m_nBytesInBitfield );
				SendSocketData( sendBuf );
			}
		}
		else
		{
			TRACE( "Torrent not found, disconnecting client\n" );
			m_bGarbage = true;
			Close();
		}
	}

	if( pLocalBitfield != NULL )
	{
		delete pLocalBitfield;
		pLocalBitfield = NULL;
	}

	delete pResult;
}

//
//
//
void Peer::TimerFired(void)
{
	if( m_pTorrent == NULL )
		return;
	
	m_nTimerCount++;

	// Check if more pieces need to be released
	if( m_nTimerCount % 1800 == 0 )
	{
//		TRACE( "Activation time is %s\n", m_pTorrent->GetActivationTime().Format( "%c" ) );
		unsigned int nMinutes = (unsigned int)( CTime::GetCurrentTime() - m_pTorrent->GetActivationTime() ).GetTotalMinutes();
		for( set<TorrentPiece>::const_iterator iter = m_pTorrent->m_sSeededPieces.begin();
			iter != m_pTorrent->m_sSeededPieces.end(); iter++ )
		{
			if( nMinutes > iter->m_nTimeInterval )
				m_sAvailablePieces.insert( iter->m_nPiece );
		}
	}

	// If we are incomplete, send a HAVE message every X seconds
    if( !m_bComplete && m_nTimerCount % 187 == 0 )
	{
		*(int *)(m_aHaveMessage + 5) = swapbytes( randInt( 0, m_pTorrent->m_nBitsInBitfield-1 ) );

		SendSocketData( m_aHaveMessage, sizeof(m_aHaveMessage) );
		return;
	}

	// Every X seconds fluctuate the speed of sent packets
	if( m_nTimerCount % 45 == 0 )
	{
		FluctuateSpeed();
	}

	// Send pieces if not choking the peer
	if( !m_bAmChokingPeer && !m_vPieceRequests.empty() &&
		( ( m_eSpeed == FAST && m_nTimerCount % 2 == 0 ) || ( m_eSpeed == SLOW && m_nTimerCount % 10 == 0 ) ) )
	{
		if( !SendPieces() )
		{
			TRACE( "Peer::TimerFired(void): Error while sending piece... closing\n" );
			m_bGarbage = true;
			Close();
			return;
		}
		else
		{
			return;
		}
	}

	// Unchoke if choking
	if( ( ( m_eSpeed == FAST && m_nTimerCount % 120 == 0 ) ||
		( m_eSpeed == SLOW && m_nTimerCount % 300 == 0 ) ) && m_bPeerInterestedInMe )
	{
//		TRACE( "Peer::TimerFired(void): Unchoking peer\n" );
		m_bAmChokingPeer = false;
		SendUnchoke();

		return;
	}

	// Send keep alive if doing nothing else
	if( m_nTimerCount % 120 == 0 )
	{
//		TRACE( "Peer::TimerFired(void): Sending keep-alive\n" );
		SendKeepAlive();
	}
}

//
//
//
bool Peer::SendPieces(void)
{
	if( m_vPieceRequests.empty() )
		return true;

	if( m_pTorrent->m_nBytesInBitfield == 0 )
	{
		TRACE( "***ERROR***: bool Peer::SendPieces(void): m_pTorrent->m_nBytesInBitfield == 0, returning false\n" );
		return false;
	}

	for( unsigned int i = 0; i < m_nMaxPiecesSent && !m_vPieceRequests.empty(); i++ )
	{
		if( m_vPieceRequests.begin()->m_bChoke )
		{
			m_bAmChokingPeer = true;
			SendChoke();

			m_vPieceRequests.clear();
			return true;
		}
		else
		{
			ByteBuffer sendBuf( 4 + 1 + 4 + 4 + m_vPieceRequests.begin()->m_nLength );
			sendBuf.PutInt( swapbytes( 9 + (DWORD)m_vPieceRequests.begin()->m_nLength ) );
			sendBuf.PutByte( BT_PIECE );
			sendBuf.PutInt( swapbytes( (DWORD)m_vPieceRequests.begin()->m_nIndex ) );
			sendBuf.PutInt( swapbytes( (DWORD)m_vPieceRequests.begin()->m_nBegin ) );

			// Try to get requested data from the data block.  If there was no error,
			//  send the data to the peer, otherwise return error.
			if( GetIndexedData( m_vPieceRequests.begin()->m_nIndex, m_vPieceRequests.begin()->m_nBegin,
				m_vPieceRequests.begin()->m_nLength, sendBuf ) )
			{
				SendSocketData( sendBuf );
			}
			else
			{
				m_vPieceRequests.clear();
				return false;
			}
		}

		if( !m_vPieceRequests.empty() )
			m_vPieceRequests.erase( m_vPieceRequests.begin() );
	}

	return true;
}

//
//
//
void Peer::SetTorrentSpeedFast(void)
{
	m_eSpeed = FAST;
	m_nMaxPiecesSent = m_nMaxFastPiecesSent;
}

//
//
//
void Peer::SetTorrentSpeedSlow(void)
{
	m_eSpeed = SLOW;
	m_nMaxPiecesSent = m_nMaxSlowPiecesSent;
}

//
//
//
void Peer::FluctuateSpeed(void)
{
	if( m_eSpeed == FAST )
		m_nMaxPiecesSent = randInt( 1, m_nMaxFastPiecesSent );
}

//
//
//
double Peer::PercentCompleted(void) const
{
	if( m_pRemoteBitField == NULL || m_pTorrent->m_nBytesInBitfield == 0 )
		return 0;

	unsigned int nBitsSet = 0;
	for( unsigned int i = 0; i < m_pTorrent->m_nBytesInBitfield; i++ )
	{
		if( (m_pRemoteBitField[i] & 0x80) == 0x80 )
			nBitsSet++;
		if( (m_pRemoteBitField[i] & 0x40) == 0x40 )
			nBitsSet++;
		if( (m_pRemoteBitField[i] & 0x20) == 0x20 )
			nBitsSet++;
		if( (m_pRemoteBitField[i] & 0x10) == 0x10 )
			nBitsSet++;
		if( (m_pRemoteBitField[i] & 0x08) == 0x08 )
			nBitsSet++;
		if( (m_pRemoteBitField[i] & 0x04) == 0x04 )
			nBitsSet++;
		if( (m_pRemoteBitField[i] & 0x02) == 0x02 )
			nBitsSet++;
		if( (m_pRemoteBitField[i] & 0x01) == 0x01 )
			nBitsSet++;
	}

	return (nBitsSet / (double)m_pTorrent->m_nBitsInBitfield) * 100;
}

//
//
//
void Peer::SetBit(size_t nBit)
{
	if( m_pRemoteBitField == NULL || (nBit / 8) > m_pTorrent->m_nBytesInBitfield )
		return;

	switch( nBit % 8 )
	{
	case 0:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] | 0x80;
		break;
	case 1:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] | 0x40;
		break;
	case 2:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] | 0x20;
		break;
	case 3:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] | 0x10;
		break;
	case 4:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] | 0x08;
		break;
	case 5:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] | 0x04;
		break;
	case 6:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] | 0x02;
		break;
	case 7:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] | 0x01;
		break;
	}
}

//
//
//
void Peer::ClearBit(size_t nBit)
{
	if( m_pRemoteBitField == NULL || (nBit / 8) > m_pTorrent->m_nBytesInBitfield )
		return;

	switch( nBit % 8 )
	{
	case 0:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] & 0x7F;
		break;
	case 1:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] & 0xBF;
		break;
	case 2:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] & 0xDF;
		break;
	case 3:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] & 0xEF;
		break;
	case 4:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] & 0xF7;
		break;
	case 5:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] & 0xFB;
		break;
	case 6:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] & 0xFD;
		break;
	case 7:
		m_pRemoteBitField[nBit/8] = m_pRemoteBitField[nBit/8] & 0xFE;
		break;
	}
}

