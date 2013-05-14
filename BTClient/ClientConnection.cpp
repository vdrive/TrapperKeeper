#include "StdAfx.h"
#include "clientconnection.h"

CClientConnection::CClientConnection(void)
{

	char * id = CreatePeerID();
	strcpy(m_peer_id,id);
	nextport = 6881;

}

CClientConnection::~CClientConnection(void)
{
	
	if (p_bitfield != NULL)
	{
		delete [] p_bitfield;
	}

	for (int i =0; i < (int)mv_sockets.size(); i++)
	{

			delete [] &mv_sockets[i];

	}

}

UINT CClientConnection::Run()
{
	while(!this->b_killThread){
		//do whatever forever
		CSingleLock lock(&m_lock,TRUE);
	

		Sleep(50);
	}
	return 0;
}
void CClientConnection::StartConnections(TorrentFile& the_torrent)
{
	m_torrent=the_torrent;
	PeerList * peers = m_torrent.GetPeers();
	p_bitfield = new bool[the_torrent.GetNumPieces()];

	m_listener.Startup();
	int peercount = peers->GetCount();
	
	for(int i = 0;i < peercount ;i++)
	{/*
		if (peers->IsSeed())
		{

			CClientSocket * connectsocket = new CClientSocket(&peers[i],&the_torrent);
			connectsocket->InitParent(this);
			char * connectip = new char[strlen(peers[i].GetIP().c_str())];
			strcpy(connectip,peers[i].GetIP().c_str());
			connectsocket->Connect(connectip,atoi(peers[i].GetPort().c_str()));
			mv_sockets.push_back(connectsocket);
			delete [] connectip;
			break;  // just testing one for now
		}
		*/
		peers = peers->GetNext();
	}
	this->StartThread();
}

void CClientConnection::Stop(void)
{
	this->StopThread();
}



char * CClientConnection::CreatePeerID() {
	char * chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	char * id = new char [21];
	strcpy(id, "-AZ2202-");
	srand( (unsigned)time( NULL ) );
	for(int i = 8; i < 20; i++) {
		int pos = (int) ( rand() % 62);
		id[i] = chars[pos];
	}
	id[20] = 0;
	return id;
}

unsigned char * CClientConnection::GetHandshakeMessage()
{
	unsigned char * message = new unsigned char[68];
	unsigned short * info_hash = m_torrent.GetInfoHash();
	string id = m_peer_id;
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