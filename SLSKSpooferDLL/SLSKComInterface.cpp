#include "StdAfx.h"
#include "SLSKcominterface.h"
#include "../SamplePlugIn/Dll.h"

SLSKComInterface::SLSKComInterface(void)
{
}

//
//
//
SLSKComInterface::~SLSKComInterface(void)
{
}

//
//
//
void SLSKComInterface::DataReceived(char *source_ip, void *data, UINT data_length)
{
	//received remote comm data
	p_dll->DataReceived(source_ip, data, data_length);
}