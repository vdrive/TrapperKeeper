#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\vector.h"

#define NUMDECOYBINS 5000

class DecoyPool : public ThreadedObject
{
public:
	DecoyPool(void);
	~DecoyPool(void);
	UINT Run(void);
	void CreateSignature(byte* sig);

	CCriticalSection m_lock;
	Vector mv_decoys[256];	
	bool GetDecoyFile(Vector& v, byte* hash);
	void FillBufferWithMP3Data(byte* mp3_buffer, UINT mp3_buf_len);
};
