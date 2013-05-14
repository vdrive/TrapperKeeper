#pragma once
#include "SupplyProject.h"
#include "DataBaseInfo.h"

class SupplyManagerThreadData
{
public:
	enum vals
	{
		NumberOfEvents=3
	};

	SupplyManagerThreadData(void);
	~SupplyManagerThreadData(void);

	void Clear();
	vector<SupplyProject>* p_thread_supply_projects;
	vector<SupplyProject>* p_sm_supply_projects;
	DataBaseInfo* p_piolet_raw_db_info;
	HANDLE m_events[NumberOfEvents];
};
