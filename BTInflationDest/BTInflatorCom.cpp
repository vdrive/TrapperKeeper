#include "StdAfx.h"
#include ".\BTInflatorCom.h"
#include "btinflatordll.h"

BTInflatorCom::BTInflatorCom(void)
{
}

BTInflatorCom::~BTInflatorCom(void)
{
}

void BTInflatorCom::InitParent(BTInflatorDll * parent)
{
	p_parent = parent;
}

void BTInflatorCom::DataReceived(char *source_ip, void *data, UINT data_length)
{
	p_parent->DataReceived(source_ip,data,data_length);
}
