#pragma once
#include "..\tkcom\object.h"
#include "UInt128.h"
using namespace Kademlia;
class KadContact : public Object
{
public:
	CUInt128 m_hash;
	UINT m_ip;
	string m_sip;
	unsigned short m_port;
	CTime m_create_time;
	CTime m_last_bootstrap_request;
	
	// Compares this object to another.  Classes should override this function if they want sorting capability etc.
	inline int CompareTo(Object* object)
	{
		KadContact *ah=(KadContact*)object;
		return -m_hash.compareTo(ah->m_hash);
	}

	KadContact(CUInt128 &hash,UINT ip,unsigned short port,CTime &create_time);
	KadContact();
	~KadContact(void);
	bool IsExpired(void);
	void WriteToFile(HANDLE file);
	byte* ReadFromBuffer(byte* ptr);
};
