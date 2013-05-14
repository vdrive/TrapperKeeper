#include "StdAfx.h"
#include "SupplyManagerThreadData.h"

SupplyManagerThreadData::SupplyManagerThreadData(void)
{
	p_thread_supply_projects=NULL;
	p_sm_supply_projects=NULL;
	p_gnutella_raw_db_info=NULL;
	m_retrieve_zero_size_only = false;
	m_abort_supply_retrieval = false;
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
	m_retrieve_zero_size_only = false;
	m_abort_supply_retrieval = false;
}