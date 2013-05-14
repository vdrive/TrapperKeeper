#pragma once

struct keyword
{
public:
	int project_id;
	CString word;
	int weight;
};

class keywords
{
public:
	vector<keyword> Keywords;
	keywords();
	~keywords();
	void getKeywords(void);
};