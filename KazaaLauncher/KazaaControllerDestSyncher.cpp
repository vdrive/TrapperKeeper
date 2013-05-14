#include "StdAfx.h"
#include "kazaacontrollerdestsyncher.h"
#include "KazaaLauncherDll.h"


KazaaControllerDestSyncher::KazaaControllerDestSyncher(void)
{
}

//
//
//
KazaaControllerDestSyncher::~KazaaControllerDestSyncher(void)
{
}

//
//
//
void KazaaControllerDestSyncher::InitParent(KazaaLauncherDll *parent)
{
	p_parent = parent;
}

//
//
//
void KazaaControllerDestSyncher::MapFinishedChanging(const char* source_ip)
{
	p_parent->NewNameListArrived(source_ip);
}