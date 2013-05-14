// WEventSocket.h

#ifndef WEVENT_SOCKET_H
#define WEVENT_SOCKET_H

#include "WSocket.h"

class WEventSocket : virtual public WSocket
{
public:
	WEventSocket();
	~WEventSocket();

	WSAEVENT ReturnEventHandle();
	WSANETWORKEVENTS ReturnNetworkEvents();

	virtual int Create(unsigned short int port,unsigned int ip=INADDR_ANY);
	virtual int Close();

	virtual void Attach(SOCKET hSocket);
	virtual SOCKET Detach();

	int Connect(char *host,unsigned short int port);

	virtual void OnConnect(int error_code);
	virtual bool OnReceive(int error_code);
	virtual void OnSend(int error_code);
	virtual void OnAccept(int error_code);
	virtual void OnClose(int error_code);

protected:
	int EventSelect(long events);

	// Protected Data Members
	WSAEVENT m_hEvent;
};

#endif // WEVENT_SOCKET_H
