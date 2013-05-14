// SearchManagerThreadData.h

#pragma once
#include "SearchRequest.h"
#include <queue>
using namespace std;


#define SEARCH_MANAGER_WAIT_EVENTS 4

class SearchManagerThreadData
{
public:
	SearchManagerThreadData();
	~SearchManagerThreadData();
	void Clear();

	HANDLE m_reserved_events[SEARCH_MANAGER_WAIT_EVENTS];
	queue<SearchRequest *>* p_sr;
};