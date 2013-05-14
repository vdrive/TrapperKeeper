#include "StdAfx.h"
#include "btscominterface.h"
#include "../SamplePlugIn/Dll.h"

BTSComInterface::BTSComInterface(void)
{
}

//
//
//
BTSComInterface::~BTSComInterface(void)
{
}

//
//
//
void BTSComInterface::DataReceived(char *source_ip, void *data, UINT data_length)
{
	//received remote comm data
	p_dll->DataReceived(source_ip, data, data_length);
}