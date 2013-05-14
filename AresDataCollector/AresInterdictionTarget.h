#pragma once
#include "..\tkcom\object.h"

class AresInterdictionTarget : public Object
{
public:
	unsigned int m_ip;
	unsigned short m_port;
	unsigned int m_size;
	byte m_hash[20];
public:
	AresInterdictionTarget(unsigned int ip, unsigned short port,unsigned int size, const char* hash);
	~AresInterdictionTarget(void);
};
