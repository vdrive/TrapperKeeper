#include "stdafx.h"
#include "inflationsocketmanager.h"
#include "BTInflatorDll.h"


InflationSocketManager::InflationSocketManager(void)
{
}

InflationSocketManager::~InflationSocketManager(void)
{
}

void InflationSocketManager::InitParent(BTInflatorDll * parent)
{
	
	p_parent = parent;

}


void InflationSocketManager::MakeTorrentConnections(Torrent * the_torrent)
{

	int num_sockets = the_torrent->GetNumConnections();

	vector<PeerInfo> info;
	the_torrent->GetPeerInfo(&info);
	/*
	char host[512];
	strcpy(host,the_torrent->GetAnnounceUrl().c_str());
	char *ptr, *ptrbegin;
	unsigned short port;
	//skip the http
	ptr = strstr(host,"http://");
	if (ptr != NULL)
	{
		ptr +=7;
		ptrbegin = ptr;
		ptr = strchr(ptr,'/');
	}
	else // there is no http so begin at the beginning
	{
		ptrbegin = ptr;
		ptr = strchr(host,'/');
	}

	*ptr = '\0';
	strcpy(host,ptrbegin);
	port = 80;
	*/

	char init[512];
	int urlsize = (int)strlen(the_torrent->GetAnnounceUrl().c_str());
	strcpy(init,the_torrent->GetAnnounceUrl().c_str());
	char * host = new char[urlsize+1];
	char * url = new char[urlsize+1];
	int * port = new int;

	ParseHttpString(init,host,url,port);
	bool hasconnected = the_torrent->HasConnected();

	for(int i = 0; i < (int)info.size(); i++)
	{
		
		InflationSocket * socket = new InflationSocket;
		socket->InitParent(this);
		socket->InitConnection(the_torrent->GetAnnounceUrl(),the_torrent->GetInfoHash(),info[i].m_peer_id,info[i].m_port,info[i].m_is_seed,info[i].m_key);
		if(info[i].m_has_connected == false)
		{
			socket->SetFirstConnect();
		}
	/*
		char traceme[256];
		sprintf(traceme,"connecting to %s \r\n",info[i].m_peer_id.c_str());
		TRACE0(traceme);
		*/
		socket->Create();
		socket->Connect(host,*port);
		v_sockets.push_back(socket);
	}

	delete[] url;
	delete[] host;
	delete[] port;
}

void InflationSocketManager::TorrentFailure(string info_hash,string announce_url,string peer_id,InflationSocket * socketptr)
{

	p_parent->TorrentFailure(info_hash,announce_url,peer_id);
	int size = (int)v_sockets.size();
//	InflationSocket *socket;
	for (int i = 0; i < (int)v_sockets.size(); i ++)
	{
//		socket = v_sockets[i];
		if(v_sockets[i] == socketptr)
		{
			socketptr->Close();
			v_sockets.erase(v_sockets.begin() +i);
			break;

		}

	}

	//TRACE0("deleting failed torrent\r\n");
	delete socketptr;

	size = (int)v_sockets.size();
	//TRACE0("deleted failed torrent\r\n");
}

void InflationSocketManager::TorrentSuccess(string info_hash,string announce_url,string peer_id,InflationSocket * socketptr)
{

	p_parent->TorrentSuccess(info_hash,announce_url,peer_id);
	for (int i = 0; i < (int)v_sockets.size(); i ++)
	{
		
		if(v_sockets[i] == socketptr)
		{
			socketptr->Close();
			v_sockets.erase(v_sockets.begin() +i);
			break;
		}

	}

	//TRACE0("deleting successful torrent\r\n");
	delete socketptr;
	//TRACE0("deleted successful torrent\r\n");
}



