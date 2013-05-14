#pragma once
#include "FastTrackFileInfo.h"
#include "DataBaseInfo.h"
#include "ProjectKeywords.h"

class FileSharingManagerThreadData
{
public:
	enum vals
	{
		NumberOfEvents=3
	};

	FileSharingManagerThreadData(void);
	~FileSharingManagerThreadData(void);

	void Clear();
	vector<FastTrackFileInfo>* p_thread_supply_projects;
	vector<ProjectKeywords>* p_keywords;
	DataBaseInfo* p_raw_db_info;
	HANDLE m_events[NumberOfEvents];
	bool m_abort_supply_retrieval;
};
