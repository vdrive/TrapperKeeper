#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\vector.h"
#include "UInt128.h"

using namespace Kademlia;
class KadFile : public Object
{
public:
	KadFile(const char* name,byte* hash,UINT size);
	~KadFile(void);

	Timer m_last_publish;
	bool mb_published;
	string m_name;
	UINT m_size;
	CUInt128 m_hash;
	Vector mv_keywords;

private:
	void CreateKeywords(void);
public:
	void AddKeyword(const char* str);
	bool IsWordBreakChar(char ch);
};
