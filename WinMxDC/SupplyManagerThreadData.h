#pragma once
#include "ProjectKeywords.h"

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
	vector<ProjectKeywords>* p_keywords;
	HANDLE m_events[NumberOfEvents];
	bool m_abort_supply_retrieval;
};
