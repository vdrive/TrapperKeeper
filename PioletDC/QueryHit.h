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
	void Filename(const char *filename);

	char* MD5();
	void MD5(char* md5);

//	char *Info();
//	void Info(char *info);
	
//	static bool IsSpoofDistPoison(char *data,int len);

	// Public Data Members
//	GUID m_guid;
//	unsigned short int m_port;
	unsigned int m_ip;
	unsigned int m_header_counter;
	CString m_nickname;
//	unsigned int m_speed;
//	unsigned int m_file_index;
	unsigned int m_file_size;
	int m_track;
	int m_bitrate;
	int m_sampling_rate;
	int m_stereo;
	int m_song_length;



//	char m_sha1[32+1];

//	bool m_is_searcher_query_hit;
//	bool m_is_spoof_dist_poison;

	CString m_project;
	CTime m_timestamp;

private:
	char *p_filename;
	char *p_md5;
//	char *p_info;
};