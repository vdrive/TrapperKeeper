// WEventSocket.h
#pragma once

#include "WSocket.h"

class WEventSocket : public WSocket
{
public:
	WEventSocket();
	~WEventSocket();

	WSAEVENT ReturnEventHandle();
	WSANETWORKEVENTS ReturnNetworkEvents();

	virtual int Create(unsigned int port);
	virtual int Close();

	virtual void OnConnect(int error_code)=0;
	virtual void OnReceive(int error_code)=0;
	virtual void OnAccept(int error_code);
	virtual void OnClose(int error_code);

protected:
	WSAEVENT m_hEvent;

	int EventSelect(long events);
};