// SupplyTableEntry.cpp

#include "stdafx.h"
#include "SupplyTableEntry.h"

//
//
//
SupplyTableEntry::SupplyTableEntry()
{
	Clear();
}

//
//
//
void SupplyTableEntry::Clear()
{
	m_project_id=0;
	m_user_id=0;
	m_file_id=0;
}