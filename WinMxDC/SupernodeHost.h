#pragma once

class SupernodeHost
{
public:
	SupernodeHost(void);
	~SupernodeHost(void);
	SupernodeHost(const SupernodeHost &host);	// copy constructor
	bool operator < (const SupernodeHost&  other)const;
	bool operator ==(SupernodeHost &host);
	SupernodeHost& operator=(const SupernodeHost &host);
	void Clear();
	void GetIP(char* ip);
	void SetIP(const char* ip);

	DWORD m_ip;
	unsigned short m_port;
};

class SupernodeHostHash : public hash_compare <SupernodeHost>
{
public:
	using hash_compare<SupernodeHost>::operator();
	size_t operator()(const SupernodeHost& key)const
	{
		return key.m_ip;
	}

};