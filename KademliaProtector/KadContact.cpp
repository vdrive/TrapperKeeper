#include "StdAfx.h"
#include "kadcontact.h"
#include <afxsock.h>		// MFC socket extensions

KadContact::KadContact(CUInt128 &hash,UINT ip,unsigned short port,CTime &create_time)
{
	m_ip=ip;
	m_port=port;
	m_hash=hash;
	m_create_time=create_time;
	m_last_bootstrap_request=CTime::GetCurrentTime()-CTimeSpan(0,1,59,45);
	in_addr a;
	*(u_long*)&a.S_un=(u_long)ip;

	m_sip=inet_ntoa(a);
	//TRACE("KadContact %s:%d created\n",m_sip.c_str(),m_port);
}

KadContact::KadContact()
{
	m_last_bootstrap_request=CTime::GetCurrentTime();
}

KadContact::~KadContact(void)
{
}

bool KadContact::IsExpired(void)
{
	CTimeSpan max_age(0,72,0,0);
	CTime now=CTime::GetCurrentTime();
	if(m_create_time<now-max_age)
		return true;
	else return false;
}

void KadContact::WriteToFile(HANDLE file)
{
	DWORD tmp;

	WriteFile(file,m_hash.getDataPtr(),sizeof(CUInt128),&tmp,NULL);
	WriteFile(file,&m_ip,sizeof(DWORD),&tmp,NULL);
	WriteFile(file,&m_port,sizeof(unsigned short),&tmp,NULL);
	WriteFile(file,&m_create_time,sizeof(m_create_time),&tmp,NULL);
}

byte* KadContact::ReadFromBuffer(byte* ptr)
{
	m_hash.setValueBE(ptr);
	ptr+=sizeof(CUInt128);
	m_ip=*(UINT*)ptr;
	ptr+=sizeof(UINT);
	m_port=*(unsigned short*)ptr;
	ptr+=sizeof(unsigned short);
	m_create_time=*(CTime*)ptr;
	ptr+=sizeof(CTime);

	in_addr a;
	*(u_long*)&a.S_un=(u_long)m_ip;

	m_sip=inet_ntoa(a);

	m_last_bootstrap_request=CTime::GetCurrentTime()-CTimeSpan(0,0,59,45);

	return ptr;
}
