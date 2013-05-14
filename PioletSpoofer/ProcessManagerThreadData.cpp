#include "StdAfx.h"
#include "processmanagerthreaddata.h"

ProcessManagerThreadData::ProcessManagerThreadData(void)
{
	Clear();
}

//
//
//
ProcessManagerThreadData::~ProcessManagerThreadData(void)
{
}

//
//
//
void ProcessManagerThreadData::Clear()
{
	m_spoof_progress = 0;
	p_projects = NULL;
}