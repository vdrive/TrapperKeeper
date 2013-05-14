#pragma once
#include "..\tkcom\object.h"

class SearchTask : public Object
{
public:
	string m_project;
	string m_search_string;
	UINT m_minimum_size;
	
	SearchTask(void);
	SearchTask(const char* project, const char* search_string, UINT minimum_size);
	~SearchTask(void);

	const SearchTask& operator =(  SearchTask *st ){
		m_project=st->m_project;
		m_search_string=st->m_search_string;
		m_minimum_size=st->m_minimum_size;
	}

	SearchTask( const SearchTask &copy){
		*this=copy;
	}

};
