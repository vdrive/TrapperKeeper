#include "parentcominterface.h"
#include "SoulSeekClientDll.h"
#include "PacketIO.h"

ParentComInterface::ParentComInterface(void)
{
}

ParentComInterface::~ParentComInterface(void)
{
}

void ParentComInterface::InitParent(SoulSeekClientDll *parent)
{
	p_parent=parent;
}

void ParentComInterface::DataReceived(char *source_ip, void *data, UINT data_length)
{
	//char* packet = new char[data_length-4];
	//memcpy(packet,(char *)data+4,(size_t)data_length-4);

	unsigned char* packet = new unsigned char[(*(size_t *)data)+1];
	memcpy(packet,(unsigned char *)data+4,*(size_t *)data);
	packet[*(size_t *)data]= '\0';
	p_parent->m_pa.AssemblePacket((unsigned char *) packet);
	TRACE("Recieved packet from Server\n");
}