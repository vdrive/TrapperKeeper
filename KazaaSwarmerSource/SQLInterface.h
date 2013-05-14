#pragma once

#include "FileEntry.h"
#include "MusicProject.h"

#include "mysql.h"

class SQLInterface
{
public:
	SQLInterface(void);
	~SQLInterface(void);

	bool GetData(vector<FileEntry> *entries, int num,int numcatalog);// vector<MusicProject> projects);
};
