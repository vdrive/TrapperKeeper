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
	p_thread_supply_projects=NULL;
	//m_abort_supply_retrieval=false;
}