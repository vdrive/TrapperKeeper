// PacketAssemblerThreadData.cpp

#include "stdafx.h"
#include "PacketAssemblerThreadData.h"

//
//
//
PacketAssemblerThreadData::PacketAssemblerThreadData()
: p_preprocessedPackets(NULL)
{
	Clear();

	for( int i = 0; i < PACKET_ASSEMBLER_WAIT_EVENTS; i++ )
	{
		m_reserved_events[i] = CreateEvent( NULL, true, false, NULL );
	}
}

//
//
//
PacketAssemblerThreadData::~PacketAssemblerThreadData()
{
	if( p_preprocessedPackets != NULL )
	{
		while( !p_preprocessedPackets->empty() )
		{
			if( p_preprocessedPackets->front() != NULL )
				delete p_preprocessedPackets->front();
			p_preprocessedPackets->pop();
		}
		delete p_preprocessedPackets;
		p_preprocessedPackets = NULL;
	}
}

//
//
//
void PacketAssemblerThreadData::Clear()
{
	if( p_preprocessedPackets != NULL )
	{
		while( !p_preprocessedPackets->empty() )
		{
			if( p_preprocessedPackets->front() != NULL )
				delete p_preprocessedPackets->front();
			p_preprocessedPackets->pop();
		}
		delete p_preprocessedPackets;
		p_preprocessedPackets = NULL;
	}
	memset(m_reserved_events,0,sizeof(m_reserved_events));
}