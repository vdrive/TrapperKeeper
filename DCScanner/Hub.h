#include <string.h>
#pragma once

class Hub
{
public:
	Hub(void);
	~Hub(void);
	bool m_connected;
	int m_users;
	char* m_name;
	char* m_description;
	char* m_address;
	int m_port;
};
