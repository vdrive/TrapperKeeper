#include "StdAfx.h"
#include ".\ipanduserhash.h"

IPAndUserHash::IPAndUserHash(void)
{
	memset(&m_user_hash,0,16);
	m_port=4662; //default overnet tcp port
}

IPAndUserHash::~IPAndUserHash(void)
{
}

bool IPAndUserHash::operator ==(IPAndUserHash &other)
{
	if(m_ip.Compare(other.m_ip)==0)
		return true;
	else
		return false;
}

void IPAndUserHash::SetIP(const char* ip)
{
	m_ip = ip;
	int seed=0;
	for(int i=0;i<strlen(ip);i++)
		seed += (int)ip[i];
	srand(seed);
	for(int i=0;i<sizeof(m_user_hash);i++) //randomly create my user id hash
		m_user_hash[i]=rand()%256;
	srand((unsigned)time(NULL));
}
