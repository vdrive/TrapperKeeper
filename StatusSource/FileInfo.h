#pragma once

class FileInfo
{
public:
	FileInfo(void);
	~FileInfo(void);

	void Clear();
	int GetBufferLength();
	int ReadFromBuffer(char* buf);
	int WriteToBuffer(char* buf);

	CTime m_creation_date;
	UINT m_file_size;
	CString m_filename;
	CString m_path;
};
