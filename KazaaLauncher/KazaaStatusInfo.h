// KazaaStatusInfo.h: interface for the KazaaStatusInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
class KazaaStatusInfo  
{
public:
	KazaaStatusInfo();
	virtual ~KazaaStatusInfo();

	//DWORD m_process_id;
	//char m_account_name[50];
	int m_ip;
	int m_port;

};