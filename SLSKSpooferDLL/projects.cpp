#include "stdafx.h"
#include "projects.h"
#include <fstream>


projects::projects()
{
	getProjects();
}

projects::~projects()
{
}

void projects::getProjects()
{
	char* proj;
	int pid;

	fstream fin("projects.txt", ios::in);

	while(!fin.eof())
	{
		fin >> pid >> proj;
		project p;
		p.project_id = pid;
		p.project_name = proj;
		Projects.push_back(p);
	}
}
