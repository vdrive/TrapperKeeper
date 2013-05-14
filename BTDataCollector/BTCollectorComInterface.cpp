#include "StdAfx.h"
#include ".\btcollectorcominterface.h"
#include "BTDataDll.h"

BTCollectorComInterface::BTCollectorComInterface(void)
{
}

BTCollectorComInterface::~BTCollectorComInterface(void)
{
}


void BTCollectorComInterface::InitParent(BTDataDll * parent)
{
	p_parent = parent;
}

void BTCollectorComInterface::DataReceived(char *source_ip, void *data, UINT data_length)
{
	p_parent->DataReceived(source_ip,data,data_length);
}
