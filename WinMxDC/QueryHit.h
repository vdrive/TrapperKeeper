// QueryHit.h
#pragma once

typedef enum
{
	MEDIA_TYPE_UNKNOWN  = 0x00,
	MEDIA_TYPE_AUDIO    = 0x01,
	MEDIA_TYPE_VIDEO    = 0x02,
	MEDIA_TYPE_IMAGE    = 0x03,
	MEDIA_TYPE_DOCUMENT = 0x04,
	MEDIA_TYPE_SOFTWARE = 0x05
} WINMXMediaType;

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
	char m_hash[16];
	CString m_filename;
	CString m_project;
	unsigned int m_project_id;
	WORD m_bitrate;
	int m_sample_frequency;
	int m_duration;
	WINMXMediaType m_media_type;
	
	CString m_folder_name;
};