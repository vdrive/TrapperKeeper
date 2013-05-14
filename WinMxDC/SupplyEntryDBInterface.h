#pragma once
#include "DBInterface.h"
#include "ProjectKeywords.h"
#include "QueryHit.h"
class SupplyEntryDBInterface :	public DBInterface
{
public:
	SupplyEntryDBInterface(void);
	~SupplyEntryDBInterface(void);

	bool RetrieveSupplyEntry(ProjectKeywords& keyword, vector<QueryHit>& supplies, HWND hwnd);
};
