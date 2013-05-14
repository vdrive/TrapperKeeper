#pragma once

class FileSizeAndHash
{
	friend class FileSizeAndHashHash;
public:
	FileSizeAndHash(void);
	~FileSizeAndHash(void);
	bool operator < (const FileSizeAndHash&  other)const;
	bool operator ==(FileSizeAndHash &other);
	void Clear();
	void SetHashAndSize(byte* hash, UINT size);

	unsigned int m_filesize;

private:
	void SetHashChecksum();

	byte m_hash[FST_FTHASH_LEN];
public:
	unsigned int m_checksum;
};

class FileSizeAndHashHash : public hash_compare <FileSizeAndHash>
{
public:
	using hash_compare<FileSizeAndHash>::operator();
	size_t operator()(const FileSizeAndHash& key)const
	{
		return key.m_checksum;
	}

};