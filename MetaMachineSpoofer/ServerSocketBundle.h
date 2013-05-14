#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Vector.h"
#include "ServerSocket.h"

class ServerSocketBundle :
	public Object
{
private:
	Vector mv_server_sockets;
	unsigned short m_port;
public:
	ServerSocketBundle(void);
	~ServerSocketBundle(void);
	void Init(unsigned short port,SocketEventListener* p_listener,vector <string>&v_system_ips);
	void Close(void);
	unsigned short GetPort(){return m_port;}
};
