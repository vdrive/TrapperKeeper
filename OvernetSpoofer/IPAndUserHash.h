#pragma once

class IPAndUserHash
{
public:
	IPAndUserHash(void);
	~IPAndUserHash(void);
	bool operator ==(IPAndUserHash &other);
	void SetIP(const char* ip);

	CString m_ip;
	byte m_user_hash[16];
	unsigned short m_port;
};
