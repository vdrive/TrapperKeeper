#pragma once

class DataBaseInfo
{
public:
	DataBaseInfo(void);
	~DataBaseInfo(void);
	DataBaseInfo(const DataBaseInfo &entry);
	DataBaseInfo& operator=(const DataBaseInfo &entry);

	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);	// returns buffer length

	string m_db_host;
	string m_db_user;
	string m_db_password;
};
