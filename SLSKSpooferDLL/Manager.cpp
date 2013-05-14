#include "StdAfx.h"
#include "Manager.h"
#include "SLSKController.h"

Manager::Manager(void)
{
	p_parent = NULL;
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
void Manager::InitParent(SLSKController *parent)
{
	p_parent=parent;
}