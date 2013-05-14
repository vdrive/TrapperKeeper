#include "StdAfx.h"
#include "metacom.h"
#include "MetaSystem.h"

MetaCom::MetaCom(void)
{
}

MetaCom::~MetaCom(void)
{
}

void MetaCom::DataReceived(char *source_ip, void *data, UINT data_length)
{
	MetaSystemReference system;
	system.System()->ComDataReceived(this,source_ip,(byte*)data,data_length);
}