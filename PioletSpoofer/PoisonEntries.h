#pragma once
#include "PoisonEntry.h"

class PoisonEntries
{
public:
	PoisonEntries(void);
	~PoisonEntries(void);
	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);


	vector<PoisonEntry> v_poison_entries;
};
