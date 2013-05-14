#include "StdAfx.h"
#include "kazaaprocess.h"

KazaaProcess::KazaaProcess(void)
{
	m_process_id = 0;
	m_connecting_supernode_port = 0;
	m_connecting_supernode_ip = "";
	m_connected_supernode_port = 0;
	m_connected_supernode_ip = "";
	m_alive = true;
	m_desktop_num = 0;
	m_has_reported_to_kazaa_supply_taker = false;
}

//
//
//
KazaaProcess::~KazaaProcess(void)
{
}

//
//
//
/*
bool KazaaProcess::operator ==(const KazaaProcess& right)const
{
	if( (m_process_id == right.m_process_id)
		&& (m_supernode_ip.compare(right.m_supernode_ip)==0)
			&& (m_supernode_port == right.m_supernode_port))
		return true;
	else
		return false;
}
*/