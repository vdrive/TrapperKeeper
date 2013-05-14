#include "StdAfx.h"
#include "autoupdatercom.h"
#include "../SamplePlugIn/dll.h"
#include "AutoUpdaterDll.h"

AutoUpdaterCom::AutoUpdaterCom(void)
{
}

AutoUpdaterCom::~AutoUpdaterCom(void)
{
}

void AutoUpdaterCom::InitParent(AutoUpdaterDll *parent)
{
	p_parent = parent;
}

//
//
void AutoUpdaterCom::DataReceived(char *source_ip, void *data, UINT data_length)
{
	//received remote comm data
	p_parent->DataReceived(source_ip, data, data_length);
}