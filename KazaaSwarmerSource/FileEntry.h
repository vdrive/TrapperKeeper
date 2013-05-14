#pragma once

class FileEntry
{
public:
	FileEntry(void);
	~FileEntry(void);

	bool operator >(FileEntry &entry);
	bool Save();
	
	char m_project[256];
	int m_track;
	char m_hash[51];
	char m_filename[256];
	int m_size;
	int m_count;
	char m_timestamp[256];
};
