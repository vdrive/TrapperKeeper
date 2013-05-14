// QueryHitResultHeader.h
#pragma once

class QueryHitResultHeader
{
public:
	QueryHitResultHeader();
	~QueryHitResultHeader();
	void Clear();

	// Read Data Functions
	unsigned int Size();
	unsigned int Index();

	// Write Data Functions
	void Index(unsigned int index);
	void Size(unsigned int size);

private:
	unsigned char m_data[8];
};