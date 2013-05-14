#pragma once

class MetaData
{
public:
	MetaData(void);
	~MetaData(void);
	bool operator < (const MetaData&  other)const;
	bool operator ==(MetaData &other);

	void Clear();
	void SetFileHash(byte* hash);
	void SetKeywordHash(byte* hash);


	byte m_file_hash[16];
	byte m_keyword_hash[16];
	CString m_keyword;
	unsigned int m_checksum;

	//meta data
	CString m_filename;
	UINT m_filesize;
	CString m_file_type;
	CString m_file_format;
	CString m_artist;
	CString m_album;
	CString m_title;
	CString m_length;
	CString m_codec;
	UINT m_bitrate;
	UINT m_availability;
};
class MetaDataHash : public hash_compare <MetaData>
{
public:
	using hash_compare<MetaData>::operator();
	size_t operator()(const MetaData& key)const
	{
		return key.m_checksum;
	}
};