#pragma once
#include "SupplyDataByTrack.h"

class SupplyDataSet
{
public:
	SupplyDataSet(void);
	SupplyDataSet(UINT project_id);
	~SupplyDataSet(void);	
	void Clear();
	bool operator ==(SupplyDataSet &other);
	bool operator < (const SupplyDataSet&  other)const;
	UINT GetNumSupplies();

	UINT m_project_id;
	vector<SupplyDataByTrack> v_supply_data_by_track; //v_supply_data_by_track[0] is track 0, v_supply_data_by_track[1] is track 1...etc
};

class SupplyDataSetHash : public hash_compare <SupplyDataSet>
{
public:
	using hash_compare<SupplyDataSet>::operator();
	size_t operator()(const SupplyDataSet& key)const
	{
		return key.m_project_id;
	}
};

