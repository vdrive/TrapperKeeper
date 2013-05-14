#pragma once
#include "..\SupplyProcessor\SupplyData.h"

class SupplyDataByTrack
{
public:
	SupplyDataByTrack(void);
	~SupplyDataByTrack(void);
	UINT GetNumSupplies(){ return (UINT)v_supply_data.size(); }

	vector<SupplyData> v_supply_data;
};
