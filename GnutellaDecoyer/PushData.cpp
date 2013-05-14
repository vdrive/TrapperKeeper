// PushData.cpp

#include "stdafx.h"
#include "PushData.h"

//
//
//
PushData::PushData()
{
	Init();
}

//
//
//
PushData::~PushData()
{
}

//========================//
// Data Reading Functions //
//========================//

//
//
//
GUID PushData::Guid()
{
	GUID *ptr=(GUID *)&m_data[0];
	return *ptr;
}

//
//
//
unsigned int PushData::FileIndex()
{	
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(GUID)];
	return *ptr;
}

//
// Big-Endian
//
unsigned int PushData::IP()
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(GUID)+sizeof(unsigned int)];
	return *ptr;
}

//
// Big-Endian
//
char *PushData::IPString()
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(GUID)+sizeof(unsigned int)];

	unsigned int ip_val=*ptr;

	unsigned int ip1,ip2,ip3,ip4;
	ip1=(ip_val>>0)&0xFF;
	ip2=(ip_val>>8)&0xFF;
	ip3=(ip_val>>16)&0xFF;
	ip4=(ip_val>>24)&0xFF;
	
	char ip[16];
	sprintf(ip,"%u.%u.%u.%u",ip1,ip2,ip3,ip4);
	
	return &ip[0];
}

//
//
//
unsigned short int PushData::Port()
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(GUID)+sizeof(unsigned int)+sizeof(unsigned int)];
	return *ptr;
}

//========================//
// Data Writing Functions //
//========================//

//
//
//
void PushData::Init()
{
	memset(m_data,0,sizeof(m_data));

	// Init the GUID
	GUID guid;
	CoCreateGuid(&guid);

	// To make ourselves look like "new" gnutella clients, set byte 8 to 0xFF and byte 15 to 0x00.
	unsigned char *ptr=(unsigned char *)&guid;
	ptr[8]=0xFF;
	ptr[15]=0x00;;
	
	Guid(guid);
}

//
//
//
void PushData::Init(GUID guid,unsigned int file_index,unsigned int ip,unsigned short int port)
{
	Guid(guid);
	FileIndex(file_index);
	IP(ip);
	Port(port);
}

//
//
//
void PushData::Init(GUID guid,unsigned int file_index,char *ip,unsigned short int port)
{
	Guid(guid);
	FileIndex(file_index);
	IP(ip);
	Port(port);
}

//
//
//
void PushData::Guid(GUID guid)
{
	GUID *ptr=(GUID *)&m_data[0];
	*ptr=guid;
}

//
//
//
void PushData::FileIndex(unsigned int file_index)
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(GUID)];
	*ptr=file_index;
}

//
// Big-Endian
//
void PushData::IP(unsigned int ip)
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(GUID)+sizeof(unsigned int)];
	*ptr=ip;
}

//
// Big-Endian
//
void PushData::IP(char *ip)
{
	unsigned int ip1,ip2,ip3,ip4;
	sscanf(ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);

	unsigned int ip_val;
	ip_val=(ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);

	unsigned int *ptr=(unsigned int *)&m_data[sizeof(GUID)+sizeof(unsigned int)];
	*ptr=ip_val;
}

//
//
//
void PushData::Port(unsigned short int port)
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(GUID)+sizeof(unsigned int)+sizeof(unsigned int)];
	*ptr=port;
}