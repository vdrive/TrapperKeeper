#include "StdAfx.h"
#include "syncherdest.h"
#include "WinMxDcDll.h"

SyncherDest::SyncherDest(void)
{
	m_id=0;
}

SyncherDest::~SyncherDest(void)
{
}

//
//
//
void SyncherDest::InitParent(WinMxDcDll *parent)
{
	p_parent = parent;
}

//
//
//
void SyncherDest::MapFinishedChanging(const char* source_ip)
{
	switch(m_id)
	{
		case 0:
		{
			p_parent->SupplySynched(source_ip);
			break;
		}
	}
}