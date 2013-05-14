#pragma once
#include "QueryHit.h"
//#include "Query.h"

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
	vector<QueryHit *> *p_project_query_hits;
//	vector<Query *> *p_project_queries;
	
	HANDLE m_events[NumberOfEvents];

	CString m_today_date_string;
};
