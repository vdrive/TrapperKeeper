#include "StdAfx.h"
#include "metaspoofercom.h"
#include "SpoofingSystem.h"

MetaSpooferCom::MetaSpooferCom(void)
{
}

MetaSpooferCom::~MetaSpooferCom(void)
{
}

void MetaSpooferCom::DataReceived(char *source_ip, void *data, UINT data_length)
{
	MetaSpooferReference ref;
	ref.System()->ComDataReceived(source_ip,(byte*)data,data_length);
}