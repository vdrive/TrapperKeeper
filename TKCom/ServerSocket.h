#pragma once
#include "SocketEventListener.h"
#include "..\tkcom\object.h"

#include <vector>
using namespace std;

class ServerSocket : public CAsyncSocket, public Object
{
protected:
	string m_bind_ip;
	SocketEventListener* mp_listener;
	unsigned short m_port;
	virtual void OnAccept(int nErrorCode);
public:
	
	ServerSocket();
	virtual ~ServerSocket();
	void Init(unsigned short port,SocketEventListener* p_listener);
	void SetBindIP(const char* bind_ip){m_bind_ip=bind_ip;}
	unsigned short GetPort(){return m_port;}
};


