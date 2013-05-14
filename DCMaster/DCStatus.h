#pragma once
#include "ProjectKeywords.h"

class DCStatus
{
public:
	DCStatus(void);
	~DCStatus(void);
	bool operator == (const DCStatus& dc_status) const;
	bool operator == (char* ip) const;
	bool operator < (const DCStatus& dc_status) const;

	CTime m_last_init_response_time;
	vector<ProjectKeywords> v_projects;
	char m_ip[16];
};
