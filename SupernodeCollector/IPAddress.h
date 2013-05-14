#pragma once
#include <hash_set>
#include "IPAndPort.h"

class IPAddress
{
public:
	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);
	IPAddress(void);
	IPAddress(IPAndPort& ip);
	~IPAddress(void);

	bool operator == (const IPAddress & other)const;
	bool operator < (const IPAddress&  other)const;
	bool operator != (const IPAddress& other)const;
	bool IsAssignedToThisRackAlready(const char* rack);
	void RemoveThisAssignedRack(const char* rack);
	void RemoveAllThisAssignedRack(const char* rack);

	int m_ip;
	int m_port;
	bool m_connecting;
//	bool m_is_up;
//	CTime m_up_time;
//	CTime m_down_time;
//	string m_rack_name;
	int m_num_assigned;
	CString m_username;
	CTime m_connecting_time;

	vector<string> v_assigned_racks;	
};


class IPAddressHash : public hash_compare <IPAddress>
{
public:
	using hash_compare<IPAddress>::operator();
	size_t operator()(const IPAddress& key)const
	{
		return key.m_ip;
	}

};
