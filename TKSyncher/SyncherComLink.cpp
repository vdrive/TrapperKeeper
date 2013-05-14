#include "StdAfx.h"
#include "synchercomlink.h"

SyncherComLink::SyncherComLink(void)
{
	p_service=NULL;
}

SyncherComLink::~SyncherComLink(void)
{
}

void SyncherComLink::DataReceived(char *source_ip, void *data, UINT data_length){
	p_service->ReceivedComData(source_ip,(byte*)data,data_length);
}
void SyncherComLink::SetService(SyncherService* service)
{
	p_service=service;
}
