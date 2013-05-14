// TEventSocket.h
#pragma once

#include "WEventSocket.h"

class TEventSocket : public WEventSocket
{
public:
	TEventSocket();
	~TEventSocket();

	virtual int Create(unsigned int port=0);
};