#include "StdAfx.h"
#include "processmanagerthreaddata.h"

ProcessManagerThreadData::ProcessManagerThreadData(void)
{
	Clear();
}

//
//
//
ProcessManagerThreadData::~ProcessManagerThreadData(void)
{
	Clear();
}

//
//
//
void ProcessManagerThreadData::Clear()
{
	m_demand_progress = 0;
	m_supply_progress = 0;
	m_reverse_dns_progress = 0;
	//p_projects = NULL;
	while(v_projects.size())
	{
		delete *(v_projects.begin());
		v_projects.erase(v_projects.begin());
	}
}