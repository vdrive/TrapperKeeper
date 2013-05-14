// GnutellaHeader.cpp

#include "stdafx.h"
#include "GnutellaHeader.h"

//
//
//
GnutellaHeader::GnutellaHeader()
{
	Init();
}

//
//
//
void GnutellaHeader::Init()
{
//	memset(this,0,sizeof(GnutellaHeader));
	memset(m_data,0,sizeof(m_data));

	// Init the GUID
	GUID guid;
	CoCreateGuid(&guid);

	// To make ourselves look like "new" gnutella clients, set byte 8 to 0xFF and byte 15 to 0x00.
	unsigned char *ptr=(unsigned char *)&guid;
	ptr[8]=0xFF;
	ptr[15]=0x00;;

	Guid(guid);

	// Init the TTL to 4
	TTL(4);
}

//========================//
// Data Reading Functions //
//========================//

//
//
//
GUID GnutellaHeader::Guid()
{
	GUID *ptr=(GUID *)&m_data[0];
	return *ptr;
}

//
//
//
unsigned char GnutellaHeader::Op()
{
	unsigned char *ptr=(unsigned char *)&m_data[sizeof(GUID)];
	return *ptr;
}

//
//
//
unsigned char GnutellaHeader::TTL()
{
	unsigned char *ptr=(unsigned char *)&m_data[sizeof(GUID)+sizeof(char)];
	return *ptr;
}

//
//
//
unsigned char GnutellaHeader::Hops()
{
	unsigned char *ptr=(unsigned char *)&m_data[sizeof(GUID)+sizeof(char)+sizeof(char)];
	return *ptr;
}

//
//
//
unsigned int GnutellaHeader::Length()
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(GUID)+sizeof(char)+sizeof(char)+sizeof(char)];
	return *ptr;
}

//========================//
// Data Writing Functions //
//========================//

//
//
//
void GnutellaHeader::Guid(GUID guid)
{
	GUID *ptr=(GUID *)&m_data[0];
	*ptr=guid;
}

//
//
//
void GnutellaHeader::Op(unsigned char op)
{
	unsigned char *ptr=(unsigned char *)&m_data[sizeof(GUID)];
	*ptr=op;
}

//
//
//
void GnutellaHeader::TTL(unsigned char ttl)
{
	unsigned char *ptr=(unsigned char *)&m_data[sizeof(GUID)+sizeof(char)];
	*ptr=ttl;
}

//
//
//
void GnutellaHeader::Hops(unsigned char hops)
{
	unsigned char *ptr=(unsigned char *)&m_data[sizeof(GUID)+sizeof(char)+sizeof(char)];
	*ptr=hops;
}

//
//
//
void GnutellaHeader::Length(unsigned int length)
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(GUID)+sizeof(char)+sizeof(char)+sizeof(char)];
	*ptr=length;
}
