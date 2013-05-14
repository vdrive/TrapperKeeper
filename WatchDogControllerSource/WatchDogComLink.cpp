#include "StdAfx.h"
#include "watchdogcomlink.h"
#include "../SamplePlugIn/Dll.h"

WatchDogComLink::WatchDogComLink(void)
{
}

WatchDogComLink::~WatchDogComLink(void)
{
}

void WatchDogComLink::DataReceived(char *source_ip, void *data, UINT data_length)
{
	//received remote comm data
	p_dll->DataReceived(source_ip, data, data_length);
}