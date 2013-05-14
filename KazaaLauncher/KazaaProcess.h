#pragma once

class KazaaProcess
{
public:
	KazaaProcess(void);
	~KazaaProcess(void);
//	bool operator ==(const KazaaProcess& right)const;

	DWORD m_process_id;
	
	string m_connecting_supernode_ip; //supernode that we are supposed to connect
	int m_connecting_supernode_port;

	string m_connected_supernode_ip; //supernode that this process connected to
	int m_connected_supernode_port;

	bool m_alive;
	int m_desktop_num;
	bool m_has_reported_to_kazaa_supply_taker;
	CString m_running_dir;
};
