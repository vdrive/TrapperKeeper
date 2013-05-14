#pragma once
#include "ProcessDirectory.h"

class ProcessesDirectories
{
public:
	ProcessesDirectories(void);
	~ProcessesDirectories(void);
	void InsertDirectory(DWORD process_id, string directory);
	void InsertRealID(DWORD process_id);
	void RemoveDirectory(DWORD process_id);
	bool IsAlreadyExisted(string directory);
	void ClearAll();

private:
	vector<ProcessDirectory> v_directories;
};
