// ConnectionModuleThreadData.cpp

#include "stdafx.h"
#include "ClientModuleThreadData.h"

//
//
//
ClientModuleThreadData::ClientModuleThreadData()
: p_qPacketsToSend(NULL)
{
	Clear();
	/*for( int i = 0; i < 4; i++ )
	{
		m_reserved_events[i] = CreateEvent( NULL, true, false, NULL );
	}*/
}

//
//
//
ClientModuleThreadData::~ClientModuleThreadData()
{
	if( p_qPacketsToSend != NULL )
	{
		while( !p_qPacketsToSend->empty() )
		{
			if( p_qPacketsToSend->front() != NULL )
				delete p_qPacketsToSend->front();
			p_qPacketsToSend->pop();
		}
		delete p_qPacketsToSend;
		p_qPacketsToSend = NULL;
	}
}

//
//
//
void ClientModuleThreadData::Clear()
{
	if( p_qPacketsToSend != NULL )
	{
		while( !p_qPacketsToSend->empty() )
		{
			if( p_qPacketsToSend->front() != NULL )
				delete p_qPacketsToSend->front();
			p_qPacketsToSend->pop();
		}
		delete p_qPacketsToSend;
		p_qPacketsToSend = NULL;
	}
	//p_critical_section=NULL;
	memset(m_reserved_events,0,sizeof(m_reserved_events));
}