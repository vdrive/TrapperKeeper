#include "StdAfx.h"
#include "manager.h"
#include "OvernetProtectorDll.h"

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
void Manager::InitParent(OvernetProtectorDll *parent)
{
	p_parent=parent;
}