// PacketAssembler.h

#pragma once
#include <queue>
#include <vector>
using namespace std;

#include "PacketIO.h"

#include "PacketAssemblerMessageWnd.h"
#include "PacketAssemblerThreadData.h"

class CCriticalSection;
class SoulSeekClientDll;

class PacketAssembler
{
public:
	PacketAssembler();
	~PacketAssembler();
	void Run();
	void InitParent(SoulSeekClientDll *parent);
	void SetPAEvent(unsigned int theEvent);

	// Messages
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void ReturnResults(PacketWriter* packet);
	void AssemblePacket(unsigned char* packetdata);
	void WriteToLog(char* stringToWrite, int warningLevel);
	void WriteToLogFile(char* s);

private:
	SoulSeekClientDll *p_parent;

	CCriticalSection *p_critical_section;
	PacketAssemblerThreadData *p_thread_data;
	CWinThread *p_thread;

	PacketAssemblerMessageWnd m_wnd;
};