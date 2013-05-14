#pragma once
#include "DBInterface.h"
#include "ProjectKeywords.h"
#include "FastTrackFileInfo.h"

class SupplyEntryDBInterface :	public DBInterface
{
public:
	SupplyEntryDBInterface(void);
	~SupplyEntryDBInterface(void);

	bool RetrieveSupplyEntry(ProjectKeywords& keyword, vector<FastTrackFileInfo>& supplies, HWND hwnd);
};
