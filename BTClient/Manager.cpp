#include "StdAfx.h"
#include "manager.h"
//#include "FastTrackGiftDll.h"
#include "BTClientDLL.h"

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
void Manager::InitParent(BTClientDll *parent)
{
	p_parent=parent;
}