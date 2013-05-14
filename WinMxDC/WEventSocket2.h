// WEventSocket2.h

#ifndef W_EVENT_SOCKET2_H
#define W_EVENT_SOCKET2_H

#include "WSocket2.h"

class WEventSocket2 : public WSocket2
{
public:
	WEventSocket2();
	~WEventSocket2();

	WSAEVENT ReturnEventHandle();
	WSANETWORKEVENTS ReturnNetworkEvents();

	int Create(unsigned int port=0);
	int Connect(char *host,unsigned int port);
	virtual int Close();

	virtual void OnConnect(int error_code);
	virtual void OnReceive(int error_code);
	virtual void OnAccept(int error_code);
	virtual void OnClose(int error_code);

protected:
	WSAEVENT m_hEvent;

	int EventSelect(long events);
};

#endif // W_EVENT_SOCKET_H