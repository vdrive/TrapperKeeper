#include "StdAfx.h"
#include "dbmanagerthreaddata.h"

DBManagerThreadData::DBManagerThreadData(void)
{
	Clear();
}

//
//
//
DBManagerThreadData::~DBManagerThreadData(void)
{
}

//
//
//
void DBManagerThreadData::Clear()
{
	p_project_query_hits=NULL;
	m_today_date_string.Empty();
//	p_project_queries=NULL;
}