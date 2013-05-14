//NetAddress.cpp

#include "stdafx.h"
#include "NetAddress.h"

//
//
//
NetAddress::NetAddress()
{
	m_ip = 0;
	m_port = 0;
}
	
//
//
//
void NetAddress::SetIP(unsigned int ip)
{
	m_ip = ip;
}

//
//
//
void NetAddress::SetIP(string ip)
{
	char ip_string[16];
	strcpy(ip_string, ip.c_str());

	char * second;
	char * third;
	char * fourth;

	second = strchr(ip_string, '.');
	if(second != NULL)
	{
		second[0] = '\0';
		second++;
	}
	else 
	{
		m_ip = 0;
		return;
	}

	third = strchr(second, '.');
	
	if(third!=NULL)
	{
		third[0] = '\0';
		third++;
	}
	else 
	{
		m_ip = 0;
		return;
	}

	fourth = strchr(third, '.');

	if(fourth!=NULL)
	{
		fourth[0] = '\0';
		fourth++;
	}
	else 
	{
		m_ip = 0;
		return;
	}

	char * temp = (char*)(&m_ip);
	
	temp[0] = atoi(ip_string);
	temp[1] = atoi(second);
	temp[2] = atoi(third);
	temp[3] = atoi(fourth);
}
	
//
//
//
void NetAddress::SetPort(unsigned int port)
{
	m_port = port;
}

//
//
//
string NetAddress::GetIPString()
{
	unsigned char *temp;
	temp = (unsigned char*)(&m_ip);
	int first = temp[0];
	int second = temp[1];
	int third = temp[2];
	int fourth = temp[3];

	char element[16];

	_itoa(fourth,element,10);
	string ip_string;
	ip_string.append(element);
	ip_string.append(".");

	_itoa(third,element,10);
	ip_string.append(element);
	ip_string.append(".");

	_itoa(second,element,10);
	ip_string.append(element);
	ip_string.append(".");

	_itoa(first,element,10);
	ip_string.append(element);

	return ip_string;
}
	
//
//
//
string NetAddress::GetReversedIPString()
{
	unsigned char *temp;
	temp = (unsigned char*)(&m_ip);
	int first = temp[3];
	int second = temp[2];
	int third = temp[1];
	int fourth = temp[0];

	char element[16];

	_itoa(fourth,element,10);
	string ip_string;
	ip_string.append(element);
	ip_string.append(".");

	_itoa(third,element,10);
	ip_string.append(element);
	ip_string.append(".");

	_itoa(second,element,10);
	ip_string.append(element);
	ip_string.append(".");

	_itoa(first,element,10);
	ip_string.append(element);

	return ip_string;
}
//
//
//
unsigned int NetAddress::GetIPInt()
{
	return m_ip;
}

//
//
//
unsigned int NetAddress::GetPort()
{
	return m_port;
}

//
//
//
bool NetAddress::IsNull()
{
	if(m_ip==0)
		return true;
	else return false;
}
