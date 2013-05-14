// PongData.cpp

#include "stdafx.h"
#include "PongData.h"

//
//
//
PongData::PongData()
{
	Init();
}

//
//
//
PongData::~PongData()
{
}

//========================//
// Data Reading Functions //
//========================//

//
//
//
unsigned short int PongData::Port()
{
	unsigned short int *ptr=(unsigned short int *)&m_data[0];
	return *ptr;
}

//
// Big-Endian
//
unsigned int PongData::IP()
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned short int)];
	return *ptr;
}

//
// Big-Endian
//
string PongData::IPString()
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned short int)];
	
	unsigned int ip_val=*ptr;

	unsigned int ip1,ip2,ip3,ip4;
	ip1=(ip_val>>0)&0xFF;
	ip2=(ip_val>>8)&0xFF;
	ip3=(ip_val>>16)&0xFF;
	ip4=(ip_val>>24)&0xFF;
	
	char ip[16];
	sprintf(ip,"%u.%u.%u.%u",ip1,ip2,ip3,ip4);
	
	string ret=ip;
	return ret;
}

//
//
//
unsigned int PongData::SharedFiles()
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned short int)+sizeof(unsigned int)];
	return *ptr;
}

//
//
//
unsigned int PongData::SharedKB()
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned short int)+sizeof(unsigned int)+sizeof(unsigned int)];
	return *ptr;
}

//========================//
// Data Writing Functions //
//========================//

//
//
//
void PongData::Init()
{
	memset(m_data,0,sizeof(m_data));
}

//
//
//
void PongData::Init(unsigned short int port,unsigned int ip,unsigned int shared_files,unsigned int shared_kb)
{
	Port(port);
	IP(ip);
	SharedFiles(shared_files);
	SharedKB(shared_kb);
}

//
//
//
void PongData::Init(unsigned short int port,char *ip,unsigned int shared_files,unsigned int shared_kb)
{
	Port(port);
	IP(ip);
	SharedFiles(shared_files);
	SharedKB(shared_kb);
}

//
//
//
void PongData::Port(unsigned short int port)
{
	unsigned short int *ptr=(unsigned short int *)&m_data[0];
	*ptr=port;
}

//
// Big-Endian
//
void PongData::IP(unsigned int ip)
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned short int)];
	*ptr=ip;
}

//
// Big-Endian
//
void PongData::IP(char *ip)
{
	unsigned int ip1,ip2,ip3,ip4;
	sscanf(ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);

	unsigned int ip_val;
	ip_val=(ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);

	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned short int)];
	*ptr=ip_val;
}

//
//
//
void PongData::SharedFiles(unsigned int shared_files)
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned short int)+sizeof(unsigned int)];
	*ptr=shared_files;
}

//
//
//
void PongData::SharedKB(unsigned int shared_kb)
{
	unsigned int *ptr=(unsigned int *)&m_data[sizeof(unsigned short int)+sizeof(unsigned int)+sizeof(unsigned int)];
	*ptr=shared_kb;
}
