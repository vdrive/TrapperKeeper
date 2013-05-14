#pragma once
#include "tasyncsocket.h"
#include "MxSock.h"
#include "Manager.h"
#include "WinMXSocket.h"

class ConnectionManager;
class PeerCacheSocket :
	public TAsyncSocket
{
public:
	PeerCacheSocket(void);
	~PeerCacheSocket(void);
	void InitParent(ConnectionManager* parent);
	
	//overrided functions
	void OnConnect(int error_code);
	void OnReceive(int error_code);
	void OnClose(int error_code);
	void SocketDataReceived(char *data,unsigned int len);

	void GetSomeHosts();
	int ConnectToPeerCache(const char* IP, unsigned short port);

private:
	ConnectionManager* p_manager;
	bool m_getting_hosts;
};
