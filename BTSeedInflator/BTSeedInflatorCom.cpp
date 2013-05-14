#include "StdAfx.h"
#include ".\BTSeedInflatorCom.h"
#include "btseedinflatordll.h"

BTSeedInflatorCom::BTSeedInflatorCom(void)
{
}

BTSeedInflatorCom::~BTSeedInflatorCom(void)
{
}

void BTSeedInflatorCom::InitParent(BTSeedInflatorDll * parent)
{
	p_parent = parent;
}

void BTSeedInflatorCom::DataReceived(char *source_ip, void *data, UINT data_length)
{
	p_parent->DataReceived(source_ip,data,data_length);
}
