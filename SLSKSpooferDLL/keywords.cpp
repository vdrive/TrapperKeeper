#include "stdafx.h"
#include "keywords.h"
#include <fstream>

keywords::keywords()
{
	getKeywords();
}

keywords::~keywords()
{
}

void keywords::getKeywords()
{
	int id;
	char* w;
	int value;

	fstream fin("keywords.txt", ios::in);

	while(!fin.eof())
	{
		fin >> id >> w >> value;
		keyword k;
		k.project_id = id;
		k.word = w;
		k.weight = value;
		Keywords.push_back(k);
	}
}