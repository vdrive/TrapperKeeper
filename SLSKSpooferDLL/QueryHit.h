// QueryHit.h
#pragma once

class QueryHit
{
public:
	// Public Member Functions
	QueryHit();
	QueryHit(const QueryHit &hit);
	~QueryHit();
	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);

	QueryHit& operator=(const QueryHit &hit);

	
	// Public Data Members
	unsigned short m_search_id;
	unsigned short m_port;
	unsigned int m_ip;
	unsigned int m_file_size;
	unsigned short m_track;
	CTime m_timestamp;
	char m_hash[FST_FTHASH_LEN];
	CString m_filename;
	CString m_project;
	unsigned int m_project_id;
	bool m_from_MD; //this is MediaDefender's decoy or spoof
};