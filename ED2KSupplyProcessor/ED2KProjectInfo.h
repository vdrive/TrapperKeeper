#pragma once
#include "ProjectKeywords.h"
#include "..\tkcom\Object.h"

class ED2KProjectInfo : public Object
{
public:
	ProjectKeywords m_project_keywords;

	vector<string> mv_cross_name_hashes;
	vector<string> mv_anti_leak_hashes;

	ED2KProjectInfo(void);
	~ED2KProjectInfo(void);
};
