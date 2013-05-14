#pragma once
#include "mysql.h"
#include "ProjectKeywords.h"

class ProjectDBInterface
{
public:
	ProjectDBInterface(void);
	~ProjectDBInterface(void);
	bool OpenDBConnection(CString ip, CString login, CString pass, CString db);
	int ReadProjectKeywords(vector<ProjectKeywords>& keywords);
	void CloseDBConnection();

	MYSQL *p_conn;
};
