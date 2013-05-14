// TAsyncSocket.h
#pragma once

#include "WAsyncSocket.h"

class TAsyncSocket : public WAsyncSocket
{
public:
	TAsyncSocket();
	~TAsyncSocket();

	virtual int Create(unsigned int port=0);
};
