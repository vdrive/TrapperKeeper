#include "StdAfx.h"
#include "Comlink.h"

ComLink::ComLink( )

{
	p_service=NULL;
}

ComLink::~ComLink(void)
{
}

//Passes DataReceived notifications to FileTransferService
void ComLink::DataReceived(char *source_ip, void *data, UINT data_length){
	if(p_service){
//		p_service->WriteToLog("ComLink::DataReceived() BEGIN received COM data.");
		p_service->ReceivedComData(source_ip,(byte*)data,data_length);
//		p_service->WriteToLog("ComLink::DataReceived() END received COM data.");
		//TRACE("Finished Calling FileTransferService::ReceivedComData().\n");
	}
}

void ComLink::SetService(FileTransferService* service)
{
	p_service=service;
}
