#include "StdAfx.h"
#include "btclientcominterface.h"
#include "BTClientDll.h"

BTClientComInterface::BTClientComInterface(void)
{
}

BTClientComInterface::~BTClientComInterface(void)
{
}



void BTClientComInterface::InitParent(BTClientDll * parent)
{
	p_parent = parent;
}

void BTClientComInterface::DataReceived(char *source_ip, void *data, UINT data_length)
{
	p_parent->DataReceived(source_ip,data,data_length);
}