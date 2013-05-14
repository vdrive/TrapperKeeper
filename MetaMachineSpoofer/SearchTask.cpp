#include "StdAfx.h"
#include "searchtask.h"

SearchTask::SearchTask(const char* project, const char* search_string, UINT minimum_size)
{
	m_project=project;
	m_search_string=search_string;
	m_minimum_size=minimum_size;
}

SearchTask::SearchTask()
{
	m_minimum_size=0;
}


SearchTask::~SearchTask(void)
{
}
