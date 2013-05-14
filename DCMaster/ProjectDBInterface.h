#pragma once
#include "mysql.h"
#include "ProjectKeywords.h"

class ProjectDBInterface
{
public:
	ProjectDBInterface(void);
	~ProjectDBInterface(void);
	bool OpenDBConnection(/*CString ip, CString login, CString pass, CString db*/);
	int ReadProjectKeywords(vector<ProjectKeywords>& keywords);
	string QueryTrackName(const char* project_id, const char* track_number, bool& single);
	void CloseDBConnection();

	MYSQL *p_conn;
};
