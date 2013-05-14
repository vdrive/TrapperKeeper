#pragma once

class PoisonEntry
{
public:
	PoisonEntry(void);
	~PoisonEntry(void);

	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);

	PoisonEntry* operator=(const PoisonEntry &entry);


	string m_requester_ip;
	int m_request_port;
	UINT m_filesize;
	string m_filename;
	string m_md5;
	CTime m_timestamp;
};
