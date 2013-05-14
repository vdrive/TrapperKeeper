#include "StdAfx.h"
#include "swarmersourcecominterface.h"
#include "../SamplePlugIn/Dll.h"
#include "SwarmerSourceDll.h"

SwarmerSourceComInterface::SwarmerSourceComInterface(void)
{
}

SwarmerSourceComInterface::~SwarmerSourceComInterface(void)
{
}

void SwarmerSourceComInterface::InitParent(SwarmerSourceDll * parent)
{
	p_parent = parent;
}

void SwarmerSourceComInterface::DataReceived(char *source_ip, void *data, UINT data_length)
{
	p_parent->DataReceived(source_ip,data,data_length);
}