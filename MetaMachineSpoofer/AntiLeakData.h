#pragma once
#include "..\tkcom\vector.h"
#include <afxmt.h>

class AntiLeakData
{
public:
	CCriticalSection m_data_lock;
	Vector mv_popular_anti_leak_files;
	Vector mv_rare_anti_leak_files;
	AntiLeakData(void);
	~AntiLeakData(void);
	void Update(void);
	byte ConvertCharToInt(char ch);
	void AddPopularAntiLeakFile(const char* file_name, const char* hash, UINT size);
	void AddRareAntiLeakFile(const char* file_name, const char* hash, UINT size);
	void ClearRareAntiLeakFiles(void);
	void ClearPopularAntiLeakFiles(void);
};
