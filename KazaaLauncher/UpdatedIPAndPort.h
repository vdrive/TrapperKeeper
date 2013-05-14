#pragma once

class UpdatedIPAndPort
{
public:
	UpdatedIPAndPort(void);
	~UpdatedIPAndPort(void);
	int m_old_ip;
	int m_new_ip;
	int m_old_port;
	int m_new_port;
};
