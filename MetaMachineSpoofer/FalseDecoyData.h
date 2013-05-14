#pragma once
#include "..\tkcom\threadedobject.h"
#include "DonkeyFile.h"

class FalseDecoyData : public ThreadedObject
{
private:
	void CreateFalseDecoy(const char* project,const char* file_name, UINT file_size,int seed,bool const_seed,bool evil_file);
	UINT Run();
public:
	FalseDecoyData(void);
	~FalseDecoyData(void);

	
	Vector mv_false_decoys;

	CCriticalSection m_data_lock;

	bool mb_has_inited;  //signals whether or not we have loaded decoys for the first time, so we can delay starting the servers
};
