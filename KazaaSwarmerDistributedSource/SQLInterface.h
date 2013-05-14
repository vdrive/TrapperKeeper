#pragma once
#include "fileentry.h"

class SQLInterface
{
public:
	SQLInterface(void);
	~SQLInterface(void);

	int GetCatalogData(vector<FileEntry> * entries,int num);
	int GetSpecialData(vector<FileEntry> * entries);
	int GetRegularEntries(vector<FileEntry> * entries, int num);
	int GetNonSpecialEntries(vector<FileEntry> * entries,int num);

};
