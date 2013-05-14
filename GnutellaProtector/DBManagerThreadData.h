#pragma once
#include "Query.h"

class DBManagerThreadData
{
public:
	enum vals
	{
		NumberOfEvents=3
	};

	DBManagerThreadData(void);
	~DBManagerThreadData(void);

	void Clear();
	vector<Query *> *p_project_queries;
	
	HANDLE m_events[NumberOfEvents];
};
