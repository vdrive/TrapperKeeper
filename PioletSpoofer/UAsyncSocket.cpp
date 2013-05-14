// UAsyncSocket.cpp

#include "stdafx.h"
#include "UAsyncSocket.h"

//
//
//
UAsyncSocket::UAsyncSocket()
{
}

//
//
//
UAsyncSocket::~UAsyncSocket()
{
}

//
//
//
int UAsyncSocket::Create(unsigned short int port,unsigned int ip)
{
	return WAsyncSocket::Create(port,ip);
}

//
//
//
int UAsyncSocket::Close()
{
	return WAsyncSocket::Close();
}