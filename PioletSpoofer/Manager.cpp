#include "StdAfx.h"
#include "manager.h"
#include "PioletSpooferDll.h"

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
void Manager::InitParent(PioletSpooferDll *parent)
{
	p_parent=parent;
}