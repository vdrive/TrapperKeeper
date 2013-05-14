// QueryHitHeader
#pragma once

class QueryHitHeader
{
public:
	QueryHitHeader();
	~QueryHitHeader();
	void Clear();

	// Read Data Functions
//	unsigned int Size();

	// Write Data Functions
	void NumberOfHits(unsigned char count);
	void Port(unsigned short int port);
	void IP(unsigned int ip);
	void Speed(unsigned int speed);
	
private:
	unsigned char m_data[11];
};