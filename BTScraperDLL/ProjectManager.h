#pragma once

#include "projectdata.h"
#include "SQLInterface.h"

class ProjectManager
{
public:
	ProjectManager(void);
	~ProjectManager(void);

	int GetProjectData(vector<ProjectData> *v_project_data);
	void SetQuery(string query);
protected:
	SQLInterface m_sql;
	string m_query;
};
