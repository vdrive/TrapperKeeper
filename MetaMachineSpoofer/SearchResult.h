#pragma once
#include "..\tkcom\object.h"

class SearchResult : public Object
{
public:
	string m_project;
	string m_hash;
	string m_server_ip;
	string m_file_name;
	UINT m_availability;
	UINT m_size;

	SearchResult(void);
	~SearchResult(void);
};
