#include "StdAfx.h"
#include "SupplyManagerThreadData.h"

SupplyManagerThreadData::SupplyManagerThreadData(void)
{
	p_thread_supply_projects=NULL;
	p_sm_supply_projects=NULL;
	p_piolet_raw_db_info=NULL;
}

//
//
//
SupplyManagerThreadData::~SupplyManagerThreadData(void)
{
}

//
//
//
void SupplyManagerThreadData::Clear()
{
	p_thread_supply_projects->clear();
	p_sm_supply_projects->clear();
}