void InflationSocketManager::TorrentFailure(InflationSocket * socketptr)
{

	p_parent->TorrentFailure(socketptr->m_info_hash,socketptr->m_announce_url,socketptr->m_peer_id);
	InflationSocket *socket;
	for (int i = 0; i < (int)v_sockets.size(); i ++)
	{
		socket = v_sockets[i];
		if(v_sockets[i] == socketptr)
		{
			socketptr->Close();
			/*
			char traceme[256];
			sprintf(traceme,"found failed torrent %s \r\n",socket->m_peer_id.c_str());
			TRACE0(traceme);
			*/
			v_sockets.erase(v_sockets.begin() +i);
			break;

		}

	}
	char traceme[256];
	int size = (int)v_sockets.size();
	sprintf(traceme,"deleting failed torrent %s vector size = %i\r\n",socketptr->m_peer_id.c_str(), size);
	TRACE0(traceme);
	delete socketptr;


	//TRACE0("deleted failed torrent\r\n");
}

void InflationSocketManager::TorrentSuccess(InflationSocket * socketptr)
{

	p_parent->TorrentSuccess(socketptr->m_info_hash,socketptr->m_announce_url,socketptr->m_peer_id);
	InflationSocket *socket;
	for (int i = 0; i < (int)v_sockets.size(); i ++)
	{
		
		socket = v_sockets[i];
		if(v_sockets[i] == socketptr)
		{
			socketptr->Close();
			/*
			char traceme[256];
			sprintf(traceme,"found successful torrent %s \r\n",socket->m_peer_id.c_str());
			TRACE0(traceme);
			*/
			v_sockets.erase(v_sockets.begin() +i);
			break;
		}

	}

	int size = (int)v_sockets.size(); 
	char traceme[256];
	sprintf(traceme,"deleting failed torrent %s vector size = %i\r\n",socketptr->m_peer_id.c_str(), size);
	TRACE0(traceme);
	delete socketptr;
	//TRACE0("deleted successful torrent\r\n");
}



void InflationSocketManager::ParseHttpString(char * input, char * host, char * url, int * port)
{

	int tempport;
	char *ptr, *ptrbegin, *portptr;
	//skip the http
	ptr = strstr(input,"http://");
	if (ptr != NULL)
	{
		ptr +=7;
	}
	else
	{
		ptr = strstr(input,"udp://");
		if (ptr != NULL)
		{
			ptr +=6;
		}
	}


	if (ptr != NULL)
	{
		ptrbegin = ptr;
	}
	else
	{
		ptrbegin = input;
	}

	ptr = strchr(ptrbegin,':');
	if (ptr != NULL)
	{
		portptr = strchr(ptr,'/');
		*portptr = '\0';
		tempport = atoi(ptr+1);
		*portptr = '/';
		// Get the host as well to be used later
		*ptr = '\0';  // Break String
		strcpy(host,ptrbegin);
		*ptr = ':';
		ptr = portptr;
	}
	else
	{
		ptr = strchr(ptrbegin,'/');

		// if there is no / the url was invalid so give up
		if (ptr == NULL)
		{
			return;
		}
		*ptr = '\0';  // Break String
		strcpy(host,ptrbegin);
		*ptr = '/';
		tempport = 80;
	}

	strcpy(url,ptr);
	*port = tempport;

}

void InflationSocketManager::ClearStaleSockets()
{

	int size = (int)v_sockets.size();
	InflationSocket * tempsocket;
	for (int i = 0; i < (int)v_sockets.size();i++)
	{
		if((v_sockets[i]->IsSocket() == false) ||(v_sockets[i]->IsTimedout()))
		{
			
			char traceme[256];
			tempsocket = v_sockets[i];
			v_sockets.erase(v_sockets.begin() +i);
			sprintf(traceme,"Cleaning torrent %s \r\n",v_sockets[i]->m_peer_id.c_str());
			TRACE0(traceme);
			if (tempsocket!=NULL)
			{
				tempsocket->Close();
				delete tempsocket;
			}
			i--;
		}
		/*
		else 
		{
			i++;
		}
*/
	}

	TRACE0("Cleanup finished \r\n");

}

