// UEventSocket.cpp

#include "stdafx.h"
#include "UEventSocket.h"

//
//
//
UEventSocket::UEventSocket()
{
}

//
//
//
UEventSocket::~UEventSocket()
{
}

//
//
//
int UEventSocket::Create(unsigned short int port,unsigned int ip)
{
	return WEventSocket::Create(port,ip);
}

//
//
//
int UEventSocket::Close()
{
	return WEventSocket::Close();
}

//
// Virtual Functions
//
void UEventSocket::OnReceive(int error_code){}
