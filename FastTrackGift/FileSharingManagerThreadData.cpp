#include "StdAfx.h"
#include "FileSharingManagerThreadData.h"

FileSharingManagerThreadData::FileSharingManagerThreadData(void)
{
	Clear();
}

//
//
//
FileSharingManagerThreadData::~FileSharingManagerThreadData(void)
{
}

//
//
//
void FileSharingManagerThreadData::Clear()
{
//	p_thread_supply_projects->clear();
	p_thread_supply_projects=NULL;
	p_raw_db_info=NULL;
	p_keywords=NULL;
	m_abort_supply_retrieval=false;
}