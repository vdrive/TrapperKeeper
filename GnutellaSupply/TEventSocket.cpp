// TEventSocket.cpp

#include "stdafx.h"
#include "TEventSocket.h"

//
//
//
TEventSocket::TEventSocket()
{
}

//
//
//
TEventSocket::~TEventSocket()
{
}

//
//
//
int TEventSocket::Create(unsigned int port)
{
	return WEventSocket::Create(port);
}