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
	void InitSwarmTrailer(const char* artist, const char* album, const char* title, int bitrate,int track);
	void InitGUID();

	int GetBufferLength();
	void WriteToBuffer(char *buf);

	void Tweak();

	// Public Data Members
	QueryHitHeader m_hdr;
	vector<QueryHitResult> v_results;
	char m_trailer[7+2];	// +2 private data ... 409 -> 0x0199
	char* m_swarm_trailer;
	int m_swarm_trailer_len;
	GUID m_guid;
};