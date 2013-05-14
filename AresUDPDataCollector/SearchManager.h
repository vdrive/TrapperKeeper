#pragma once
#include "..\tkcom\ThreadedObject.h"
#include "..\tkcom\Vector.h"
#include "..\tkcom\timer.h"

class SearchManager : public ThreadedObject
{
protected:

	CString EncodeBase16(const unsigned char* buffer, unsigned int bufLen)
	{
		CString base16_buff;
		static byte base16_alphabet[] = "0123456789ABCDEF";

		for(unsigned int i = 0; i < bufLen; i++) {
			base16_buff += base16_alphabet[buffer[i] >> 4];
			base16_buff += base16_alphabet[buffer[i] & 0xf];
		}

		return base16_buff;
	}


	Timer m_last_search;

	Vector mv_search_projects;
	Vector mv_search_results[NUMCONBINS];
	UINT m_num_search_results;
public:
	SearchManager(void);
	~SearchManager(void);
	
	UINT Run(void);
	
	void GetNextSearchJob(Vector &v);
	void PostSearchResult(UINT query_id, const char* file_name, const char* artist, const char* album, const char* title, const char* user_name,const char* ip, byte* hash,UINT file_size);
	void LogSearchResults(void);
};
