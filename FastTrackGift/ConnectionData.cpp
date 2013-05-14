// ConnectionData.cpp

#include "stdafx.h"
#include "ConnectionData.h"

//
//
//
ConnectionData::ConnectionData()
{
//	memset(this,0,sizeof(ConnectionData));

	m_ip=0;
	m_filename.erase();
	p_socket=NULL;
}

//
//
//
bool ConnectionData::operator ==(ConnectionData &data)
{
	if(m_ip!=data.m_ip)
	{
		return false;
	}

	if(strcmp(m_filename.c_str(),data.m_filename.c_str())!=0)
	{
		return false;
	}

	return true;
}