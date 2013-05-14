#pragma once
#include "teventsocket.h"
//#include "tasyncsocket.h"
#include "..\BTScraperDll\PeerList.h"
#include "..\BTScraperDll\TorrentFile.h"
//#include "TrackerConnector.h"

#define BT_CHOKE			0
#define BT_UNCHOKE			1
#define BT_INTERESTED		2
#define BT_NOT_INTERESTED	3
#define BT_HAVE				4
#define BT_BITFIELD			5
#define BT_REQUEST			6
#define BT_PIECE			7
#define BT_CANCEL			8

//class CClientConnection;
class ConnectionSockets;
class CClientSocket :
	public TEventSocket
{
public:
	struct PiecePosition
	{
		int piece;
		int position;
	};

	CClientSocket(void);
	CClientSocket(int datasize);
	CClientSocket(PeerList * peer, TorrentFile * torrent);
	~CClientSocket(void);

	void InitParent(ConnectionSockets * parent);
	void SetSharing(bool state);

	void OnConnect(int error_code);
	bool OnReceive(int error_code);
	void OnClose(int error_code);
	bool IsConnected();
	void Attach(SOCKET hsocket);

	//void SendBitfield();  No need to send the bitfield to a seed, I'm pretending I have no pieces
	void SendRequest();
	void SendSpecificRequest(int piece, int begin);

	void Handshake(PeerList * peer);

	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);
	void SocketDataReceived(char *data,unsigned int len);
	
	int ReceiveSocketData(unsigned int len);
	int ReceiveSomeSocketData(unsigned int len);

	int Close();

	void SendKeepAlive();
	void SendChoke();
	void SendUnChoke();
	void SendInterested();
	void BitfieldReceived(char * data, int len);
	void SendBitfield();
	void SendHave();
	int RequestSocketInfo();

	void SetTempConnect();
	void SetDisconnected();
	bool InitTorrent(TorrentFile torrent);
	int SwapBytes(int value);
	void SetIp(char * ip);
	void SetAsSeed();
	void SetIncrement(int increment);

	void SendRandomData(int index, int begin, int length);
	void SendValidData(int index, int begin, int length);
	void RequestReceived(char * data);
	string GetIpString();

private:

	int m_index;
	ConnectionSockets *p_parent;
	int m_data_size;
	PeerList m_peer;
	//TorrentFile m_torrent;
	char m_torrent_name[256];
	int m_torrent_num_pieces;
	int m_torrent_piecelength;


	bool * p_my_bitfield;	
	bool * p_remote_bitfield;
	int m_cur_piece;
	int m_cur_pos;
	int m_offer_piece;

	int m_sharing_increment;

	char m_ip[16];

	vector<PiecePosition> v_piece_pos;


	bool m_connected;
	bool m_socket_header_received;
	bool m_handshake_received;
	bool m_is_seed;

	bool m_sharing;

};
