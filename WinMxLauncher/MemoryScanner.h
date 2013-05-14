#pragma once
#include "threadedobject.h"

#include <vector>

using namespace std ;

class MemoryScanner :
	public ThreadedObject
{
public:
	UINT Run();
	MemoryScanner(void);
	virtual ~MemoryScanner(void);
};
