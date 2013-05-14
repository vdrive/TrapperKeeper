#pragma once
#include "IPPort.h"
#include "ByteBufferReader.h"
#include "PieceRequest.h"
#include "TEventSocket.h"
#include "Torrent.h"

#define BTCLIENT_ID_SIZE			20

//
//
//
class ByteBuffer;
class ConnectionSockets;
class TorrentSearchResult;
class Peer
	: public TEventSocket
{
// functions
public:
	Peer(void);
	~Peer(void);

	void Destroy(void);

	int Close(void);
	void OnClose(int nErrorcode);

public:
	void Attach(SOCKET hSocket);

private:
	int SendSocketData(ByteBuffer &rBuf);
	int SendSocketData(ByteBuffer *pBuf, bool bDelete = true);
	int SendSocketData(unsigned char *pBuf, unsigned int nLen);

	void Init(void);
public:
	inline void InitParent(ConnectionSockets* pParent) { m_pParent = pParent; };
	inline void InitIndex(size_t nIndex) { m_nIndex = nIndex; }
	void InitClientType(int nClientType);

	bool OnReceive(int nErrorCode);

private:
	void SocketDataReceived(char *data, unsigned int data_len);
	int DataHandler(void);
	bool GetIndexedData(unsigned int nIndex, unsigned int nBegin, unsigned int nLength, ByteBuffer &rBuf) const;

public:
	void TorrentSearch(const unsigned char *pHash) const;
	void TorrentSearch(TorrentSearchResult *pResult);

	void TimerFired(void);
	inline unsigned int TimerEventsSinceLastReceive(void) { return m_nTimerCount - m_nLastReceiveCount; }

	// Returns the client ip/port holder
	inline IPPort GetClientIP(void) const { return m_clientIP; };

private:
	inline void SendKeepAlive(void) { SendSocketData( m_aUnchokeMessage, 5 ); }
	inline void SendChoke(void) { SendSocketData( m_aChokeMessage, 5 ); }
	inline void SendUnchoke(void) { SendSocketData( m_aUnchokeMessage, 5 ); }

	bool SendPieces(void);
	void SetTorrentSpeedFast(void);
	void SetTorrentSpeedSlow(void);
	void FluctuateSpeed(void);

	// bit field functions
	double PercentCompleted(void) const;
	void SetBit(size_t nBit);
	void ClearBit(size_t nBit);

// variables
private:
	IPPort					m_clientIP;

	bool					m_bGarbage;			// Set true when the connection has been killed
	bool					m_bDestroying;

	// negotiating protocols
	bool					m_bNeedHandshake;
	bool					m_bGettingHandshake;
	bool					m_bFinishingHandshake;
	bool					m_bNeedInitialSearchResult;

	// communication protocol
	bool					m_bGettingHeader;

	ByteBufferReader		m_receivedBufferReader;

	set<unsigned int>		m_sAvailablePieces;

	char					m_aLocalID[BTCLIENT_ID_SIZE];
	char					m_aRemoteID[BTCLIENT_ID_SIZE];

	bool					m_bNeverUnchokedPeer;
	bool					m_bAmChokingPeer;
	bool					m_bPeerChokingMe;
	bool					m_bAmInterestedInPeer;
	bool					m_bPeerInterestedInMe;

	bool					m_bReceivedBitField;
	unsigned char *			m_pRemoteBitField;
	bool					m_bComplete;

	unsigned char			m_aKeepAliveMessage[4];
	unsigned char			m_aChokeMessage[5];
	unsigned char			m_aUnchokeMessage[5];
	unsigned char			m_aHaveMessage[9];

	unsigned int			m_nTimerCount;
	unsigned int			m_nLastReceiveCount;

	unsigned int			m_nMaxPiecesSent;
	const unsigned int		m_nMaxFastPiecesSent;
	const unsigned int		m_nMaxSlowPiecesSent;
	vector<PieceRequest>	m_vPieceRequests;

	double					m_nSlowPercentageLimit;
	enum TorrentSpeed
	{
		FAST,
		SLOW
	}						m_eSpeed;

	const Torrent *			m_pTorrent;

	ConnectionSockets *		m_pParent;
	size_t					m_nIndex;
};
