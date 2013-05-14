#include "StdAfx.h"
#include "autoupdatersyncher.h"
#include "AutoUpdaterDll.h"

AutoUpdaterSyncher::AutoUpdaterSyncher()
{
}

AutoUpdaterSyncher::~AutoUpdaterSyncher(void)
{
}

void AutoUpdaterSyncher::InitParent(AutoUpdaterDll *parent)
{
	p_parent = parent;
}

void AutoUpdaterSyncher::MapFinishedChanging(const char* source_ip)
{
	p_parent->GetGoodDllsFromSource((char *)source_ip);
}