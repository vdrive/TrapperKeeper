#include "StdAfx.h"
#include ".\btiplist.h"

BTIPList::BTIPList(void)
{
//	m_torrent_id = 0;
	m_num_ips = 0;
}

BTIPList::BTIPList(int id)
{
//	m_torrent_id = id;
	m_num_ips = 0;
}

BTIPList::~BTIPList(void)
{
}

vector<BTIP> BTIPList::GetIps()
{
	
	return v_ips;

}

/*
int BTIPList::GetID()
{
	
	return m_torrent_id;
}

void BTIPList::SetID(int id)
{
	
	m_torrent_id = id;
}
*/

void BTIPList::CalculateNumIPs()
{
	
	m_num_ips = (int)v_ips.size(); 
}

void BTIPList::AddIpList(vector<BTIP> ips)
{

	for (int i = 0; i < (int)ips.size(); i++)
	{
		int found = 0;
		for (int j = 0; j < (int)v_ips.size(); j ++)
		{
			if (strcmp(ips[i].m_ip,v_ips[j].m_ip) == 0)
			{
				found = 1;
				break;
			}

		}
		if (found == 0)
		{
			v_ips.push_back(ips[i]);
		}
	}
}

void BTIPList::AddIp(BTIP ip)
{


	int found = 0;
	for (int j = 0; j < (int)v_ips.size(); j ++)
	{
		if (strcmp(ip.m_ip,v_ips[j].m_ip) == 0)
		{
			found = 1;
			break;
		}


	}
	if (found == 0)
	{
		v_ips.push_back(ip);
	}

	
}

void BTIPList::Clear()
{
	m_num_ips = 0;
	v_ips.clear();
}

int BTIPList::GetBufferLength()
{
	int len = 0;

	len += sizeof(m_num_ips);

	for(int i=0; (int)i<v_ips.size(); i++)
	{
		len += v_ips[i].GetBufferLength();
	}

	return len;	
}

int BTIPList::WriteToBuffer(char * buf)
{

	char * ptr =  buf;
	int buf_length = GetBufferLength();

	*((int *)ptr)=m_num_ips;
	ptr+=sizeof(unsigned int);

	for(int i=0; i<(int)v_ips.size(); i++)
	{	
		ptr+=v_ips[i].WriteToBuffer(ptr);
	}

	return buf_length;


}

int BTIPList::ReadFromBuffer(char * buf)
{

	Clear();

	char *ptr = buf;
	char *bufbegin = buf;


	m_num_ips=*((int *)ptr);
	ptr+=sizeof(int);

	for(int i=0;i<m_num_ips;i++)
	{
		BTIP ip;
		ip.Clear();
		ptr += ip.ReadFromBuffer(ptr);
		v_ips.push_back(ip);
	}

	return (int)(ptr-bufbegin);


}
