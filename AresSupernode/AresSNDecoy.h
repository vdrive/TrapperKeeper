#pragma once
#include "..\tkcom\object.h"

class AresSNDecoy : public Object
{
public:
	AresSNDecoy(const char* hash, UINT size);
	~AresSNDecoy(void);

	byte m_hash[20];
	//int m_hash_sum;
	UINT m_size;
	UINT CalcDiff(byte* hash);
};
