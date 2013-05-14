// ThreadData.cpp

#include "stdafx.h"
#include "ThreadData.h"

//
//
//
ThreadData::ThreadData()
{
	Clear();
}

//
//
//
void ThreadData::Clear()
{
	m_hwnd=NULL;

	m_table_name.Empty();
	m_db_name.Empty();
	m_db_pass.Empty();
	m_db_login.Empty();
	m_db_ip.Empty();

	m_project.clear();
	v_results.clear();

	m_swarming = false;
	m_week_number = 32767;
}