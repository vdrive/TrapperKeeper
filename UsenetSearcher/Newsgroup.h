#pragma once

class Newsgroup
{
public:
	Newsgroup(void);
	~Newsgroup(void);

	CString m_name;
	unsigned int m_last_header_received;
};
