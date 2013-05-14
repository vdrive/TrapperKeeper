#pragma once

#include "..\BTScraperDLL\bencode.h"

class ScrapeData
{
public:
	ScrapeData(string hash_id, string tracker_number, string hash, string url);
	~ScrapeData(void);

	bool Parse(int size, unsigned short * buf);
	string GetHash(unsigned short * p_hash, int size);

	int m_complete;
	int m_downloaded;
	int m_incomplete;
	string m_name; 
	string m_status;
	string m_hash_id;
	string m_tracker_number;
	string m_failure_reason;
	string m_hash;
	string m_url;
	int m_min_request_interval;
};
