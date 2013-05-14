#include "StdAfx.h"
#include "processesdirectories.h"

ProcessesDirectories::ProcessesDirectories(void)
{
}

//
//
//
ProcessesDirectories::~ProcessesDirectories(void)
{
	v_directories.clear();
}

//
//
//
void ProcessesDirectories::InsertDirectory(DWORD process_id, string directory)
{
	ProcessDirectory pd;
	pd.m_process_id = process_id;
	pd.m_process_running_folder = directory;
	v_directories.push_back(pd);
}

//
//
//
void ProcessesDirectories::RemoveDirectory(DWORD process_id)
{
	vector<ProcessDirectory>::iterator iter = v_directories.begin();
	while(iter != v_directories.end())
	{
		if(iter->m_process_id == process_id)
		{
			v_directories.erase(iter);
			break;
		}
		iter++;
	}
}

//
//
//
void ProcessesDirectories::InsertRealID(DWORD process_id)
{
	vector<ProcessDirectory>::iterator iter = v_directories.begin();
	while(iter != v_directories.end())
	{
		if(iter->m_process_id == 0)
		{
			iter->m_process_id = process_id;
			break;
		}
		iter++;
	}
}

//
//
//
bool ProcessesDirectories::IsAlreadyExisted(string directory)
{
	bool found = false;
	vector<ProcessDirectory>::iterator iter = v_directories.begin();
	while(iter != v_directories.end())
	{
		if(stricmp(iter->m_process_running_folder.c_str(), directory.c_str())==0)
		{
			found = true;
			break;
		}
		iter++;
	}	
	return found;
}

//
//
//
void ProcessesDirectories::ClearAll()
{
	v_directories.clear();
}