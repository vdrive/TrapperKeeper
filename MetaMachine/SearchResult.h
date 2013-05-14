#pragma once
#include "..\tkcom\vector.h"

class SearchResult :	public Object
{
public:
	class SuperInt : public Object{  //an int wrapper so we can store ints in Vector, since vector <UINT> is sucking balls doing all the appends.
	public:
		UINT m_val;
		SuperInt(UINT val){
			m_val=val;
		}
	};
	byte m_hash[16];
	string m_file_name;
	UINT m_size;
	//UINT m_availability;
	//vector <UINT>mv_ips;	//collection of ips that are sources for this file.
	//Vector mv_ips;
	string m_search_string;
	
private:
	static UINT sm_search_result_count;

public:
	SearchResult(void);
	~SearchResult(void);
	void Clear(void);
	//void AddSource(UINT ip);
	bool IsHash(byte* hash);
	void Dump(void);

	int CompareTo(Object* object)  //for sorting by hash
	{
		SearchResult *sr=(SearchResult*)object;
		return memcmp(m_hash,sr->m_hash,16);
	}
};
