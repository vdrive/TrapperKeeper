#pragma once
#include "..\tkcom\ThreadedObject.h"
#include "..\tkcom\Vector.h"



class AresDecoyPool : public ThreadedObject
{
public:
	AresDecoyPool(void);
	~AresDecoyPool(void);
	UINT Run(void);

	Vector mv_small_decoys[256];
	Vector mv_big_decoys[256];

	int m_small_index1;
	int m_small_index2;

	int m_big_index1;
	int m_big_index2;

	CCriticalSection m_lock;
	bool mb_ready;
	bool GetNextDecoy(UINT desired_size, Vector& v);
	bool GetDecoy(byte* hash, Vector& v);
};
