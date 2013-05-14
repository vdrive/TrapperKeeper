#include "StdAfx.h"
#include "getrealprocessidthreaddata.h"

GetRealProcessIDThreadData::GetRealProcessIDThreadData(void)
{
	m_hwnd = NULL;
	m_process_id = 0;
	m_ending_thread=false;
}

GetRealProcessIDThreadData::~GetRealProcessIDThreadData(void)
{
}
