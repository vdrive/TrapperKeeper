#pragma once
#include "ProjectDataStructure.h"

class HashTable : public ProjectDataStructure
{
public:
	HashTable(void);
	~HashTable(void);
	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);


	CString m_sha1;
	UINT m_filesize;
};
