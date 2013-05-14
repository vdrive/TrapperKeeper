#pragma once
#include "IPAddress.h"

class VectorIPAddress
{
public:
	VectorIPAddress(void);
	~VectorIPAddress(void);
	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);

	vector<IPAddress> v_ip_addresses;
};
