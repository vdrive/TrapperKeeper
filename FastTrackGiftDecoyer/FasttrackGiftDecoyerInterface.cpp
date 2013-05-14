#include "StdAfx.h"
#include "FasttrackGiftDecoyerInterface.h"
#include "DllInterface.h"
#include "../SupernodeCollector/KazaaControllerHeader.h"
#include "../DllLoader/AppID.h"

FasttrackGiftDecoyerInterface::FasttrackGiftDecoyerInterface(void)
{
}

//
//
//
FasttrackGiftDecoyerInterface::~FasttrackGiftDecoyerInterface(void)
{
}

//
//
//
bool FasttrackGiftDecoyerInterface::InterfaceReceivedData(AppID from_app_id,void* input_data,void* output_data)
{
	return false; //pass this data to the main app
}

//
//
//
bool FasttrackGiftDecoyerInterface::StopSharing()
{
	byte buf[sizeof(KazaaControllerHeader)];
	KazaaControllerHeader* header = (KazaaControllerHeader*)&buf;
	header->op = KazaaControllerHeader::Source_Kill_All_Kazaa;
	header->size = 0;

	AppID app_id;
	app_id.m_app_id = 66;
	app_id.m_version = 0x00000001;

	return DllInterface::SendData(app_id,buf);	
}

//
//
//
bool FasttrackGiftDecoyerInterface::StartSharing()
{
	byte buf[sizeof(KazaaControllerHeader)];
	KazaaControllerHeader* header = (KazaaControllerHeader*)&buf;
	header->op = KazaaControllerHeader::Source_Launch_All_Kazaa;
	header->size = 0;

	AppID app_id;
	app_id.m_app_id = 66;
	app_id.m_version = 0x00000001;

	return DllInterface::SendData(app_id,buf);	
}
