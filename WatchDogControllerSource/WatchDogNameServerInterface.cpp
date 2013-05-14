#include "StdAfx.h"
#include "watchdognameserverinterface.h"
#include "WatchDogControllerSourceDll.h"

WatchDogNameServerInterface::WatchDogNameServerInterface(void)
{
	//p_parent = NULL;
}

WatchDogNameServerInterface::~WatchDogNameServerInterface(void)
{
}

//
//
//
void WatchDogNameServerInterface::InitParent(WatchDogControllerSourceDll *parent)
{
	p_parent = parent;
}

//
//
//
void WatchDogNameServerInterface::ReceivedIPs(vector<string> &v_ips)
{	
	p_parent->v_ip_list = v_ips;
	/*if (p_parent != NULL)
	{
		p_parent->ReceivedIPs(v_ips);
	}*/
}
