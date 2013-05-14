#pragma once

#include "ProjectDataStructure.h"

class FileInfo : public ProjectDataStructure
{
public:
	unsigned int m_length;
	string m_md5;
	string m_pathname;

public:
	FileInfo(void);
	~FileInfo(void);

	virtual void Clear();						// call this in the constructors of the derived classes
	virtual int GetBufferLength();
	virtual int WriteToBuffer(char *buf);
	virtual int ReadFromBuffer(char *buf);	// returns buffer length
};
