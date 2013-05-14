#include "StdAfx.h"
#include "peercachesocket.h"
#include "ConnectionManager.h"

PeerCacheSocket::PeerCacheSocket(void)
{
	m_getting_hosts=false;
}

//
//
//
PeerCacheSocket::~PeerCacheSocket(void)
{
}

//
//
//
void PeerCacheSocket::OnConnect(int error_code)
{
	if(error_code!=0)
	{
		Close();
		m_getting_hosts=false;
	}
}

//
//
//
void PeerCacheSocket::OnClose(int error_code)
{
	Close();
	m_getting_hosts=false;
}

//
//
//
void PeerCacheSocket::OnReceive(int error_code)
{
	ReceiveSocketData(149);
}

//
//
//
void PeerCacheSocket::SocketDataReceived(char *data,unsigned int len)
{
	BYTE* ptr = (BYTE*)data;
	 
	if(len!=149)
	{
		Close();
		return;
	}
	char Buff[512] = {'\0'};
	BYTE *pNodeBuff = (BYTE *)Buff + sizeof(Buff) / 2;
	PARENTNODEINFO FreeNode;
	PARENTNODEINFO NodeInfo[10];
	int i = 0;
	WORD wHostIndex = 0;
	WORD wPortIndex = 0;

	if(*ptr == 0x38)
	{
		ptr++;
		if(GetCryptKeyID(ptr) != 0x54)
		{
			Close();
			return;
		}
		else
			ptr+=16;

		// Decrypt Node Information
		DecryptFrontCode(ptr, pNodeBuff);
		memcpy(&NodeInfo, pNodeBuff, 120);

		/*
		// Sort by Free Secondary Num
		for(j = 0 ; j <= 9 ; j++)
		{
			iFree = j;
			for(i = j + 1 ; i < 10 ; i++)
			{
				if(NodeInfo[i].bFreeSec > NodeInfo[iFree].bFreeSec || (NodeInfo[i].bFreeSec == NodeInfo[iFree].bFreeSec && NodeInfo[i].bFreePri < NodeInfo[iFree].bFreePri))
					iFree = i;
			}
			FreeNode = NodeInfo[j];
			NodeInfo[j] = NodeInfo[iFree];
			NodeInfo[iFree] = FreeNode;
		}
		*/
	}
	Close();
	vector<SupernodeHost> hosts;
	for(i = 0 ; i <= 9 ; i++)
	{
		SupernodeHost host;
		host.m_ip = NodeInfo[i].dwNodeIP;
		host.m_port = NodeInfo[i].wTCPPort;
		hosts.push_back(host);
	}

	p_manager->ReportHosts(hosts);
	m_getting_hosts=false;
}

//
//
//
void PeerCacheSocket::InitParent(ConnectionManager* parent)
{
	p_manager = parent;
}

//
//
//
int PeerCacheSocket::ConnectToPeerCache(const char* IP, unsigned short port)
{
	if(!m_getting_hosts)
	{
		m_getting_hosts=true;
		Close();
		Create();
		return TAsyncSocket::Connect(IP,port);
	}
}