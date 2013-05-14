// QueryHitResult.h
#pragma once
#define MEDIA_AUDIO	0
#define MEDIA_VIDEO 1
#define MEDIA_OTHER 2

class QueryHitResult
{
public:
	// Public Member Functions
	QueryHitResult();
	~QueryHitResult();
	void Clear();

	int GetBufferLength();
	int GetDiskStoringBufferLength();
	void WriteToDiskStoringBuffer(char* buf);
	int ReadFromDiskStoringBuffer(char* buf);
	void WriteToBuffer(char *buf);
	bool CalculateHash();
	void ExtractKeywordsFromQuery(const char *query);
	void ClearNonAlphaNumeric(char *buf,int buf_len);

	// Public Data Members
	unsigned int m_file_index;
	unsigned int m_file_size;
	string m_file_name;
	string m_info;
	string m_artist;
	string m_album;
	string m_title;
	string m_path;
	string m_comment;
	int m_bitrate;
	int m_type; // 0 - music, 1 - movie, 2 - software
	int m_duration;

	vector<string>v_keywords;
};