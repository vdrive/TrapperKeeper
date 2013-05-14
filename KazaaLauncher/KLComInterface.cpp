#include "StdAfx.h"
#include "klcominterface.h"
#include "../SamplePlugIn/Dll.h"

KLComInterface::KLComInterface(void)
{
}

//
//
//
KLComInterface::~KLComInterface(void)
{
}

//
//
//
void KLComInterface::DataReceived(char *source_ip, void *data, UINT data_length)
{
	//received remote comm data
	p_dll->DataReceived(source_ip, data, data_length);
}