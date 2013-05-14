#include "StdAfx.h"
#include "manager.h"
#include "OvernetSpooferDll.h"

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
void Manager::InitParent(OvernetSpooferDll *parent)
{
	p_parent=parent;
}