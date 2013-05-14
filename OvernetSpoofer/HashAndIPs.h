#pragma once
#include "IPAndUserHash.h"

class HashAndIPs
{
public:
	HashAndIPs(void);
	~HashAndIPs(void);
	void Clear();
	bool operator < (const HashAndIPs&  other)const;
	bool operator ==(HashAndIPs &other);
	void SetHash(byte* hash);
	void InsertServerIP(IPAndUserHash& ip);
	void InsertServerSpoofedIP(IPAndUserHash& ip);

	byte m_hash[16];
	byte m_spoof_hash[16];
	UINT m_filesize;
	unsigned int m_checksum;
	vector<IPAndUserHash> v_server_IPs;
	vector<IPAndUserHash> v_server_spoofed_IPs;
};
class HashAndIPsHash : public hash_compare <HashAndIPs>
{
public:
	using hash_compare<HashAndIPs>::operator();
	size_t operator()(const HashAndIPs& key)const
	{
		return key.m_checksum;
	}
};

