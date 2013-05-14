#pragma once

struct project
{
public:
	CString project_name;
	int project_id;
};

class projects
{
public:
	vector<project> Projects;
	projects();
	~projects();
	void getProjects(void);
};
