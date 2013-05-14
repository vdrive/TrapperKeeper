// ConnectionModuleStatusData.h
#pragma once
#include "..\SupplyProcessor\SupplyData.h"

class ConnectionModuleStatusData
{
public:
	// Public Member Functions
	ConnectionModuleStatusData();
	~ConnectionModuleStatusData();
	void Clear();
	UINT m_num_peers;
	vector<SupplyData>v_supply_data;
};