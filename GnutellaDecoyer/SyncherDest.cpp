#include "StdAfx.h"
#include "syncherdest.h"
#include "GnutellaDecoyerDll.h"

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
void SyncherDest::InitParent(GnutellaDecoyerDll *parent, int id)
{
	p_parent = parent;
	m_id=id;
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
			p_parent->DecoySupplySynched();
			break;
		}
	}
}