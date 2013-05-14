// QueryHitResultHeader.cpp

#include "stdafx.h"
#include "QueryHitResultHeader.h"

//
//
//
QueryHitResultHeader::QueryHitResultHeader()
{
	Clear();
}

//
//
//
QueryHitResultHeader::~QueryHitResultHeader()
{
}

//
//
//
void QueryHitResultHeader::Clear()
{
	memset(m_data,0,sizeof(m_data));
}

//=====================//
// Read Data Functions //
//=====================//

//
//
//
unsigned int QueryHitResultHeader::Size()
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned int)];
	return *ptr;
}

//
//
//
unsigned int QueryHitResultHeader::Index()
{
	unsigned int *ptr=(unsigned int *)&m_data[0];
	return *ptr;
}

//======================//
// Write Data Functions //
//======================//

//
//
//
void QueryHitResultHeader::Index(unsigned int index)
{
	unsigned int *ptr=(unsigned int *)&m_data[0];
	*ptr=index;
}

//
//
//
void QueryHitResultHeader::Size(unsigned int size)
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned int)];
	*ptr=size;
}