#include "StdAfx.h"
#include "SupplyManagerThreadData.h"

SupplyManagerThreadData::SupplyManagerThreadData(void)
{
	Clear();
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
	p_keywords=NULL;
	m_abort_supply_retrieval=false;
}