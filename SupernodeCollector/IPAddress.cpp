#include "StdAfx.h"
#include "ipaddress.h"

IPAddress::IPAddress(void)
{
	m_ip = 0;
	m_port = 0;
	Clear();
//	m_up_time = CTime::GetCurrentTime();
//	m_down_time = CTime::GetCurrentTime();
}

//
//
//
IPAddress::IPAddress(IPAndPort &ip)
{
	m_ip = ip.m_ip;
	m_port = ip.m_port;
	Clear();
//	m_up_time = CTime::GetCurrentTime();
//	m_down_time = CTime::GetCurrentTime();
}

//
//
//
IPAddress::~IPAddress(void)
{
	v_assigned_racks.clear();
}

//
//
//
bool IPAddress::operator == (const IPAddress & other)const
{
	if(m_ip == other.m_ip)
		if(m_port == other.m_port)
			return true;
	return false;
}

//
//
//
bool  IPAddress::operator < (const IPAddress& other)const
{
	if(this->m_ip < other.m_ip)
		return true;
	else
		return false;
}

//
//
//
bool IPAddress::operator != (const IPAddress & other)const
{
	if(m_ip == other.m_ip)
		if(m_port == other.m_port)
			return false;
	return true;
}

//
//
//
void IPAddress::Clear()
{
//	m_is_up = false;
	//m_up_time = CTime::GetCurrentTime();
	m_connecting = false;
//	m_rack_name = "NULL";
	m_username.Empty();
	m_num_assigned = 0;
	m_connecting_time = CTime::GetCurrentTime();
}


//
//
//
int IPAddress::GetBufferLength()
{
	int len=0;
	len += sizeof(m_ip);
	len += sizeof(m_port);
	len += sizeof(m_connecting);
	len += sizeof(m_num_assigned);
	len+=(int)m_username.GetLength()+1;	// +1 for NULL
	len += sizeof(CTime);
	return len;
}

//
//
//
int IPAddress::WriteToBuffer(char *buf)
{
//	int i;

	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	*((int*)ptr)=m_ip;
	ptr+=sizeof(int);
	*((int*)ptr)=m_port;
	ptr+=sizeof(int);
	*((bool*)ptr)=m_connecting;
	ptr+=sizeof(bool);
	*((int*)ptr)=m_num_assigned;
	ptr+=sizeof(int);

	strcpy(ptr,m_username);
	ptr+=m_username.GetLength()+1;	// +1 for NULL
	*((CTime*)ptr)=m_connecting_time;
	ptr+=sizeof(CTime);

	return GetBufferLength();
}

//
// Returns buffer length on read
//
int IPAddress::ReadFromBuffer(char *buf)
{
//	int i;
	Clear();
	char *ptr=buf;
	
	m_ip = *((int *)ptr);
	ptr+=sizeof(int);
	m_port = *((int *)ptr);
	ptr+=sizeof(int);
	m_connecting = *((bool *)ptr);
	ptr+=sizeof(bool);
	m_num_assigned = *((int *)ptr);
	ptr+=sizeof(int);

	m_username=ptr;
	ptr+=m_username.GetLength()+1;

	*((CTime*)ptr)=m_connecting_time;
	ptr+=sizeof(CTime);

	return GetBufferLength();
}

//
//
//
bool IPAddress::IsAssignedToThisRackAlready(const char* rack)
{
	for(UINT i=0; i<v_assigned_racks.size(); i++)
	{
		if(strcmp(rack, v_assigned_racks[i].c_str())==0)
			return true;
	}
	return false;
}

//
//
//
void IPAddress::RemoveThisAssignedRack(const char* rack)
{
	vector<string>::iterator iter = v_assigned_racks.begin();
	while( iter != v_assigned_racks.end())
	{
		if(strcmp(rack, (*iter).c_str())== 0)
		{
			v_assigned_racks.erase(iter);
			break;
		}
		iter++;
	}
}

//
//
//
void IPAddress::RemoveAllThisAssignedRack(const char* rack)
{
	vector<string>::iterator iter = v_assigned_racks.begin();
	while( iter != v_assigned_racks.end())
	{
		if(strcmp(rack, (*iter).c_str())== 0)
		{
			v_assigned_racks.erase(iter);
		}
		else
			iter++;
	}
}