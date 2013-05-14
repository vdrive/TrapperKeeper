#pragma once

class DatfileRecord
{
public:
	DatfileRecord(void);
	~DatfileRecord(void);

	void ClearRecord();
	void SetRecord(char * filename, char * hash, int filesize, string datfilename);
	DatfileRecord operator=(DatfileRecord Record);
	bool operator>(DatfileRecord Record);
	bool operator<(DatfileRecord Record);

	char m_filename[256+1];
	char m_hash[40+1];
	int m_size;
	char m_datfilename[256+1];
	CTime m_time;
};
