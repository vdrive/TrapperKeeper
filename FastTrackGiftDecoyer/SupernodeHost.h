#pragma once

class SupernodeHost
{
public:
	SupernodeHost(void);
	~SupernodeHost(void);
	bool operator < (const SupernodeHost&  other)const;
	bool operator ==(SupernodeHost &host);
	void Clear();
	void GetIP(char* ip);
	void SetIP(const char* ip);

	unsigned int m_ip;
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