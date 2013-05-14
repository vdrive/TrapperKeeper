// SearchManagerThreadData.cpp

#include "stdafx.h"
#include "SearchManagerThreadData.h"

//
//
//
SearchManagerThreadData::SearchManagerThreadData()
: p_sr(NULL)
{
	Clear();

	for( int i = 0; i < SEARCH_MANAGER_WAIT_EVENTS; i++ )
	{
		m_reserved_events[i] = CreateEvent( NULL, true, false, NULL );
	}
}

//
//
//
SearchManagerThreadData::~SearchManagerThreadData()
{
	if( p_sr != NULL )
	{
		while( !p_sr->empty() )
		{
			if( p_sr->front() != NULL )
				delete p_sr->front();
			p_sr->pop();
		}
		delete p_sr;
		p_sr = NULL;
	}
}

//
//
//
void SearchManagerThreadData::Clear()
{
	if( p_sr != NULL )
	{
		while( !p_sr->empty() )
		{
			if( p_sr->front() != NULL )
				delete p_sr->front();
			p_sr->pop();
		}
		delete p_sr;
		p_sr = NULL;
	}
	memset(m_reserved_events,0,sizeof(m_reserved_events));
}