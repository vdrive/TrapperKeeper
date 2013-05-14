#pragma once
#include "QueryHitResult.h"
#include "UploadInfo.h"

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
	vector<QueryHitResult>* p_thread_supply_projects;
	HANDLE m_events[NumberOfEvents];
	//bool m_abort_supply_retrieval;
	vector<UploadInfo> v_upload_logs;
};
