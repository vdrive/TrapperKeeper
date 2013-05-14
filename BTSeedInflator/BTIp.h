#pragma once

#include "ProjectDataStructure.h"

class BTIP : public ProjectDataStructure
{
public:
	BTIP(void);
	~BTIP(void);

	char m_id[40+1];
	char m_ip[15+1];
	int m_port;
	int m_torrent_id;

	virtual void Clear();						// call this in the constructors of the derived classes
	virtual int GetBufferLength();
	virtual int WriteToBuffer(char *buf);
	virtual int ReadFromBuffer(char *buf);	// returns buffer length
};
