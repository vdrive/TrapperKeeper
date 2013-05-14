#include "StdAfx.h"
#include "aresudpsupernode.h"

AresUDPSupernode::AresUDPSupernode(unsigned int ip,unsigned short port)
:AresHost(ip,port)
{
	m_last_response=CTime::GetCurrentTime();
	m_create_time=CTime::GetCurrentTime();
	m_last_1e_ping=CTime::GetCurrentTime();
	m_last_1e_receive=m_create_time;
	m_last_search=CTime::GetCurrentTime();
	m_last_search_result=CTime::GetCurrentTime();
	mb_alive=false;
	mb_saveable=false;
	mb_accepted=false;

}

AresUDPSupernode::AresUDPSupernode(const char* ip,unsigned short port)
:AresHost(ip,port)
{
	m_last_response=CTime::GetCurrentTime();
	m_create_time=CTime::GetCurrentTime();
	m_last_1e_ping=CTime::GetCurrentTime();
	m_last_1e_receive=CTime::GetCurrentTime();
	m_last_search=CTime::GetCurrentTime();
	m_last_search_result=CTime::GetCurrentTime();
	mb_alive=false;
	mb_saveable=false;
	mb_accepted=false;

}

AresUDPSupernode::~AresUDPSupernode(void){

}