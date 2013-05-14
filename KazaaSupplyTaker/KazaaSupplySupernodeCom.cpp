#include "StdAfx.h"
#include "kazaasupplysupernodecom.h"
#include "KazaaSupplyTakerDll.h"
#include "../SupernodeDistributer/KazaaControllerHeader.h"

KazaaSupplySupernodeCom::KazaaSupplySupernodeCom(void)
{
}

KazaaSupplySupernodeCom::~KazaaSupplySupernodeCom(void)
{
}

void KazaaSupplySupernodeCom::InitParent(KazaaSupplyTakerDll *dll)
{
	p_dll = dll;
}

//
//	The only thing we should recieve from this guy is a list of all known supernodes, once per day.
//
void KazaaSupplySupernodeCom::DataReceived(char *source_ip, void *data, UINT data_length)
{
	//received com data
	TRACE("KazaaSupplyTaker: Received Supernode Com Data\n");

	// What kind of message was this?
	char* ptr = (char*)data;
	KazaaControllerHeader* header = (KazaaControllerHeader*)ptr;
	ptr+=sizeof(KazaaControllerHeader);

	switch(header->op)
	{
		case KazaaControllerHeader::Source_Supernodes_Reply:
			{
				p_dll->m_dlg.Log(0, NULL, "Received new supernode list from com.");
//				p_dll->m_dlg.manager.SetSuperNodeList(ptr, data_length);
				break;
			}
	}
}