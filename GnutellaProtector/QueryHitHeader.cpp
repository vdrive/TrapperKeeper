// QueryHitHeader.cpp

#include "stdafx.h"
#include "QueryHitHeader.h"

//
//
//
QueryHitHeader::QueryHitHeader()
{
	Clear();
}

//
//
//
QueryHitHeader::~QueryHitHeader()
{
}

//
//
//
void QueryHitHeader::Clear()
{
	memset(m_data,0,sizeof(m_data));
}

//=====================//
// Read Data Functions //
//=====================//
/*
//
//
//
unsigned int QueryHitHeader::Size()
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned char)+sizeof(unsigned short int)];
	return *ptr;
}
*/
//======================//
// Write Data Functions //
//======================//

//
//
//
void QueryHitHeader::NumberOfHits(unsigned char count)
{
	unsigned char *ptr=(unsigned char *)&m_data[0];
	*ptr=count;
}

//
//
//
void QueryHitHeader::Port(unsigned short int port)
{
	unsigned short int *ptr=(unsigned short int *)&m_data[sizeof(unsigned char)];
	*ptr=port;
}

//
//
//
void QueryHitHeader::IP(unsigned int ip)
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned char)+sizeof(unsigned short int)];
	*ptr=ip;
}

//
//
//
void QueryHitHeader::Speed(unsigned int speed)
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned char)+sizeof(unsigned short int)+sizeof(unsigned int)];
	*ptr=speed;
}