#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\vector.h"
#include <afxmt.h>

#define NUMDECOYBINS 30

class AresDecoyPoolCache : public ThreadedObject
{
private:
	CCriticalSection m_lock;
	UINT m_movie_index1;
	UINT m_movie_index2;

	UINT m_music_index1;
	UINT m_music_index2;

	UINT m_num_music_decoys;
	UINT m_num_movie_decoys;

public:
	AresDecoyPoolCache(void);
	~AresDecoyPoolCache(void);

	Vector mv_music_decoys[NUMDECOYBINS];
	Vector mv_movie_decoys[NUMDECOYBINS];
	UINT Run(void);
	bool GetDecoy(UINT& actual_size, byte* actual_hash, byte* desired_hash,UINT desired_size);
};
