#include "StdAfx.h"
#include "manager.h"
#include "GnutellaSupplyDll.h"

Manager::Manager(void)
{
}

//
//
//
Manager::~Manager(void)
{
}

//
//
//
void Manager::InitParent(GnutellaSupplyDll *parent)
{
	p_parent=parent;
}