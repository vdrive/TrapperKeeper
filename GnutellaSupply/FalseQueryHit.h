// FalseQueryHit,h
#pragma once

#include "QueryHitHeader.h"
#include "QueryHitResult.h"

class FalseQueryHit
{
public:
	// Public Member Functions
	FalseQueryHit();
	~FalseQueryHit();
	void Clear();
	void InitTrailer();
	void InitGUID();

	int GetBufferLength();
	void WriteToBuffer(char *buf);

	void Tweak();

	// Public Data Members
	QueryHitHeader m_hdr;
	vector<QueryHitResult> v_results;
	char m_trailer[7+2];	// +2 private data ... 409 -> 0x0199
	GUID m_guid;
};