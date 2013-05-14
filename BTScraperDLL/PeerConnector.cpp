#include "afxsock.h"
#include "peerconnector.h"

IMPLEMENT_DYNAMIC(PeerConnector, CAsyncSocket)


				
PeerConnector::PeerConnector(void)
{
	m_torrent_file = NULL;
}

PeerConnector::PeerConnector(TorrentFile * tf)
{
	m_torrent_file = tf;
}


PeerConnector::~PeerConnector(void)
{
}

int PeerConnector::FindSeeds() 
{
	return FindSeeds(m_torrent_file->GetPeers());
}

int PeerConnector::FindSeeds(PeerList * peers) 
{
	if(peers != NULL) {
		Connect(peers);
		PeerList * next = peers->GetNext();
		if(next != NULL) {
			PeerConnector pc(m_torrent_file);
			return (peers->IsSeed()) + pc.FindSeeds(next);
		}
		else return 0;

	}
	else return 0;
}



void PeerConnector::OnSend(int errorCode) 
{
	if(errorCode!= 0) {
		// some error handling
	}
	CAsyncSocket::OnSend(errorCode);
}

void PeerConnector::OnAccept(int errorCode) 
{
	if(errorCode!= 0) {
		// some error handling
	}
	CAsyncSocket::OnAccept(errorCode);
}

void PeerConnector::OnConnect(int errorCode) 
{
	if(errorCode!= 0) {
		// some error handling
	}
	else if(Handshake(m_peer)) {
		m_peer->IsSeed(IsSeed(m_peer));
	}
	CAsyncSocket::OnConnect(errorCode);
	Close();
}

void PeerConnector::OnClose(int errorCode) 
{
	if(errorCode!= 0) {
		// some error handling
	}
	CAsyncSocket::OnClose(errorCode);
}

	
void PeerConnector::Connect(PeerList * peer) 
{
	m_peer = peer;
	string ip = peer->GetIP();
	int port = atoi(peer->GetPort().c_str());
	cout << "ip: " << ip << ":" << port << endl;
	Create();
	if(!CAsyncSocket::Connect(ip.c_str(), port)) {
		int err = GetLastError();	
		if(err == WSAEWOULDBLOCK) {
			cout << "block\n";	
		}
		else {
			cout << err << endl;
		}
	}
	else if(Handshake(peer)) {
		peer->IsSeed(IsSeed(peer));
		Close();
	}
}

bool PeerConnector::Handshake(PeerList * peer) 
{
	TCHAR * message = GetHandshakeMessage(peer);
	TCHAR * sz = new TCHAR[1024];

	Send(message, 68);
	int nRead = Receive(sz, 68);
	int size = nRead;
	while(nRead > 0) {
		nRead = Receive(sz, 1024);
		size += nRead;
	}
	if(size < 0) {
		delete [] message;
		message = NULL;
		delete [] sz;
		sz = NULL;
		return false;
	}
	int cur_pos = 0;
	int len = sz[cur_pos];
	cur_pos++;
	string protocol = &sz[cur_pos];
	cur_pos += len;
	cur_pos += 8;
	TCHAR info_hash[20];
	memcpy(info_hash, &sz[cur_pos], 20);
	cur_pos += 20;
	TCHAR peer_id[20];
	memcpy(peer_id, &sz[cur_pos], 20);
	cur_pos += 20;	

	delete [] message;
	message = NULL;
	delete [] sz;
	sz = NULL;
	return true;
}

bool PeerConnector::IsSeed(PeerList *peer) {
	TCHAR * sz = new TCHAR[1024];
	int nRead = Receive(sz, 1024);
	bool isBitField = false;
	int start = -1;
	for(int i = 0; i < nRead; i++) { // go past the keep alive messages
		if(!isBitField && sz[i] == 0) continue;
		else {
			start = i;
			break;
		}
	}
	if(start > 0) {
		int cur_pos = start;
		int size = sz[cur_pos];
		cur_pos++;
		int type = sz[cur_pos];
		if(type == 5) {
			// check each bit... 
		}
	}
	else return false;
	return true;
}

char * PeerConnector::GetHandshakeMessage(PeerList * peer) {
	TCHAR * message = new TCHAR[68];
	unsigned short * info_hash = m_torrent_file->GetInfoHash();
	string id = peer->GetID();
	string protocol = "BitTorrent protocol";
	int len = (int)protocol.length();
	int cur_pos = 0;
	message[0] = len;
	cur_pos++;
	memcpy(&message[1], protocol.c_str(), len); // protocol
	cur_pos += len;
	for(int i = 0; i < 8 ; i++ ) { // reserve bits
		message[i+cur_pos] = 0;
	}
	cur_pos += 8;
	for(int i = 0 ; i < 20; i++) {
		message[i+cur_pos] = (TCHAR)info_hash[i];
	}
	cur_pos += 20;
	memcpy(&message[cur_pos], id.c_str(), id.length());
	cur_pos += (int)id.length();
	return message;
}