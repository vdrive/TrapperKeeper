// GnutellaHeader.h
#pragma once

#include "GnutellaOpCode.h"

class GnutellaHeader
{
public:
	GnutellaHeader();
	~GnutellaHeader(){};

	void Init();

	// Data Reading Functions
	GUID Guid();
	unsigned char Op();
	unsigned char TTL();
	unsigned char Hops();
	unsigned int Length();

	// Data Writing Functions
	void Guid(GUID guid);
	void Op(unsigned char op);
	void TTL(unsigned char ttl);
	void Hops(unsigned char hops);
	void Length(unsigned int length);
	
private:
	unsigned char m_data[23];
};