// PacketAssemblerThreadData.h

#pragma once
#include <queue>
using namespace std;


#define PACKET_ASSEMBLER_WAIT_EVENTS 4

class PacketAssemblerThreadData
{
public:
	PacketAssemblerThreadData();
	~PacketAssemblerThreadData();
	void Clear();

	HANDLE m_reserved_events[PACKET_ASSEMBLER_WAIT_EVENTS];
	queue<unsigned char *>* p_preprocessedPackets;
};