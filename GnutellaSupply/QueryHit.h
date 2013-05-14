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

	char *Filename();
	void Filename(char *filename);
//	char *Info();
//	void Info(char *info);
	
	static bool IsSpoofDistPoison(char *data,int len);
	bool IsSwarm();

	// Public Data Members
	GUID m_guid;
	unsigned short int m_port;
	int m_ip;
	unsigned int m_speed;
	unsigned int m_file_index;
	unsigned int m_file_size;
	int m_track;

	char m_sha1[32+1];

//	bool m_is_searcher_query_hit;
	bool m_is_spoof_dist_poison;

	CString m_project;
	CTime m_timestamp;

	UINT m_project_id;

private:
	char *p_filename;
//	char *p_info;
};