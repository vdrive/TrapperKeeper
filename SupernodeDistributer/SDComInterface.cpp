#include "StdAfx.h"
#include "sdcominterface.h"
#include "../SamplePlugIn/Dll.h"

SDComInterface::SDComInterface(void)
{
}

//
//
//
SDComInterface::~SDComInterface(void)
{
}

//
//
//
void SDComInterface::DataReceived(char *source_ip, void *data, UINT data_length)
{
	//received remote comm data
	p_dll->DataReceived(source_ip, data, data_length);
}