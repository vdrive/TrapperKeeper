#pragma once
#include "..\tkcom\object.h"
#include "UInt128.h"
using namespace Kademlia;
class KadFileKey : public Object
{
public:
	string m_name;
	CUInt128 m_key;

	KadFileKey(const char* name);
	~KadFileKey(void);
};
