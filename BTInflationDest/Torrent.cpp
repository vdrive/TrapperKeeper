#include "StdAfx.h"
#include ".\torrent.h"

Torrent::Torrent(void)
{

	m_last_attempt = NULL;
	m_has_connected = false;

}

Torrent::~Torrent(void)
{


}

void Torrent::InitTorrent(string name, string announce_url, string info_hash, int num_seeds, int num_peers, int port)
{
	m_announce_url = announce_url;
	m_info_hash = info_hash;
	int tempport = port;
	m_torrent_name = name;

	int infosize = sizeof(PeerInfo);

	for (int i = 0; i < num_seeds; i++)
	{
		PeerInfo tempinfo;
		tempinfo.m_peer_id = CreatePeerId();
		tempinfo.m_has_connected = false;
		tempinfo.m_is_seed = true;
		tempinfo.m_port = tempport;
		tempinfo.m_key = CreateKeyID();
		tempport++;
		v_peer_ids.push_back(tempinfo);
		infosize = sizeof(tempinfo);
	}

	for (int j = 0; j < num_peers; j++)
	{
		PeerInfo tempinfo;
		tempinfo.m_peer_id = CreatePeerId();
		tempinfo.m_is_seed = false;
		tempinfo.m_has_connected = false;
		tempinfo.m_port = tempport;
		tempinfo.m_key = CreateKeyID();
		tempport++;
		v_peer_ids.push_back(tempinfo);
		infosize = sizeof(tempinfo);
	}



	

}

void Torrent::SetHasConnected()
{
	m_has_connected = true;
}

bool Torrent::HasConnected()
{

	return m_has_connected;

}

string Torrent::CreatePeerId() 
{
	char * id = new char [21];
	strcpy(id, "-AZ2202-");
	for(int i = 8; i < 20; i++) {
		int pos = (int) ( rand() % 62);
		id[i] = chars[pos];
	}
	id[20] = 0;

	string retstring = id;
	delete [] id;
	return retstring;
}


void Torrent::UpdatePeerConnect(string peer_id)
{
	for (int i = 0; i < (int)v_peer_ids.size(); i ++)
	{
		if (strcmp(peer_id.c_str(),v_peer_ids[i].m_peer_id.c_str())== 0)
		{
			v_peer_ids[i].m_has_connected = true;
			v_peer_ids[i].m_last_connect = CTime::GetCurrentTime();
			v_peer_ids[i].m_last_attempt = v_peer_ids[i].m_last_connect;
		}

	}

}

void Torrent::UpdatePeerFailure(string peer_id)
{
	for (int i = 0; i < (int)v_peer_ids.size(); i ++)
	{
		if (strcmp(peer_id.c_str(),v_peer_ids[i].m_peer_id.c_str()) == 0)
		{
			v_peer_ids[i].m_last_failure = CTime::GetCurrentTime();
			v_peer_ids[i].m_last_attempt = v_peer_ids[i].m_last_failure;
		}

	}

}

bool Torrent::operator==(const Torrent & tor)
{

	//if (m_info_hash.equals(tor.m_info_hash) == false)
	if (strcmp(m_info_hash.c_str(),tor.m_info_hash.c_str()) != 0)
		return false;
	//if (m_announce_url.equals(tor.m_announce_url) == false)
	if (strcmp(m_announce_url.c_str(),tor.m_announce_url.c_str()) != 0)
		return false;

	return true;

}

bool Torrent::IsReady()
{
		
	char * ptr;
	ptr = strstr(m_announce_url.c_str(),"prq.to");
	if (ptr != NULL)
	{
		return true;
	}
	
	CTimeSpan diff = CTime::GetCurrentTime()-m_last_attempt;
	int change = (int)diff.GetTotalMinutes();
	if (diff.GetTotalMinutes() >= 4)
	{
		m_last_attempt = CTime::GetCurrentTime();
		return true;
	}
	return false;

}

string Torrent::GetAnnounceUrl()
{

	return m_announce_url;

}

string Torrent::GetName()
{

	return m_torrent_name;

}

string Torrent::GetInfoHash()
{

	return m_info_hash;

}

int Torrent::GetNumConnections()
{
	
	return (int)v_peer_ids.size();

}

void Torrent::GetPeerInfo(vector<PeerInfo> *info)
{

	*info = v_peer_ids;

}

void Torrent::SetAttemptTime()
{

		m_last_attempt = CTime::GetCurrentTime();

}

string Torrent::CreateKeyID() 
{
	string retstring;
	int key_len = 8;
	char * id = new char [key_len+1];
	strcpy(id, "");
	for(int i = 0; i < key_len; i++) {
		int pos = (int) ( rand() % 62);
		id[i] = chars[pos];
	}
	id[key_len] = 0;
	retstring = id;
	delete [] id;
	return retstring;
}