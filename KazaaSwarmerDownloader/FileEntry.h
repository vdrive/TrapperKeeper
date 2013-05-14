#pragma once

class FileEntry
{
public:
	FileEntry(void);
	~FileEntry(void);

	bool operator >(FileEntry &entry);
	FileEntry& operator = (const FileEntry &entry);
	bool Save();
	
	char m_project[256];
	int m_track;
	char m_hash[41];
	char m_filename[256];
	int m_size;
	int m_count;
	char m_timestamp[256];
	int m_state;

};
