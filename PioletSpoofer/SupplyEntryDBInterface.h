#pragma once
#include "DBInterface.h"
#include "SupplyProject.h"
class SupplyEntryDBInterface :	public DBInterface
{
public:
	SupplyEntryDBInterface(void);
	~SupplyEntryDBInterface(void);

	int RetrieveSupplyEntry(SupplyProject& sp,int entry_limit);
	int RetrievePoisonEntry(SupplyProject& sp,int entry_limit);
};
