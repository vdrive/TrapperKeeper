// QueryHitResult.h
#pragma once

class QueryHitResult
{
public:
	// Public Member Functions
	QueryHitResult();
	~QueryHitResult();
	void Clear();

	int GetBufferLength();
	void WriteToBuffer(char *buf);

	// Public Data Members
	unsigned int m_file_index;
	unsigned int m_file_size;
	string m_file_name;
	string m_info;
};