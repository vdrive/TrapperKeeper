#include "StdAfx.h"
#include ".\btipgatherercom.h"
#include "btipgathererdll.h"

BTIPGathererCom::BTIPGathererCom(void)
{
}

BTIPGathererCom::~BTIPGathererCom(void)
{
}

void BTIPGathererCom::InitParent(BTIPGathererDll * parent)
{
	p_parent = parent;
}

void BTIPGathererCom::DataReceived(char *source_ip, void *data, UINT data_length)
{
	p_parent->DataReceived(source_ip,data,data_length);
}
