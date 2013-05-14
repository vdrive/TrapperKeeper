#pragma once
#include "ProjectDataStructure.h"
#include "ProjectKeywords.h"

class ProjectKeywordsVector : public ProjectDataStructure
{
public:
	ProjectKeywordsVector(void);
	~ProjectKeywordsVector(void);
	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);

	vector<ProjectKeywords> v_projects;
};
