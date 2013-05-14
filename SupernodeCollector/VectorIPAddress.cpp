#include "StdAfx.h"
#include "vectoripaddress.h"

VectorIPAddress::VectorIPAddress(void)
{
	Clear();
}

//
//
//
VectorIPAddress::~VectorIPAddress(void)
{
}

//
//
//
void VectorIPAddress::Clear()
{
	v_ip_addresses.clear();
}


//
//
//
int VectorIPAddress::GetBufferLength()
{
	int len=0;
	len+=sizeof(int);	// number of IPAddress
	for(int i=0;i<(int)v_ip_addresses.size();i++)
	{
		len+=(int)v_ip_addresses[i].GetBufferLength();
	}
	return len;
}

//
//
//
int VectorIPAddress::WriteToBuffer(char *buf)
{
	int i;

	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	*((int *)ptr)=(int)v_ip_addresses.size();
	ptr+=sizeof(int);
	for(i=0;i<(int)v_ip_addresses.size();i++)
	{
		ptr+=v_ip_addresses[i].WriteToBuffer(ptr);
	}
	return GetBufferLength();
}

//
// Returns buffer length on read
//
int VectorIPAddress::ReadFromBuffer(char *buf)
{
	int i;
	Clear();
	char *ptr=buf;
	
	int len=*((int *)ptr);
	ptr+=sizeof(int);
	for(i=0;i<len;i++)
	{
		IPAddress ip;
		ptr+=ip.ReadFromBuffer(ptr);
		v_ip_addresses.push_back(ip);
	}
	return GetBufferLength();
}