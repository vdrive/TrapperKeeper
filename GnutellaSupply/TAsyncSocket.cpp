// TAsyncSocket.cpp

#include "stdafx.h"
#include "TAsyncSocket.h"

//
//
//
TAsyncSocket::TAsyncSocket()
{
}

//
//
//
TAsyncSocket::~TAsyncSocket()
{
}

//
//
//
int TAsyncSocket::Create(unsigned int port)
{
	return WAsyncSocket::Create(port);
}