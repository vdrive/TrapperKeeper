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
	p_project_queries=NULL;
}