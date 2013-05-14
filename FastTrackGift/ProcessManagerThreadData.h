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

	int m_supply_progress;

	vector<ProjectKeywords>* p_projects;
	
	HANDLE m_events[NumberOfEvents];
};
