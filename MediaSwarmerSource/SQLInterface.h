#pragma once

#include "FileEntry.h"

#include "mysql.h"

class SQLInterface
{
public:
	SQLInterface(void);
	~SQLInterface(void);

	void GetData(vector<FileEntry> *entries, int num);
};
