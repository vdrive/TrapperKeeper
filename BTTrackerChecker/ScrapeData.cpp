#include "scrapedata.h"
#include "stdafx.h"

ScrapeData::ScrapeData(string hash_id, string tracker_number, string hash, string url)
{
	m_name = "";
	m_status = "unknown";
	m_complete = -1;
	m_downloaded = -1;
	m_incomplete = -1;
	m_min_request_interval = 0;
	m_hash = hash;
	m_hash_id = hash_id;
	m_tracker_number = tracker_number;
	m_url = url;
}

bool ScrapeData::Parse(int size, unsigned short * buf) 
{
	if(buf == NULL) return false;
	int cur_pos = 0;
	if(buf[cur_pos] != 'd' || buf[cur_pos+1] == 'b') {
		m_status = "not found";
		return false;
	}
	bool b_match = false;
	int open = 0; // how many open dictionaries there are.
	do {
		if(buf[cur_pos] == 'd') {
			cur_pos++;
			open++;
		}
		else if(buf[cur_pos] == 'e') {
			cur_pos++;
			open--;
		}
		else if(buf[cur_pos] == 'i') {
			DecodeInt(buf, &cur_pos);
		}
		else {
			if(buf[cur_pos] == '2' && buf[cur_pos+1] == '0' && !b_match) {
				int temp_pos = cur_pos;
				if(DecodeString(buf, &cur_pos).compare("min_request_interval") == 0) {
					cur_pos = temp_pos;
				}
				else {
					cur_pos = temp_pos;
					cur_pos += 3;
					unsigned short hash[20+1];
					memcpy(hash, &buf[cur_pos], 20*sizeof(unsigned short));
					hash[20] = '\0';
					cur_pos += 20;
					b_match = (m_hash.compare(GetHash(hash, 20)) == 0);
					if(!b_match) {
						TRACE("\tno match: %s\n", m_hash.c_str());
					}
					continue;
				}
			}
			string tempstring = DecodeString(buf, &cur_pos);
			if(tempstring.compare("files") == 0) {
			}
			
			else if(tempstring.compare("complete") == 0) {
				if(b_match || size < 51) m_complete = DecodeInt(buf, &cur_pos);
			}
			else if(tempstring.compare("downloaded") == 0) {
				if(b_match || size < 51) m_downloaded = DecodeInt(buf, &cur_pos);
			}
			else if(tempstring.compare("incomplete") == 0) {
				if(b_match || size < 51) m_incomplete = DecodeInt(buf, &cur_pos);
			}
			else if(tempstring.compare("name") == 0) {
				if(b_match || size < 51) m_name = DecodeString(buf, &cur_pos);
			}
			else if(tempstring.compare("flags") == 0) {

			}
			else if(tempstring.compare("min_request_interval") == 0) {
				m_min_request_interval = DecodeInt(buf, &cur_pos);
			}
			else if(tempstring.compare("failure reason") == 0) {
				m_failure_reason = DecodeString(buf, &cur_pos);
				if(m_failure_reason.find("SCRAPE_HASH_NOT_FOUND") != -1) return false;
			}
			else  {
				TRACE("NO MATCH FOR %s\n", tempstring.c_str());
				if(tempstring.find("SCRAPE_HASH_NOT_FOUND") != -1) return false;
				if(buf[cur_pos] == 'i') TRACE("value: %d\n", DecodeInt(buf, &cur_pos));
			}
		}

	} while(cur_pos < size && open > 0);
	if(!b_match && m_complete != -1 && m_downloaded != -1 && m_incomplete != -1) {
		b_match = true;
	}
	if(!b_match && cur_pos != 11 && cur_pos != 49 && cur_pos != 50) {
		TRACE("No match");
	}
	return b_match;
}

ScrapeData::~ScrapeData(void)
{
}

string ScrapeData::GetHash(unsigned short * p_hash, int size) {
	if(p_hash == NULL) return "";
	bool b_has_data = false;
	for(int i = 0; i < size; i++) {
		if(p_hash[i] != 0) {
			b_has_data = true;
			break;
		}
	}
	if(!b_has_data) return "";
	unsigned short * hash = new unsigned short[size*2];
	memcpy(hash, p_hash, size*sizeof(unsigned short));
	char * hashStr = new char[2*size+1];
	char szTemp[4];
	strcpy(szTemp, "");
	strcpy(hashStr, "");
	for(int i = 0; i < size; i++) {
		int value = (unsigned char)hash[i];
		if(value < 16) strcat(hashStr, "0");
		itoa(value, szTemp, 16);
		strcat(hashStr, szTemp);
	}
	//TRACE("hash: %s\n", hashStr);
	delete [] hash;
	string retVal= string(hashStr);
	delete [] hashStr;
	return retVal;
}

