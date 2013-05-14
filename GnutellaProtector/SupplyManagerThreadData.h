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
	DataBaseInfo* p_gnutella_raw_db_info;
	HANDLE m_events[NumberOfEvents];
	bool m_retrieve_zero_size_only;
	bool m_abort_supply_retrieval;
};
