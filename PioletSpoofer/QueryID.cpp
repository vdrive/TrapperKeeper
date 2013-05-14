// QueryID.cpp

#include "stdafx.h"
#include "QueryID.h"

//
//
//
QueryID::QueryID()
{
	Clear();
}

//
//
//
QueryID::~QueryID()
{
}

//
//
//
void QueryID::Clear()
{
	m_ip=0;
	m_count=0;
}