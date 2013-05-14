// BlubsterHeader.cpp

#include "stdafx.h"
#include "BlubsterHeader.h"

#include "BlubsterOpCode.h"

//
//
//
BlubsterHeader::BlubsterHeader()
{
	memset(m_data,0,sizeof(m_data));
}

//
//
//
BlubsterHeader::~BlubsterHeader()
{
}

//
//
//
unsigned int BlubsterHeader::EndianSwitch(unsigned int val)
{
	unsigned int ret=0;
	ret|=((val>>0) & 0xFF) << 24;
	ret|=((val>>8) & 0xFF) << 16;
	ret|=((val>>16) & 0xFF) << 8;
	ret|=((val>>24) & 0xFF) << 0;
	return ret;
}

//
//
//
unsigned short int BlubsterHeader::EndianSwitch(unsigned short int val)
{
	unsigned short int ret=0;
	ret|=((val>>0) & 0xFF) << 8;
	ret|=((val>>8) & 0xFF) << 0;
	return ret;
}

//
// Reading
//

//
//
//
unsigned short int BlubsterHeader::Checksum()
{
	unsigned short int *ptr=(unsigned short int *)&m_data[0];
	return EndianSwitch(*ptr);
}

//
//
//
unsigned int BlubsterHeader::Counter()
{
	return EndianSwitch(*((unsigned int *)&m_data[sizeof(unsigned int)]));
}

//
//
//
unsigned int BlubsterHeader::SourceIP()
{
//	return EndianSwitch(*((unsigned int *)&m_data[sizeof(unsigned int)*2]));
	return *((unsigned int *)&m_data[sizeof(unsigned int)*2]);
}

//
//
//
unsigned int BlubsterHeader::DestIP()
{
//	return EndianSwitch(*((unsigned int *)&m_data[sizeof(unsigned int)*3]));
	return *((unsigned int *)&m_data[sizeof(unsigned int)*3]);
}

//
//
//
unsigned char BlubsterHeader::TTL()
{
	return m_data[sizeof(unsigned int)*4];
}

//
//
//
unsigned char BlubsterHeader::Op()
{
	return m_data[sizeof(unsigned int)*4+1];
}

//
//
//
unsigned short int BlubsterHeader::DataLength()
{
	return EndianSwitch(*((unsigned short int *)&m_data[sizeof(unsigned int)*4+2]));
}

//
// Writing
//

//
//
//
void BlubsterHeader::Checksum(unsigned short int checksum)
{
	m_data[0]=(checksum>>8) & 0xFF;
	m_data[1]=(checksum>>0) & 0xFF;
}

//
//
//
void BlubsterHeader::Counter(unsigned int counter)
{
	*((unsigned int *)&m_data[sizeof(unsigned int)])=EndianSwitch(counter);
}

//
//
//
void BlubsterHeader::SourceIP(unsigned int ip)
{
//	*((unsigned int *)&m_data[sizeof(unsigned int)*2])=EndianSwitch(ip);
	*((unsigned int *)&m_data[sizeof(unsigned int)*2])=ip;
}

//
//
//
void BlubsterHeader::DestIP(unsigned int ip)
{
//	*((unsigned int *)&m_data[sizeof(unsigned int)*3])=EndianSwitch(ip);
	*((unsigned int *)&m_data[sizeof(unsigned int)*3])=ip;
}

//
//
//
void BlubsterHeader::TTL(unsigned char ttl)
{
	m_data[16]=ttl;
}

//
//
//
void BlubsterHeader::Op(unsigned char op)
{
	m_data[17]=op;
}

//
//
//
void BlubsterHeader::DataLength(unsigned short int len)
{
	m_data[18]=(len>>8)&0xFF;
	m_data[19]=(len>>0)&0xFF;
}
