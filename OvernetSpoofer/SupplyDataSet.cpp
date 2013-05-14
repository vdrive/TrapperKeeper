#include "StdAfx.h"
#include "supplydataset.h"

SupplyDataSet::SupplyDataSet(void)
{
	Clear();
}

SupplyDataSet::SupplyDataSet(UINT project_id)
{
	Clear();
	m_project_id=project_id;
}

SupplyDataSet::~SupplyDataSet(void)
{
}

//
//
//
bool SupplyDataSet::operator < (const SupplyDataSet& other)const
{
	if(this->m_project_id < other.m_project_id)
		return true;
	else
		return false;
}

//
//
//
bool SupplyDataSet::operator ==(SupplyDataSet &other)
{
	return (this->m_project_id == other.m_project_id);
}

//
//
//
void SupplyDataSet::Clear()
{
	m_project_id=0;
}

//
//
//
UINT SupplyDataSet::GetNumSupplies()
{
	UINT size=0;
	for(UINT i=0;i<v_supply_data_by_track.size();i++)
		size += v_supply_data_by_track[i].GetNumSupplies();
	return size;
}
