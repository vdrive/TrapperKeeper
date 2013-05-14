#pragma once
#include "ProjectKeywords.h"

class ProcessManagerThreadData
{
public:

	enum vals
	{
		NumberOfEvents=3
	};
	ProcessManagerThreadData(void);
	~ProcessManagerThreadData(void);

	void Clear();

	int m_demand_progress;
	int m_supply_progress;
	int m_reverse_dns_progress;

	vector<ProjectKeywords>* p_projects;
	
	HANDLE m_events[NumberOfEvents];
};
