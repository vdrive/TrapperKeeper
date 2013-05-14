#include "StdAfx.h"
#include "manager.h"
#include "GnutellaProtectorDll.h"

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
void Manager::InitParent(GnutellaProtectorDll *parent)
{
	p_parent=parent;
}