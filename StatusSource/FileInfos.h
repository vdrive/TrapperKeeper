#pragma once
#include "FileInfo.h"

class FileInfos
{
public:
	FileInfos(void);
	~FileInfos(void);

	void Clear();
	int GetBufferLength();
	int ReadFromBuffer(char* buf);
	int WriteToBuffer(char* buf);

	vector<FileInfo> v_files;
};
