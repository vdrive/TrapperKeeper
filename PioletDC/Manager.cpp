#include "StdAfx.h"
#include "manager.h"
#include "PioletDCDll.h"

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
void Manager::InitParent(PioletDCDll *parent)
{
	p_parent=parent;
}