#pragma once
#include "TEventSocket.h"
#include "ByteBufferReader.h"
#include "IPPort.h"
#include "Peer.h"

class ByteBuffer;
class ConnectionSockets;
class ConnectionModule;
class TorrentIndexResult;
class BTClient
	: public TEventSocket
{
// functions
public:
	BTClient(void);
	~BTClient(void);
	void Destroy(void);
	int Close(void);

	void OnClose(int nErrorcode);

	void Attach(SOCKET hSocket);

	int SendSocketData(ByteBuffer &rBuf);
	int SendSocketData(ByteBuffer *pBuf, bool bDelete = true);
	int SendSocketData(unsigned char *pBuf, unsigned int nLen);

	void Init(void);
	inline void InitParent(ConnectionSockets* pParent) { m_pParent = pParent; };
	inline void InitIndex(size_t nIndex) { m_nIndex = nIndex; }

	bool OnReceive(int nErrorCode);
	void SomeSocketDataReceived(char *data, unsigned int data_len, unsigned int new_len, unsigned int max_len);

	int DataHandler(void);
	void TorrentIndexRes(TorrentIndexResult *pTorrentIndexResult);

	// Returns the client ip/port holder
	inline IPPort GetClientIP(void) { return m_clientIP; };

// variables
private:
	IPPort					m_clientIP;

	bool					m_bGarbage;							// Set true when the connection has been killed
	bool					m_bDestroying;

	ByteBufferReader		m_receivedBufferReader;
	ConnectionSockets *		m_pParent;

	size_t					m_nIndex;

	Peer					m_mdPeer;

	bool					m_bSupportsCompact;
};
